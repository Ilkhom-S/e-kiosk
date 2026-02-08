/* @file Движок тегов. */

#include "Tags.h"

#include <QtCore/QRegularExpression>

//--------------------------------------------------------------------------------
namespace Tags {
const char BR[] = "[br]";
} // namespace Tags

void Tags::Engine::appendByGroup(bool aBitField,
                                 Type::Enum aType,
                                 const QByteArray &aPrefix,
                                 const QByteArray &aOpen,
                                 const QByteArray &aClose) {
    append(aType, STagData(aBitField, aPrefix, aOpen, aClose));
    m_PrefixData.insert(aType, aPrefix);
}

//--------------------------------------------------------------------------------
void Tags::Engine::appendSingle(Type::Enum aType,
                                const QByteArray &aPrefix,
                                const QByteArray &aOpen,
                                const QByteArray &aClose) {
    appendByGroup(false, aType, aPrefix, aOpen, aClose);
}

//--------------------------------------------------------------------------------
void Tags::Engine::appendCommon(Type::Enum aType,
                                const QByteArray &aPrefix,
                                const QByteArray &aOpen,
                                const QByteArray &aClose) {
    appendByGroup(true, aType, aPrefix, aOpen, aClose);
}

//--------------------------------------------------------------------------------
void Tags::Engine::set(Type::Enum aType) {
    appendByGroup(false, aType, "", "", "");
}

//--------------------------------------------------------------------------------
QByteArray Tags::Engine::getTag(const TTypes &aTypes, const Direction::Enum aDirection) const {
    if (aTypes.isEmpty()) {
        return "";
    }

    STagData tagData = operator[](*aTypes.begin());
    bool isClose = aDirection == Direction::Close;

    // если не битовое поле или закрываем - склеиваем префикс и актив
    if ((aTypes.size() == 1) || isClose) {
        return tagData.prefix + (isClose ? tagData.close : tagData.open);
    }

    char tag = 0;

    foreach (Type::Enum type, aTypes) {
        if (m_Buffer.contains(type)) {
            tag |= operator[](type).open[0];
        }
    }

    return tagData.prefix + tag;
}

//--------------------------------------------------------------------------------
void Tags::Engine::splitForLexemes(const QString &aSource, TLexemesBuffer &aTagLexemes) const {
    aTagLexemes.clear();
    // Добавляем маркер конца, используя QStringLiteral для оптимизации
    QString source = aSource + Tags::None;

    // 1. Используем QRegularExpression.
    // В Qt 6 эквивалент setMinimal(true) — это квантификатор '.*?' или '.+?' в самом паттерне.
    // Убедитесь, что Tags::regExpData в 2026 году использует нежадные квантификаторы.
    QRegularExpression regExp(Tags::regExpData);

    int pos = 0;

    // 2. В Qt 6/C++14 итерируемся по строке, используя объект Match
    while (pos != -1) {
        QRegularExpressionMatch match = regExp.match(source, pos);
        int begin = match.hasMatch() ? static_cast<int>(match.capturedStart()) : -1;

        Tags::TTypes tags;
        QString lexeme;

        if (aTagLexemes.isEmpty()) {
            // Если 1-я лексема, то складываем в лист то, что до 1-го тега
            lexeme = (begin == -1) ? aSource : source.left(begin);

            // Если тегов не найдено, выходим из цикла после добавления всей строки
            if (begin == -1)
                pos = -1;
            else
                pos = begin;
        } else if (begin != -1) {
            // 3. Используем match.captured(n) вместо capturedTexts() для производительности
            QString tagName = match.captured(1);
            lexeme = match.captured(2);

            Tags::Type::Enum type = Tags::Type::None;
            Tags::Direction::Enum direction;

            if (identifyTag(tagName, type, direction)) {
                tags = aTagLexemes.last().tags;
                Tags::Type::Enum tagType = Tags::Types[tagName];

                if (direction == Tags::Direction::Open) {
                    tags.insert(tagType);
                } else {
                    // Логика закрытия тега (совместима с C++14)
                    if (!aTagLexemes.isEmpty()) {
                        if (!aTagLexemes.last().tags.contains(tagType)) {
                            for (int i = 0; i < aTagLexemes.size(); ++i) {
                                aTagLexemes[i].tags.insert(tagType);
                            }
                        }
                    }
                    tags.remove(tagType);
                }
            } else {
                // Если тег не известен
                QString nextTagName;
                QString rawThirdGroup = match.captured(3);
                bool isIdentify = identifyTag(rawThirdGroup, type, direction);

                if (!isIdentify || (type != Tags::Type::None)) {
                    nextTagName = QStringLiteral("[%1%2]")
                                      .arg(direction == Tags::Direction::Open ? QString()
                                                                              : QStringLiteral("/"))
                                      .arg(rawThirdGroup);
                }

                lexeme = QStringLiteral("[%1]%2%3").arg(tagName).arg(lexeme).arg(nextTagName);
            }

            // Сдвигаем позицию для следующего поиска
            pos = begin + tagName.size() + lexeme.size();
        } else {
            // Больше совпадений нет
            pos = -1;
            continue;
        }

        aTagLexemes.push_back(Tags::SLexeme(lexeme, tags));
    }

    cleanLexemeBuffer(aTagLexemes);
}

//--------------------------------------------------------------------------------
void Tags::Engine::cleanLexemeBuffer(TLexemesBuffer &aTagLexemes) const {
    for (int i = 0; i < aTagLexemes.size(); ++i) {
        if (i > 1) {
            if (aTagLexemes[i].tags == aTagLexemes[i - 1].tags) {
                aTagLexemes[i - 1].data += aTagLexemes[i].data;
                aTagLexemes.removeAt(i);
                i--;
            }
        }

        if (aTagLexemes[i].data.isEmpty()) {
            aTagLexemes.removeAt(i);
            i--;
        }
    }
}

//--------------------------------------------------------------------------------
Tags::TGroupTypes Tags::Engine::groupsTypesByPrefix(const TTypes &aTypes) const {
    TTypes types = aTypes;
    TGroupTypes result;

    // Используем стандартный цикл C++14
    for (Type::Enum type : aTypes) {
        if (m_Buffer.contains(type)) {
            if (!m_Buffer[type].bitField) {
                TTypes singleTypes;
                singleTypes.insert(type);
                result.insert(singleTypes);
            } else {
                // 1. Получаем список ключей
                auto keysList = m_PrefixData.keys(m_Buffer[type].prefix);

                // 2. Создаем QSet из списка через конструктор итераторов (замена .toSet())
                TTypes prefixTypes(keysList.begin(), keysList.end());

                // 3. Используем оператор & для пересечения множеств (стандарт Qt 6)
                // Это создаст новое множество, содержащее только общие элементы
                result.insert(prefixTypes & types);

                types = aTypes;
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
bool Tags::Engine::identifyTag(QString &aTag,
                               Type::Enum &aType,
                               Direction::Enum &aDirection) const {
    aDirection = Direction::Open;

    if (aTag[0] == ASCII::ForwardSlash) {
        aDirection = Direction::Close;
        aTag = aTag.mid(1);
    }

    aType = Types[aTag];

    return aType != Type::None;
}

//--------------------------------------------------------------------------------
bool Tags::Engine::contains(Type::Enum aTag) const {
    return m_PrefixData.contains(aTag);
}

//--------------------------------------------------------------------------------
uint qHash(const Tags::TTypes &aTypes) {
    uint result = 1;

    foreach (Tags::Type::Enum type, aTypes) {
        uint element = uint(type) + 2;

        switch (element) {
        case 6:
            element = 17;
            break;
        case 8:
            element = 19;
            break;
        case 10:
            element = 23;
            break;
        case 12:
            element = 27;
            break;
        }

        result *= element;
    }

    return result;
}

//--------------------------------------------------------------------------------
