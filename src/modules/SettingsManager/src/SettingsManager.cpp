/* @file Интерфейс менеджера настроек. */

// stl

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

#include <SettingsManager/SettingsManager.h>
#include <boost/foreach.hpp>
#include <fstream>
#include <vector>

SSettingsSource::SSettingsSource() : readOnly(true) {}

//---------------------------------------------------------------------------
SSettingsSource::SSettingsSource(const QString &aFileName,
                                 const QString &aAdapterName,
                                 bool aReadOnly)
    : configFileName(aFileName), adapterName(aAdapterName), readOnly(aReadOnly) {}

//---------------------------------------------------------------------------
SSettingsSource::SSettingsSource(const QString &aFileName,
                                 const QString &aAdapterName,
                                 const char *aSymlinkName)
    : configFileName(aFileName), adapterName(aAdapterName),
      symlinkName(QString::fromLatin1(aSymlinkName)), readOnly(true) {}

//---------------------------------------------------------------------------
SettingsManager::SettingsManager(const QString &aConfigPath) : m_ConfigPath(aConfigPath) {}

//---------------------------------------------------------------------------
SettingsManager::~SettingsManager() {}

//---------------------------------------------------------------------------
TPtree &SettingsManager::getProperties(const QString &aAdapterName) {
    if (aAdapterName.isEmpty()) {
        return m_Properties;
    }

    static TPtree emptyTree;
    return m_Properties.get_child(aAdapterName.toStdString(), emptyTree);
}

//---------------------------------------------------------------------------
bool SettingsManager::isEqual(const SettingsManager &aManager) const {
    return m_Properties == aManager.m_Properties;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Загружает настройки из списка источников (файлов).
// ВАЖНО: Если один и тот же ключ встречается в нескольких файлах (например, system.ini и user.ini),
// то значения из файлов, загруженных позже, будут добавлены в дерево свойств последними и могут
// переопределять предыдущие при чтении. Таким образом, пользовательские настройки (user.ini) могут
// переопределять системные (system.ini), если порядок загрузки соблюдён. Это обеспечивает гибкую
// иерархию конфигурации.
bool SettingsManager::loadSettings(const QList<SSettingsSource> &aSettingSources) {
    foreach (const SSettingsSource &source, aSettingSources) {
        QElapsedTimer elapsed;
        elapsed.start();

        QFileInfo path(QDir::isAbsolutePath(source.configFileName)
                           ? source.configFileName
                           : m_ConfigPath + "/" + source.configFileName);

        toLog(LogLevel::Normal, QString("Loading configuration file %1.").arg(path.filePath()));

        // Проверяем наличие ветки.
        TPtree newBranch;

        if (source.isSymlink()) {
            newBranch.put(source.symlinkName.toStdString(), path.filePath().toStdWString());
        } else if (!path.suffix().compare("xml", Qt::CaseInsensitive)) {
            readXML(path.filePath(), newBranch);
        } else if (!path.suffix().compare("ini", Qt::CaseInsensitive)) {
            readINI(path.filePath(), newBranch);
        } else {
            toLog(LogLevel::Error,
                  QString("Usupported file extension %1.").arg(source.configFileName));
            continue;
        }

        SSettingsSource workingSource(source);
        std::string branchName = source.adapterName.toStdString();

        TPtree::assoc_iterator ai = m_Properties.find(branchName);

        TPtree &branch = (ai != m_Properties.not_found())
                             ? m_Properties.to_iterator(ai)->second
                             : m_Properties.push_back(std::make_pair(branchName, TPtree()))->second;

        // Сохраняем имена полей, которые были подгружены.
        BOOST_FOREACH (TPtree::value_type &value, newBranch) {
            workingSource.fieldNames.append(QString::fromStdString(value.first));

            // Вставляем настройки в общую ветку.
            // Если ключ уже существует, новое значение будет добавлено последним и может
            // переопределить старое при чтении.
            TPtree &tree = branch.push_back(std::make_pair(value.first, TPtree()))->second;
            tree.swap(value.second);
        }

        m_SettingSources.append(workingSource);

        toLog(LogLevel::Debug,
              QString("Load config file %1 elapsed %2 ms.")
                  .arg(source.configFileName)
                  .arg(elapsed.elapsed()));
    }

    return true;
}

//---------------------------------------------------------------------------
bool SettingsManager::saveSettings() {
    bool result = true;

    // Сохраняем все не readOnly настройки.
    foreach (const SSettingsSource &source, m_SettingSources) {
        if (source.readOnly) {
            continue;
        }

        // Составляем новое дерево из веток, подлежащих сохранению в данном файле.
        TPtree branchToSave;

        foreach (const QString &fieldName, source.fieldNames) {
            QString path = source.adapterName + "." + fieldName;

            static TPtree emptyTreeForSave;
            branchToSave.add_child(fieldName.toStdString(),
                                   m_Properties.get_child(path.toStdString(), emptyTreeForSave));
        }

        QFileInfo path(QDir::isAbsolutePath(source.configFileName)
                           ? source.configFileName
                           : m_ConfigPath + "/" + source.configFileName);

        toLog(LogLevel::Normal, QString("Saving configuration file %1.").arg(path.filePath()));

        // Если не были указаны конкретные поля, сохраняем подветку с именем, совпадающим с именем
        // файла.
        if (source.fieldNames.empty()) {
            std::string branchName = path.baseName().toStdString();
            branchToSave.add_child(
                branchName,
                m_Properties.get_child((source.adapterName + "." + path.baseName()).toStdString()));
        }

        // Здесь храним оригинальную конфигурацию
        TPtree originalBranch;

        if (!path.suffix().compare("xml", Qt::CaseInsensitive)) {
            readXML(path.filePath(), originalBranch);

            if (originalBranch != branchToSave) {
                createBackup(path.filePath());

                if (!writeXML(path.filePath(), branchToSave)) {
                    result = false;
                    continue;
                }
            }
        } else if (!path.suffix().compare("ini", Qt::CaseInsensitive)) {
            readINI(path.filePath(), originalBranch);

            if (originalBranch != branchToSave) {
                createBackup(path.filePath());

                if (!writeINI(path.filePath(), branchToSave)) {
                    result = false;
                    continue;
                }
            }
        } else {
            result = false;
            toLog(LogLevel::Error,
                  QString("Unable to save configuration file %1: unsupported file extension.")
                      .arg(source.configFileName));
            continue;
        }
    }

    return result;
}

//---------------------------------------------------------------------------
bool SettingsManager::readXML(const QString &aFileName, TPtree &aTree) {
    QFile inputFile(aFileName);

    if (!inputFile.open(QIODevice::ReadOnly)) {
        toLog(LogLevel::Error, QString("Failed to open file: %1.").arg(aFileName));
        return false;
    }

    QXmlStreamReader xmlReader(&inputFile);

    std::vector<boost::reference_wrapper<TPtree>> stack;
    boost::reference_wrapper<TPtree> current = boost::ref(aTree);

    while (!xmlReader.atEnd()) {
        QXmlStreamReader::TokenType token = xmlReader.readNext();

        switch (token) {
        // Начало документа
        case QXmlStreamReader::StartDocument:
            break;

        // Конец документа
        case QXmlStreamReader::EndDocument:
            break;

        // Встретили открывающий тег.
        case QXmlStreamReader::StartElement: {
            QString key = xmlReader.name().toString().toLower();

            TPtree &newOne = boost::unwrap_ref(current)
                                 .push_back(std::make_pair(key.toStdString(), TPtree()))
                                 ->second;
            stack.push_back(current);
            current = boost::ref(newOne);

            // Обрабатываем список атрибутов, если такие имеются.

            QXmlStreamAttributes attributes = xmlReader.attributes();

            if (!attributes.isEmpty()) {
                TPtree &attribTree = boost::unwrap_ref(current)
                                         .push_back(std::make_pair("<xmlattr>", TPtree()))
                                         ->second;

                foreach (const QXmlStreamAttribute &attribute, attributes) {
                    attribTree.put(attribute.name().toString().toLower().toStdString(),
                                   attribute.value().toString().toStdWString());
                }
            }

            break;
        }

        // Текст внутри тегов.
        case QXmlStreamReader::Characters: {
            if (!xmlReader.isWhitespace()) {
                boost::unwrap_ref(current).put_value(xmlReader.text().toString());
            }

            break;
        }

        // Встретили закрывающий тег.
        case QXmlStreamReader::EndElement: {
            current = stack.back();
            stack.pop_back();

            break;
        }

        // Комментарий - игнорируем
        case QXmlStreamReader::Comment:
            break;

        // DTD - игнорируем
        case QXmlStreamReader::DTD:
            break;

        // Ссылка на сущность - игнорируем
        case QXmlStreamReader::EntityReference:
            break;

        // Инструкция обработки - игнорируем
        case QXmlStreamReader::ProcessingInstruction:
            break;

        // Ошибка в формате документа.
        case QXmlStreamReader::Invalid: {
            aTree.clear();

            toLog(LogLevel::Error,
                  QString("'%1' parsing error: %2, line %3, column %4.")
                      .arg(aFileName)
                      .arg(xmlReader.errorString())
                      .arg(xmlReader.lineNumber())
                      .arg(xmlReader.columnNumber()));

            return false;
        }

        // NoToken и другие непредвиденные токены
        default:
            break;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
bool SettingsManager::writeXML(const QString &aFileName, const TPtree &aTree) {
    QFile outputFile(aFileName);

    if (!outputFile.open(QIODevice::WriteOnly)) {
        toLog(LogLevel::Error, QString("Failed to open file: %1.").arg(aFileName));
        return false;
    }

    QXmlStreamWriter xmlWriter(&outputFile);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartDocument();
    writeXMLNode(xmlWriter, aTree);
    xmlWriter.writeEndDocument();

    return true;
}

//---------------------------------------------------------------------------
void SettingsManager::writeXMLNode(QXmlStreamWriter &aWriter, const TPtree &aNode) {
    BOOST_FOREACH (const TPtree::value_type &value, aNode) {
        if (value.first == "<xmlattr>") {
            BOOST_FOREACH (const TPtree::value_type &value, value.second) {
                aWriter.writeAttribute(QString::fromStdString(value.first),
                                       value.second.get_value<QString>());
            }
        } else {
            if (value.second.empty()) {
                aWriter.writeTextElement(QString::fromStdString(value.first),
                                         value.second.get_value<QString>());
            } else {
                aWriter.writeStartElement(QString::fromStdString(value.first));
                writeXMLNode(aWriter, value.second);
                aWriter.writeEndElement();
            }
        }
    }
}

//---------------------------------------------------------------------------
bool SettingsManager::readINI(const QString &aFileName, TPtree &aTree) {
    QSettings iniFile(aFileName, QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    iniFile.setIniCodec("UTF-8");
#endif

    foreach (QString key, iniFile.allKeys()) {
        QString transformedKey(key);
        // Первой секцией добавляем имя файла, иначе нельзя будет определить куда записать новую
        // секцию (ранее не описанную в файле).
        transformedKey.prepend(QFileInfo(aFileName).completeBaseName() + ".");
        transformedKey.replace('/', '.');

        // Use typeId() which is available in both Qt5.15+ and Qt6
        // QMetaType::QStringList is available in both versions
        switch (iniFile.value(key).typeId()) {
        case QMetaType::QStringList:
            aTree.add(transformedKey.toStdString(), iniFile.value(key).toStringList().join(","));
            break;

        default:
            aTree.add(transformedKey.toStdString(), iniFile.value(key).toString());
            break;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
bool SettingsManager::writeINI(const QString &aFileName, const TPtree &aTree) {
    QSettings iniFile(aFileName, QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    iniFile.setIniCodec("UTF-8");
#endif
    iniFile.clear();

    static const TPtree EmptyTree;
    const auto &sectionTree = aTree.get_child(QFileInfo(aFileName).completeBaseName().toStdString(), EmptyTree);
    for (const TPtree::value_type &value : sectionTree) {
        iniFile.beginGroup(QString::fromStdString(value.first));

        for (const TPtree::value_type &child : value.second) {
            if (!child.second.empty()) {
                toLog(LogLevel::Error,
                      "Failed to write INI file: the tree has more then 2 level hierarchy.");
                return false;
            }

            iniFile.setValue(QString::fromStdString(child.first),
                             child.second.get_value<QString>());
        }

        iniFile.endGroup();
    }

    return true;
}

//---------------------------------------------------------------------------
void SettingsManager::createBackup(const QString &aFilePath) {
    QString backupExt = QDateTime::currentDateTime().toString(".yyyy-MM-dd_hh-mm-ss") + "_backup";

    QFile::rename(aFilePath, aFilePath + backupExt);
}

//---------------------------------------------------------------------------
