/* @file Набор вспомогательных функций для qml. */

#include "Utils.h"

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtGui/QKeyEvent>
#include <QtGui/QWindow>
#include <QtMultimedia/QSound>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#include "Log.h"

namespace CUtils {
const QString MaskPlaceholders = "AaNnXxDdHhBb09#";
const QString MaskModifiers = "<!>";

const QString DefaultConf = "interface.ini";
const QString UserConf = "user.ini";

const QString UseCommonSounds = "sound/use_common";
const QString UseNarratorSounds = "sound/use_narrator";
const QString UseAutoOrderProviders = "ui/auto_order_operator";
} // namespace CUtils

//------------------------------------------------------------------------------
Utils::Utils(QQmlEngine *aEngine, const QString &aInterfacePath, const QString &aUserPath)
    : m_Engine(aEngine), m_InterfacePath(aInterfacePath), m_UserPath(aUserPath),
      m_UseCommonSounds(true), m_UseNarratorSounds(false), m_UseAutoOrderProviders(false) {
    // Получаем директорию с файлами интерфейса.
    QObject *application = m_Engine->rootContext()->contextProperty("Core").value<QObject *>();

    Q_ASSERT(application);

    loadConfiguration();

    m_Translator = QSharedPointer<Translator>(new Translator(m_InterfacePath));
    connect(m_Translator.data(), SIGNAL(languageChanged()), this, SIGNAL(updateTranslator()));

    // Модель для интерфейса
    m_GroupModel = QSharedPointer<GroupModel>(new GroupModel());
    m_GroupModel->setStatistic(getStatistic());

    m_RootGroupModel = QSharedPointer<GroupModel>(new GroupModel());
    m_RootGroupModel->setElementFilter(QStringList() << "group");

    // Модели и фильтры для поиска
    m_ProviderListModel =
        QSharedPointer<ProviderListModel>(new ProviderListModel(this, m_GroupModel));
    connect(m_GroupModel.data(),
            SIGNAL(modelReset()),
            m_ProviderListModel.data(),
            SLOT(groupsUpdated()));
    m_ProviderListModel->setPaymentService(
        application ? application->property("payment").value<QObject *>() : nullptr);
    m_ProviderListFilter = QSharedPointer<ProviderListFilter>(new ProviderListFilter(this));
    m_ProviderListFilter->setSourceModel(m_ProviderListModel.data());
    m_ProviderListFilter->setDynamicSortFilter(true);

    m_Skin = QSharedPointer<Skin>(new Skin(application, m_InterfacePath, m_UserPath));

    m_GuiService = application->property("graphics").value<QObject *>();
    connect(m_GuiService,
            SIGNAL(skinReload(const QVariantMap &)),
            this,
            SLOT(onReloadSkin(const QVariantMap &)));
}

//------------------------------------------------------------------------------
static void Utils::generateKeyEvent(int aKey, int aModifiers, const QString &aText) {
    const QGuiApplication *app = qApp;
    QWindow *focusWindow = app ? app->focusWindow() : 0;
    if (focusWindow) {
        QGuiApplication::sendEvent(
            focusWindow,
            new QKeyEvent(QEvent::KeyPress, aKey, Qt::KeyboardModifiers(aModifiers), aText));
    }
}

//------------------------------------------------------------------------------
// подсчет количества позиций в маске ввода
int maskLength(QString mask) {
    int len = 0;

    mask.remove(QRegularExpression("(;.)$"));

    for (int i = 0; i < mask.size(); ++i) {
        if (mask[i] == '\\') {
            ++i;
        } else if (mask[i] == '>' || mask[i] == '<' || mask[i] == '!') {
            continue;
        }
        ++len;
    }

    return len;
}

//------------------------------------------------------------------------------
static QString Utils::stripMask(const QString &aSource, const QString &aMask) {
    int pos = 0;
    bool escaped = false;
    QString result;

    if (!aSource.length() || !aMask.length()) {
        return aSource;
    }

    QChar userPlaceholder = aMask[aMask.length() - 1];

    foreach (QChar c, aMask) {
        if (!escaped) {
            if (c == ';') {
                break;
            } else if (CUtils::MaskPlaceholders.contains(c) && pos < aSource.length() &&
                       aSource[pos] != userPlaceholder) {
                result += aSource[pos];
            } else if (c == '\\') {
                escaped = true;
            }
        } else {
            escaped = false;
        }

        if (!escaped && !CUtils::MaskModifiers.contains(c)) {
            ++pos;
        }
    }

    return aMask.isEmpty() ? aSource : result;
}

//------------------------------------------------------------------------------
// Маска                                 : +7 (999) 999-99-99;*
// Абсолютно пустое свойство displayValue: +7 (***) ***-**-**
// Свойство displayValue после ввода     : +7 (903) ***-**-**
// Позиция первого различия справа       :       ^
// Нужная позиция курсора                :          ^  (1-й placeholder справа от различия)

static int
Utils::getCursorPosition(const QString &aMask, const QString &aBefore, const QString &aAfter) {
    if (aMask.isEmpty()) {
        return aAfter.length();
    }
    if (aBefore.length() != aAfter.length()) {
        Log(Log::Error) << "Utils::getCursorPosition(): string lengths differ.";

        return 0;
    }

    // Находим первое справа отличие пустого displayValue от заполненного
    int mismatchIndex = 0;

    for (mismatchIndex = aBefore.length() - 1; mismatchIndex >= 0; --mismatchIndex) {
        if (aBefore[mismatchIndex] != aAfter[mismatchIndex]) {
            break;
        }
    }

    // Теперь находим первый справа от различия незаполненный символ
    ++mismatchIndex;

    int placeholderIndex = 0;
    bool escaped = false;

    foreach (QChar c, aMask) {
        if (!escaped) {
            if (c == ';') {
                mismatchIndex = aAfter.length();
                break;
            } else if (placeholderIndex >= mismatchIndex && CUtils::MaskPlaceholders.contains(c)) {
                break;
            } else if (c == '\\') {
                escaped = true;
            }
        } else {
            escaped = false;
        }

        if (!escaped && !CUtils::MaskModifiers.contains(c)) {
            ++placeholderIndex;
        }
    }

    return placeholderIndex;
}

//------------------------------------------------------------------------------
static QString Utils::format(const QString &aSource, const QString &aFormat) {
    QString result = aFormat;

    if (!result.isEmpty()) {
        QRegularExpression rx(R"(\[(\d+)\])");
        QList<QPair<int, QString>> replacements;

        // Find all matches
        QRegularExpressionMatchIterator it = rx.globalMatch(result);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int srcIndex = match.captured(1).toInt() - 1;
            QString replacement = (srcIndex >= 0 && srcIndex < aSource.size())
                                      ? QString(aSource.at(srcIndex))
                                      : QString();
            replacements.append(qMakePair(match.capturedStart(), replacement));
        }

        // Replace from the end to avoid position shifts
        for (int i = replacements.size() - 1; i >= 0; --i) {
            const auto &replacement = replacements.at(i);
            QRegularExpressionMatch match = rx.match(result, replacement.first);
            if (match.hasMatch()) {
                result.replace(match.capturedStart(), match.capturedLength(), replacement.second);
            }
        }
    }

    return result;
}

//------------------------------------------------------------------------------
static QString Utils::readFile(const QString &aPath) {
    // TODO если путь относительный, то считать от каталога скина
    QFile file(aPath);
    QString result;

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray contents = file.readAll();
        QMap<QString, QTextCodec *>::iterator codec = m_CodecCache.find(aPath);

        if (codec == m_CodecCache.end()) {
            codec = m_CodecCache.insert(aPath, QTextCodec::codecForHtml(contents));
        }

        result = (*codec)->toUnicode(contents);
    } else {
        Log(Log::Error) << "Failed to read file " << aPath << ": " << file.errorString();
    }

    return result;
}

//------------------------------------------------------------------------------
static bool Utils::fileExist(const QString &aPath) {
    return QFileInfo(aPath).exists();
}

//------------------------------------------------------------------------------
static QString Utils::toHtml(const QString &aSource) {
    return QString(aSource)
        .replace("[br]", " ")
        .replace(QRegularExpression("\\s+"), " ")
        .replace(QRegularExpression(R"(\[(/*\w+)\])"), "<" + QString("\\1") + ">");
}

//------------------------------------------------------------------------------
static QString Utils::toPlain(const QString &aSource) {
    return QString(aSource)
        .replace("[br]", "")
        .replace(QRegularExpression("\\s+"), " ")
        .replace(QRegularExpression(R"(\[(/*\w+)\])"), "")
        .replace(QRegularExpression("<(/*\\w+)>"), "");
}

//------------------------------------------------------------------------------
QObject *Utils::getTranslator() {
    return m_Translator.data();
}

//------------------------------------------------------------------------------
QObject *Utils::getGroupModel() {
    return m_GroupModel.data();
}

//------------------------------------------------------------------------------
QObject *Utils::getRootGroupModel() {
    return m_RootGroupModel.data();
}

//------------------------------------------------------------------------------
QObject *Utils::getProviderList() {
    return m_ProviderListFilter.data();
}

//------------------------------------------------------------------------------
QObject *Utils::getSkin() {
    return m_Skin.data();
}

// Файлы для воспроизведения должны лежать в соответствующих папках: common, narrator, etc
// Файл из произвольного места воспроизведен не будет
//------------------------------------------------------------------------------
void Utils::playSound(const QString &aFileName) const {
    QStringList path = aFileName.split("/");
    path.removeLast();

    QString key = QString("sound/use_%1").arg(path.last());
    QString filePath;

    if (key == CUtils::UseCommonSounds && m_UseCommonSounds) {
        filePath = m_InterfacePath + QDir::separator() + "sounds" + QDir::separator() + aFileName;
    } else if (key == CUtils::UseNarratorSounds && m_UseNarratorSounds) {
        filePath = m_InterfacePath + QDir::separator() + "sounds" + QDir::separator() +
                   m_Translator->getLanguage() + QDir::separator() + aFileName;
    } else {
        // Звук отключен
        return;
    }

    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        Log(Log::Warning) << QString("Audio file %1 not found.").arg(filePath);
        return;
    }

    QSound::play(filePath);
}

//------------------------------------------------------------------------------
QString Utils::fromBase64(const QString &aSource) const {
    return QTextCodec::codecForName("UTF-8")->toUnicode(
        QByteArray::from_Base64(aSource.toLatin1()));
}

//------------------------------------------------------------------------------
QString Utils::fromUrlEncoding(const QString &aSource) const {
    return QTextCodec::codecForName("windows-1251")
        ->toUnicode(QByteArray::fromPercentEncoding(aSource.toLatin1()));
}

//------------------------------------------------------------------------------
void Utils::click() {
    QMetaObject::invokeMethod(this, "onClicked", Qt::QueuedConnection);
}

//------------------------------------------------------------------------------
static void Utils::onClicked() {
    emit clicked();
}

//------------------------------------------------------------------------------
static QVariantMap Utils::str2json(const QString &aString) {
    if (aString.isEmpty()) {
        return QVariantMap();
    }

    QJsonParseError ok;
    QString str = aString;
    QJsonDocument result = QJsonDocument::from_Json(str.replace("'", "\"").toUtf8(), &ok);

    if (ok.error == QJsonParseError::NoError) {
        return result.object().toVariantMap();
    }

    Log(Log::Error) << QString("Utils: failed to parsed JSON string %1").arg(aString);

    return QVariantMap();
}

//------------------------------------------------------------------------------
static void Utils::onReloadSkin(const QVariantMap &aParams) {
    if (m_Skin->needReload(aParams)) {
        QMetaObject::invokeMethod(m_GuiService, "reset", Qt::DirectConnection);
        m_Skin->reload(aParams);
    }
}

//------------------------------------------------------------------------------
QMap<qint64, quint32> Utils::getStatistic() const {
    QMap<qint64, quint32> result;

    if (m_UseAutoOrderProviders) {
        QObject *application = m_Engine->rootContext()->contextProperty("Core").value<QObject *>();
        QObject *paymentService =
            application ? application->property("payment").value<QObject *>() : nullptr;

        QVariantMap statistic;

        if (paymentService && QMetaObject::invokeMethod(paymentService,
                                                        "getStatistic",
                                                        Q_RETURN_ARG(QVariantMap, statistic))) {
            QMapIterator<QString, QVariant> i(statistic);

            while (i.hasNext()) {
                i.next();

                result.insert(i.key().toLongLong(), i.value().toUInt());
            }
        }
    }

    return result;
}

//------------------------------------------------------------------------------
void Utils::loadConfiguration() {
    // Настройки
    QVariantMap configuration;

    // Загружаем значения по умолчанию
    {
        QSettings defaultSettings(m_InterfacePath + QDir::separator() + CUtils::DefaultConf,
                                  QSettings::IniFormat);
        defaultSettings.setIniCodec("UTF-8");

        foreach (QString key, defaultSettings.allKeys()) {
            configuration.insert(key, defaultSettings.value(key));
        }
    }

    // Загружаем пользовательские настройки
    {
        QSettings userSettings(m_UserPath + QDir::separator() + CUtils::UserConf,
                               QSettings::IniFormat);
        userSettings.setIniCodec("UTF-8");

        foreach (QString key, userSettings.allKeys()) {
            configuration.insert(key, userSettings.value(key));
        }
    }

    m_UseCommonSounds = configuration.value(CUtils::UseCommonSounds, m_UseCommonSounds).toBool();
    m_UseNarratorSounds =
        configuration.value(CUtils::UseNarratorSounds, m_UseNarratorSounds).toBool();
    m_UseAutoOrderProviders =
        configuration.value(CUtils::UseAutoOrderProviders, m_UseAutoOrderProviders).toBool();
}

//------------------------------------------------------------------------------
