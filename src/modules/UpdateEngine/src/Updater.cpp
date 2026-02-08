/* @file Система обновления. */

// Stl

#include "Updater.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkProxy>
#include <QtXml/QDomDocument>

#include <Common/Exception.h>

//---------------------------------------------------------------------------
namespace CUpdater {
const char Name[] = "Updater";
} // namespace CUpdater
#include <Common/ScopedPointerLaterDeleter.h>

#include <NetworkTaskManager/FileDownloadTask.h>
#include <NetworkTaskManager/HashVerifier.h>
#include <NetworkTaskManager/MemoryDataStream.h>
#include <numeric>

#include "Folder.h"
#include "Misc.h"
#include "Package.h"
#include "WindowsBITS.h"

Q_DECLARE_METATYPE(CUpdaterErrors::Enum)

//---------------------------------------------------------------------------
namespace CUpdater {
const int MaxFails = 18;
const int MinutesBeforeNextTry = 2;

const QByteArray HumoStatusTag = "X-Humo-Status";
const QByteArray HumoAcceptKeysTag = "X-Humo-Accepted-Keys";
const QByteArray HumoSignatureTag = "X-signature";

const QByteArray Blocked = "blocked";
const QByteArray Wait = "wait";

QString UpdaterConfigurationDir = "/update/";
QString UpdaterConfiguration = "configuration_%1";

const QString BitsJobNamePrefix = "TCUpdater_";
} // namespace CUpdater

//---------------------------------------------------------------------------
Updater::Updater(QObject *aParent)
    : QObject(aParent), mFailCount(0), mAllTasksCount(0), mProgressPercent(0),
#ifdef Q_OS_WIN32
      mBitsManager(ILog::getInstance(CUpdater::Name)), mUseBITS(true), mJobPriority(CBITS::HIGH)
#else
      mUseBITS(false), mJobPriority(0)
#endif
{
    m_NetworkTaskManager.setLog(ILog::getInstance(CUpdater::Name));

    m_NetworkTaskManager.setDownloadSpeedLimit(80);

    connect(&mProgressTimer, SIGNAL(timeout()), this, SLOT(showProgress()));
}

//---------------------------------------------------------------------------
Updater::Updater(const QString &aConfigURL,
                 const QString &aUpdateURL,
                 const QString &aVersion,
                 const QString &aAppId,
                 const QString &aConfiguration,
                 const QString &aPointId)
    : m_ConfigURL(aConfigURL), m_UpdateURL(aUpdateURL + "/" + aAppId + "/" + aConfiguration),
      m_Version(aVersion), m_AppId(aAppId), m_Configuration(aConfiguration),
      m_NetworkTaskManager(ILog::getInstance(CUpdater::Name)), mCurrentTaskSize(0),
      mWaitUpdateServer(false), mFailCount(0), m_AP(aPointId), mAllTasksCount(0),
      mProgressPercent(0),
#ifdef Q_OS_WIN32
      mBitsManager(ILog::getInstance(CUpdater::Name)), mUseBITS(true), mJobPriority(CBITS::HIGH)
#else
      mUseBITS(false), mJobPriority(0)
#endif
{
    m_NetworkTaskManager.setDownloadSpeedLimit(80);

    connect(&mProgressTimer, SIGNAL(timeout()), this, SLOT(showProgress()));
}

//---------------------------------------------------------------------------
void Updater::setProxy(const QString &aProxy) {
    if (!aProxy.isEmpty()) {
        QRegularExpression pattern("(.+):(.*):(.*):(.*):(.+)");
        auto match = pattern.match(aProxy);

        if (match.hasMatch()) {
            QNetworkProxy proxy(static_cast<QNetworkProxy::ProxyType>(match.captured(5).toInt()),
                                match.captured(1),
                                match.captured(2).toUShort(),
                                match.captured(3),
                                match.captured(4));

            m_NetworkTaskManager.setProxy(proxy);
        } else {
            Log(LogLevel::Error, QString("Failed to set up proxy: cannot parse %1.").arg(aProxy));
        }
    } else {
        Log(LogLevel::Normal, "No proxy.");
    }
}

//---------------------------------------------------------------------------
CUpdaterErrors::Enum Updater::getComponents(Updater::TComponentList &aComponents) {
    mWaitUpdateServer = false;

    aComponents.clear();

    // Получаем с сервера файл с описанием.
    NetworkTask *task = new NetworkTask();

    QUrl url = m_ConfigURL;
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("name", m_AppId);
    urlQuery.addQueryItem("conf", m_Configuration);
    urlQuery.addQueryItem("rev", m_Version);
    urlQuery.addQueryItem("AP", m_AP);
    url.setQuery(urlQuery);

    Log(LogLevel::Normal,
        QString("Downloading component descriptions from '%1'...").arg(url.toString()));

    task->setDataStream(new MemoryDataStream);
    task->setUrl(url);
    task->setType(NetworkTask::Get);
    task->getRequestHeader().insert(CUpdater::HumoAcceptKeysTag, m_AcceptedKeys.toLatin1());

    m_NetworkTaskManager.addTask(task);

    task->waitForFinished();

    if (task->getError() != NetworkTask::NoError) {
        Log(LogLevel::Error,
            QString("Failed to download component description. Error %1.")
                .arg(task->errorString()));

        mWaitUpdateServer = true;

        return CUpdaterErrors::NetworkError;
    }

    NetworkTask::TByteMap &responseHeader = task->getResponseHeader();

    // если установлен флаг блокировки обновления ПО
    if (responseHeader.contains(CUpdater::HumoStatusTag)) {
        QByteArray statusTag = responseHeader.value(CUpdater::HumoStatusTag);
        Log(LogLevel::Warning,
            QString("Download component %1: %2")
                .arg(QString::fromLatin1(CUpdater::HumoStatusTag))
                .arg(QString::fromLatin1(statusTag)));

        mWaitUpdateServer = (statusTag == CUpdater::Wait);

        return CUpdaterErrors::UpdateBlocked;
    }

    m_ComponentsSignature = QByteArray::fromPercentEncoding(
        responseHeader.value(CUpdater::HumoSignatureTag, QByteArray()));

    auto dataStream = task->getDataStream();
    m_ComponentsContent = dataStream->takeAll();
    dataStream->close();

    auto result = loadComponents(m_ComponentsContent, aComponents, m_ComponentsRevision);

    if (result == CUpdaterErrors::OK && getSavedConfigurations().isEmpty()) {
        saveUpdateConfiguration();
    }

    return result;
}

//---------------------------------------------------------------------------
QByteArray Updater::loadUpdateConfiguration(const QString &aRevision) {
    QFile file(QDir(m_WorkingDir + CUpdater::UpdaterConfigurationDir)
                   .absoluteFilePath(CUpdater::UpdaterConfiguration.arg(aRevision) + ".xml"));

    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    }

    Log(LogLevel::Error,
        QString("Failed open file '%1': %2.").arg(file.fileName()).arg(file.errorString()));
    return QByteArray();
}

//---------------------------------------------------------------------------
void Updater::saveUpdateConfiguration() {
    QDir dir(m_WorkingDir + CUpdater::UpdaterConfigurationDir);

    if (m_UpdateComponents.isEmpty()) {
        // Обновили полностью дистрибутив - чистим старые конфигурации обновления
        foreach (const auto file,
                 dir.entryInfoList(QStringList(CUpdater::UpdaterConfiguration.arg("*.*")))) {
            QFile::remove(file.absoluteFilePath());
        }
    }

    QFile file(
        dir.absoluteFilePath(CUpdater::UpdaterConfiguration.arg(m_ComponentsRevision + ".xml")));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(m_ComponentsContent);
        file.close();
    }

    QFile fileSignature(
        dir.absoluteFilePath(CUpdater::UpdaterConfiguration.arg(m_ComponentsRevision + ".ipriv")));
    if (fileSignature.open(QIODevice::WriteOnly)) {
        fileSignature.write(m_ComponentsSignature);
        fileSignature.close();
    }
}

//---------------------------------------------------------------------------
QStringList Updater::getSavedConfigurations() {
    QDir dir(m_WorkingDir + CUpdater::UpdaterConfigurationDir);
    QRegularExpression rx(QString(CUpdater::UpdaterConfiguration).arg("(.*)\\.xml"));
    QStringList revisions;

    foreach (auto cfg,
             dir.entryList(QStringList(QString(CUpdater::UpdaterConfiguration).arg("*.xml")))) {
        auto match = rx.match(cfg);
        if (match.hasMatch()) {
            revisions << match.captured(1);
        }
    }

    return revisions;
}

//---------------------------------------------------------------------------
void Updater::setWorkingDir(const QString &aDir) {
    m_WorkingDir = aDir;

    QDir dir = QDir::currentPath();

    if (!dir.exists(m_WorkingDir)) {
        dir.mkpath(m_WorkingDir);
    }
}

//---------------------------------------------------------------------------
void Updater::addComponentForUpdate(const QStringList &aComponents) {
    m_UpdateComponents.append(aComponents);
}

//---------------------------------------------------------------------------
TFileList Updater::getWorkingDirStructure() const noexcept(false) {
    return getWorkingDirStructure("");
}

//---------------------------------------------------------------------------
void Updater::addExceptionDirs(const QStringList &aDirs) {
    foreach (auto dir, aDirs) {
        QString cleanedDir = QString(dir).replace(QRegularExpression("^/+|/+$"), "");
        m_ExceptionDirs.push_back(QString("/") + cleanedDir);
    }
}

//---------------------------------------------------------------------------
TFileList Updater::getWorkingDirStructure(const QString &aDir) const noexcept(false) {
    TFileList list;

    if (m_ExceptionDirs.contains(aDir, Qt::CaseInsensitive)) {
        return list;
    }

    QDir current(m_WorkingDir + "/" + aDir);

    foreach (auto fileInfo,
             current.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files)) {
        if (fileInfo.isFile()) {
            // Вычисляем контрольную сумму.
            QString path = current.filePath(fileInfo.fileName());
            QFile file(path);

            if (file.open(QIODevice::ReadOnly)) {
                auto filePath = aDir + "/" + fileInfo.fileName();

#if QT_VERSION >= 0x050000
                list.insert(File(filePath.replace(QRegularExpression("^/+"), ""),
                                 QString::fromLatin1(QCryptographicHash::hash(
                                                         file.readAll(), QCryptographicHash::Sha256)
                                                         .toHex()),
                                 "",
                                 fileInfo.size()));
#else
                list.insert(File(filePath.remove(QRegularExpression("^/+"), ""),
                                 QString::fromLatin1(CCryptographicHash::hash(
                                                         file.readAll(), CCryptographicHash::Sha256)
                                                         .toHex()),
                                 "",
                                 fileInfo.size()));
#endif
            } else {
                throw Exception(
                    ECategory::Application,
                    ESeverity::Major,
                    0,
                    QString("Failed to calculate checksum for file %1.").arg(fileInfo.filePath()));
            }
        } else if (fileInfo.isDir()) {
            list += getWorkingDirStructure(aDir + "/" + fileInfo.fileName());
        }
    }

    return list;
}

//-------------------------------------------------------------------------
void Updater::copyFiles(const QString &aSrcDir,
                        const QString &aDstDir,
                        const TFileList &aFiles,
                        bool aIgnoreError) noexcept(false) {
    Log(LogLevel::Normal, QString("Copy files from '%1' to '%2'.").arg(aSrcDir).arg(aDstDir));

    foreach (auto file, aFiles) {
        QString dstFilePath = aDstDir + "/" + file.name();

        // Создаем директорию назначения.
        if (!QDir().mkpath(dstFilePath.section("/", 0, -2))) {
            if (aIgnoreError) {
                Log(LogLevel::Warning,
                    QString("Failed to create destination path %1.").arg(dstFilePath));
            } else {
                throw Exception(ECategory::Application,
                                ESeverity::Major,
                                0,
                                QString("Failed to create destination path %1.").arg(dstFilePath));
            }
        }

        // Копируем файл.
        if (!QFile::copy(aSrcDir + "/" + file.name(), aDstDir + "/" + file.name())) {
            if (aIgnoreError) {
                Log(LogLevel::Warning, QString("Failed to copy file %1.").arg(file.name()));
            } else {
                throw Exception(ECategory::Application,
                                ESeverity::Major,
                                0,
                                QString("Failed to copy file %1").arg(file.name()));
            }
        }
    }
}

//---------------------------------------------------------------------------
void Updater::deleteFiles(const TFileList &aFiles, bool aIgnoreError) noexcept(false) {
    foreach (auto file, aFiles) {
        if (QFile::exists(m_WorkingDir + "/" + file.name()) &&
            !m_ExceptionDirs.contains("/" + file.dir(), Qt::CaseInsensitive)) {
            Log(LogLevel::Normal, QString("Deleting file %1.").arg(file.name()));

            if (!QFile::remove(m_WorkingDir + "/" + file.name())) {
                if (aIgnoreError) {
                    Log(LogLevel::Warning, QString("Failed to remove file %1.").arg(file.name()));
                } else {
                    throw Exception(ECategory::Application,
                                    ESeverity::Major,
                                    0,
                                    QString("Failed to remove file %1").arg(file.name()));
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
void Updater::download() {
    // Если список пуст, инициируем следующий шаг обновления.
    if (m_ActiveTasks.empty()) {
        mProgressTimer.stop();

        Log(LogLevel::Normal, "Download complete.");

        emit downloadAccomplished();

        return;
    }

#ifdef Q_OS_WIN32
    if (!bitsDownload()) {
#else
    if (true) { // Always use network download on non-Windows
#endif
        auto task = m_ActiveTasks.front();
        task->connect(
            task, SIGNAL(onComplete()), this, SLOT(downloadComplete()), Qt::UniqueConnection);

        mCurrentTaskSize = task->getDataStream()->size();
        m_NetworkTaskManager.addTask(task);

        Log(LogLevel::Normal, QString("%1 downloading...").arg(task->getUrl().toString()));
    }
}

//---------------------------------------------------------------------------
void closeFileTask(NetworkTask *aTask) {
    auto fileTask = qobject_cast<FileDownloadTask *>(aTask);
    if (fileTask) {
        fileTask->closeFile();
    }
}

//---------------------------------------------------------------------------
void Updater::downloadComplete() {
    auto task = m_ActiveTasks.front();
    task->disconnect(this, SLOT(downloadComplete()));

    auto goToNextFile = [&]() {
        // Удаляем старое задание.
        task->getDataStream()->close();
        m_ActiveTasks.pop_front();
        mFailCount = 0;

        // Продолжаем закачку другого файла.
        QMetaObject::invokeMethod(this, "download", Qt::QueuedConnection);
    };

    if (!task->getError() || task->getError() == NetworkTask::TaskFailedButVerified) {
        Log(LogLevel::Normal,
            QString("File %1 downloaded successfully.").arg(task->getUrl().toString()));

        closeFileTask(task);

        return goToNextFile();
    }

    int nextTryTimeout = CUpdater::MinutesBeforeNextTry;

    Log(LogLevel::Error,
        QString("Failed to download file %1. Error: %2. Http code: %3")
            .arg(task->getUrl().toString())
            .arg(task->errorString())
            .arg(task->getHttpError()));

    bool haveNewData = (mCurrentTaskSize != task->getDataStream()->size());
    bool retryCountReached = ++mFailCount >= CUpdater::MaxFails;

    if (task->getError() == NetworkTask::VerifyFailed ||
        task->getHttpError() == 416) // 416 - Requested Range Not Satisfiable
    {
        checkTaskVerifierResult(task);

        nextTryTimeout = 1;

        if (task->property(CComponent::OptionalTask()).toBool()) {
            Log(LogLevel::Normal,
                QString("File %1 is optional. Skip it and continue to download.")
                    .arg(task->getUrl().toString()));
            return goToNextFile();
        }
    } else {
        retryCountReached = retryCountReached && !haveNewData;
    }

    if (!retryCountReached) {
        if ((task->getHttpError() / 100) == 2) // HTTP 2xx
        {
            // При успешном ответе сервера продолжаем докачку файла незамедлительно
            Log(LogLevel::Error, "Continue download...");

            nextTryTimeout = 1000;
        } else {
            Log(LogLevel::Error,
                QString("Waiting %1 minutes before next try...").arg(nextTryTimeout));

            nextTryTimeout *= 60 * 1000;
        }

        // Делаем повторную попытку скачать файл через несколько минут.
        QTimer::singleShot(nextTryTimeout, this, SLOT(download()));
    } else {
        if (task->property(CComponent::OptionalTask()).toBool()) {
            Log(LogLevel::Normal,
                QString("File %1 is optional. Skip it and continue to download.")
                    .arg(task->getUrl().toString()));

            QMetaObject::invokeMethod(task, "resetFile", Qt::DirectConnection);
            return goToNextFile();
        }

        // Закачка была прервана.
        Log(LogLevel::Error,
            QString("Download terminated after %1 attempts.").arg(CUpdater::MaxFails));

        mProgressTimer.stop();
        emit done(CUpdaterErrors::NetworkError);
    }
}

//---------------------------------------------------------------------------
void Updater::checkTaskVerifierResult(NetworkTask *aTask) {
    IHashVerifier *verifier = dynamic_cast<IHashVerifier *>(aTask->getVerifier());

    if (verifier) {
        Log(LogLevel::Error,
            QString(
                "Failed verify. Downloaded file hash:%1 required_hash:%2. Remove temporary file")
                .arg(verifier->calculatedHash())
                .arg(verifier->referenceHash()));
    } else {
        Log(LogLevel::Error, "Failed verify downloaded file. Remove temporary file");
    }

    QMetaObject::invokeMethod(aTask, "resetFile", Qt::DirectConnection);

    mCurrentTaskSize = 0;
}

//---------------------------------------------------------------------------
void Updater::showProgress() {
    if (mAllTasksCount > 0) {
        mProgressPercent = (mAllTasksCount - m_ActiveTasks.size()) * 100 / mAllTasksCount;
    } else {
        mProgressPercent = ++mProgressPercent > 100 ? 1 : mProgressPercent;
    }

    emit progress(mProgressPercent);
}

//---------------------------------------------------------------------------
void Updater::downloadComponents(const TComponentList &aComponents) {
    m_Components = aComponents;

    try {
        Log(LogLevel::Normal, "Calculating local files checksum.");
        TFileList currentStructure = getWorkingDirStructure();

        // Формируем список файлов для загрузки.
        foreach (auto comp, m_Components) {
            m_ActiveTasks +=
                comp->download(m_UpdateURL, comp->getFiles().intersect(currentStructure));
        }

        mAllTasksCount = m_ActiveTasks.size();
        mProgressTimer.start(3 * 60 * 1000);

        // Запускаем загрузку.
        QMetaObject::invokeMethod(this, "download", Qt::QueuedConnection);
    } catch (Exception &e) {
        Log(LogLevel::Fatal, e.getMessage());
    }
}

//---------------------------------------------------------------------------
bool Updater::haveSkippedComponents() const {
    return !m_UpdateComponents.isEmpty();
}

//---------------------------------------------------------------------------
TFileList Updater::intersectByName(const TFileList &aList1, const TFileList &aList2) {
    QSet<QString> list2Names;

    foreach (auto file, aList2) {
        list2Names.insert(file.name());
    }

    TFileList result;

    foreach (auto file, aList1) {
        if (list2Names.contains(file.name())) {
            result.insert(file);
        }
    }

    return result;
}

//---------------------------------------------------------------------------
void Updater::substractByName(TFileList &aList1, const TFileList &aList2) {
    QSet<QString> list2Names;

    foreach (auto file, aList2) {
        list2Names.insert(file.name());
    }

    TFileList result;

    foreach (auto file, aList1) {
        if (list2Names.contains(file.name())) {
            result.insert(file);
        }
    }

    aList1.subtract(result);
}

//---------------------------------------------------------------------------
void Updater::deploy() {
    TFileList downloadedFiles;
    TFileList oldFiles;
    TFileList newFiles;

    Log(LogLevel::Normal, "Start deploy.");

    QString backupDir =
        m_WorkingDir + "/backup/" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

    try {
        Log(LogLevel::Normal, QString("Backup into '%1'.").arg(backupDir));

        // Создаем временную папку для бекапов.
        if (!QDir().mkpath(backupDir)) {
            throw Exception(ECategory::Application,
                            ESeverity::Major,
                            0,
                            QString("Failed to create path %1.").arg(backupDir));
        }

        // Делаем резервную копию.
        foreach (auto comp, m_Components) {
            downloadedFiles += comp->getFiles();
        }

        // Формируем список файлов для удаления (старые версии + мусор).
        auto currentFiles = getWorkingDirStructure();

        if (haveSkippedComponents()) {
            // при частичном обновлении отдельных компонент мы заменяем только файлы, содержащиеся в
            // скачиваемых компонентах
            oldFiles = intersectByName(currentFiles, downloadedFiles);
        } else {
            oldFiles = currentFiles;
        }
        oldFiles.subtract(downloadedFiles);

        // убираем из списка старых файлов все файлы компонент, имеющих флаг skip_existing
        foreach (auto comp, m_Components) {
            if (comp->skipExisting() || comp->optional()) {
                substractByName(oldFiles, comp->getFiles());
            }
        }

        newFiles = downloadedFiles;
        newFiles.subtract(currentFiles);

        // Делаем резервные копии.
        copyFiles(m_WorkingDir, backupDir, oldFiles);
    } catch (Exception &e) {
        // На этом этапе дальнейшая установка обновления невозможна.
        Log(LogLevel::Fatal, e.getMessage());

        emit done(CUpdaterErrors::UnknownError);

        return;
    }

    // Производим установку обновления.
    try {
        Log(LogLevel::Normal, "Removing old files.");

        // Удаляем файлы.
        deleteFiles(oldFiles);

        Log(LogLevel::Normal, "Deploy new files.");

        // Копируем новые файлы.
        foreach (auto comp, m_Components) {
            comp->deploy(comp->getFiles().intersect(newFiles), m_WorkingDir);
        }

        Log(LogLevel::Normal, "Applying post actions.");

        // Выполняем общие действия.
        foreach (auto comp, m_Components) {
            comp->applyPostActions(m_WorkingDir);
        }

        Log(LogLevel::Normal, "Removing empty folders.");

        removeEmptyFolders(m_WorkingDir);

        // Сохраняем успешную конфигурацию на диск
        saveUpdateConfiguration();

        // Завершаем работу приложения.
        emit done(CUpdaterErrors::OK);
    } catch (Exception &e) {
        Log(LogLevel::Fatal, QString("Failed to deploy update: %1.").arg(e.getMessage()));

        Log(LogLevel::Normal, "Restoring backup.");

        try {
            Log(LogLevel::Normal, "Deleting new files.");
            // Удаляем установленные файлы.
            deleteFiles(newFiles, true);

            Log(LogLevel::Normal, "Restoring old files from backup.");
            // Восстанавливаем удаленные файлы.
            copyFiles(backupDir, m_WorkingDir, oldFiles, true);
        } catch (Exception &e) {
            Log(LogLevel::Fatal, QString("Failed to restore backup. %1.").arg(e.getMessage()));
        }

        emit done(CUpdaterErrors::DeployError);
    }
}

//---------------------------------------------------------------------------
int Updater::checkIntegrity() {
    QMultiMap<QString, QString> files;

    foreach (QString revision, getSavedConfigurations()) {
        Log(LogLevel::Normal,
            QString("Loading package description... Revision: %1.").arg(revision));

        Updater::TComponentList components;
        auto result = loadComponents(loadUpdateConfiguration(revision), components, revision);

        if (result != CUpdaterErrors::OK) {
            Log(LogLevel::Error,
                QString("Failed to load package description revision: %1.").arg(revision));

            return -1;
        }

        foreach (auto comp, components) {
            // Выкидываем все опциональные и конфигурационные пакеты
            if (!comp->optional() && !comp->skipExisting()) {
                foreach (const File &file, comp->getFiles()) {
                    files.insert(file.name(), file.hash());
                }
            }
        }
    }

    if (files.isEmpty()) {
        Log(LogLevel::Error, "Failed to load package description.");

        return -1;
    }

    try {
        Log(LogLevel::Normal, "Calculating local files checksum.");
        TFileList currentStructure = getWorkingDirStructure();
        int diffFilesCount = 0;

        // Пересчитываем отличающиеся файлы.
        foreach (const File &file, currentStructure) {
            if (files.contains(file.name())) {
                if (!files.values(file.name()).contains(file.hash())) {
                    Log(LogLevel::Error, QString("Different local file: %1.").arg(file.name()));

                    diffFilesCount++;
                }

                files.remove(file.name());
                currentStructure.remove(file);
            }
        }

        // Пересчитываем лишние файлы.
        foreach (const File &file, currentStructure) {
            Log(LogLevel::Warning, QString("Unwanted local file: %1.").arg(file.name()));
        }

        return diffFilesCount;
    } catch (Exception &e) {
        Log(LogLevel::Fatal, e.getMessage());

        return -1;
    }
}

//---------------------------------------------------------------------------
#ifdef Q_OS_WIN32
void Updater::useBITS(bool aUseBITS, int aJobPriority) {
    mUseBITS = aUseBITS;
    mJobPriority = aJobPriority;
}
#endif

//---------------------------------------------------------------------------
void Updater::runUpdate() {
    Log(LogLevel::Normal, "Downloading package description...");

    Updater::TComponentList components;
    auto error = getComponents(components);

    switch (error) {
    case CUpdaterErrors::OK:
        Log(LogLevel::Normal,
            QString("Updating components:%1")
                .arg(std::accumulate(components.begin(),
                                     components.end(),
                                     QString(),
                                     [](const QString &str, const QSharedPointer<Component> &comp) {
                                         return str + " " + comp->getId();
                                     })));

        downloadComponents(components);
        break;

    default:
        if (mWaitUpdateServer && mFailCount < CUpdater::MaxFails) {
            ++mFailCount;

            emit updateSystemIsWaiting();
        } else {
            Log(LogLevel::Error, "Failed to download package description.");

            emit done(error);
        }
        break;
    }
}

//---------------------------------------------------------------------------
int Updater::removeEmptyFolders(const QString &aDir) {
    QDir current(aDir);

    int numFiles = 0;

    foreach (auto fileInfo,
             current.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files)) {
        if (fileInfo.isDir()) {
            int n = removeEmptyFolders(aDir + "/" + fileInfo.fileName());

            if (n == 0) {
                current.rmpath(aDir + "/" + fileInfo.fileName());
            } else {
                numFiles++;
            }
        } else {
            numFiles++;
        }
    }

    return numFiles;
}

//---------------------------------------------------------------------------
void Updater::setMD5(const QString &aMD5) {
    m_MD5 = aMD5;
}

//---------------------------------------------------------------------------
void Updater::downloadPackage() {
    Package *package = nullptr;
    QList<NetworkTask *> tasks;

    if (!m_ConfigURL.contains("=") && !m_ConfigURL.contains("?")) {
        // Фиктивный список файлов (реальный состав может отличаться).
        QString fileName = m_ConfigURL.section("/", -1, -1);
        QFileInfo fileInfo(fileName);
        auto fileList = TFileList() << File("config.xml", "", "");
        package =
            new Package(fileInfo.completeBaseName(), "1", fileList, QStringList(), "", m_MD5, 0);
        tasks = package->download(m_ConfigURL + "?", TFileList());
    } else {
        // Обманываем алгоритм для скачивания файла через запрос к скрипту
        auto fileList = TFileList() << File("config.xml", "", "");
        package = new Package(m_MD5, "1", fileList, QStringList(), "", m_MD5, 0);
        tasks = package->download(m_ConfigURL, TFileList());
    }

    if (tasks.isEmpty()) {
        // файл уже скачан
        Log(LogLevel::Normal, QString("File %1.zip was already downloaded.").arg(package->getId()));

        QMetaObject::invokeMethod(
            this, "deployDownloadedPackage", Qt::QueuedConnection, Q_ARG(QObject *, package));
    } else {
        auto task = tasks.at(0);

        mMapper.setMapping(task, package);
        mMapper.connect(task, SIGNAL(onComplete()), SLOT(map()));
        connect(&mMapper,
                SIGNAL(mapped(QObject *)),
                SLOT(packageDownloaded(QObject *)),
                Qt::UniqueConnection);

        Log(LogLevel::Normal, QString("Downloading file %1...").arg(m_ConfigURL));

        // Запускаем закачку.
        mCurrentTaskSize = task->getDataStream()->size();
        m_NetworkTaskManager.addTask(task);
    }
}

//---------------------------------------------------------------------------
void Updater::packageDownloaded(QObject *aPackage) {
    auto task = qobject_cast<NetworkTask *>(mMapper.mapping(aPackage));
    auto package = qobject_cast<Package *>(aPackage);

    bool haveNewData = (mCurrentTaskSize != task->getDataStream()->size());
    bool retryCountReached = ++mFailCount >= CUpdater::MaxFails;

    if (!task->getError() || task->getError() == NetworkTask::TaskFailedButVerified) {
        Log(LogLevel::Normal,
            QString("File %1.zip was downloaded successfully.").arg(package->getId()));

        closeFileTask(task);

        deployDownloadedPackage(package);
    } else {
        Log(LogLevel::Error,
            QString("Failed to download file %1. Network error: %2.")
                .arg(package->getId())
                .arg(task->errorString()));

        if (task->getError() &&
            task->getHttpError() == 416) // 416 - Requested Range Not Satisfiable
        {
            checkTaskVerifierResult(task);
        }

        if (retryCountReached) {
            emit done(CUpdaterErrors::NetworkError);
        } else {
            int timeout = CUpdater::MinutesBeforeNextTry * 60 * 1000;

            if ((task->getHttpError() / 100) == 2) {
                timeout = 1000;
                Log(LogLevel::Error, "Continue download package...");
            } else {
                Log(LogLevel::Error,
                    QString("Waiting %1 minutes before next try...")
                        .arg(CUpdater::MinutesBeforeNextTry));
            }

            closeFileTask(task);
            QTimer::singleShot(timeout, this, SLOT(downloadPackage()));
        }
    }

    task->deleteLater();
    package->deleteLater();
}

//---------------------------------------------------------------------------
void Updater::deployDownloadedPackage(QObject *aPackage) {
    emit deployment();

    auto package = qobject_cast<Package *>(aPackage);

    // Распаковываем архив.
    try {
        package->deploy(package->getFiles(), m_WorkingDir);

        Log(LogLevel::Normal,
            QString("File %1.zip was successfully unpacked.").arg(package->getId()));

        emit done(CUpdaterErrors::OK);
    } catch (Exception &e) {
        Log(LogLevel::Fatal,
            QString("Failed to deploy file %1.zip. (%2)")
                .arg(package->getId())
                .arg(e.getMessage()));

        emit done(CUpdaterErrors::DeployError);
    }
}

//---------------------------------------------------------------------------
void Updater::setOptionalComponents(const QStringList &aComponents) {
    m_OptionalComponents = aComponents;
}

//---------------------------------------------------------------------------
void Updater::setConfigurationRequiredFiles(const QStringList &aRequiredFiles) {
    m_RequiredFiles = aRequiredFiles;
}

//---------------------------------------------------------------------------
bool Updater::validateConfiguration(const TComponentList &aComponents) {
    if (aComponents.isEmpty()) {
        Log(LogLevel::Error, "Update configuration not valid: empty component list.");

        return false;
    }

    foreach (auto requiredFile, m_RequiredFiles) {
        bool exist = false;

        foreach (auto component, aComponents) {
            auto files = component->getFiles();

            exist |= (std::find_if(files.begin(), files.end(), [&](const File &aFile) -> bool {
                          return aFile.name().endsWith(requiredFile, Qt::CaseInsensitive);
                      }) != files.end());
        }

        if (!exist) {
            Log(LogLevel::Error,
                QString("Update configuration not valid: not exist '%1' file.").arg(requiredFile));

            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
#ifdef Q_OS_WIN32
bool Updater::bitsDownload() {
    if (!mBitsManager.isReady() || !mUseBITS) {
        return false;
    }

    if (bitsInProgress()) {
        return true;
    }

    bitsCleanupOldTasks();

    // Save job info into ini file with current updater arguments
    bitsSaveState();

    // Устанавливаем после выполнения задачи запуск себя с теми же самыми параметрами
    mBitsManager.setNotify(qApp->applicationFilePath(),
                           QString("--command bits --workdir %1").arg(m_WorkingDir));

    CBITS::SJob job;
    if (mBitsManager.createJob(bitsJobName(), job, mJobPriority)) {
        foreach (auto task, m_ActiveTasks) {
            auto fileTask = dynamic_cast<FileDownloadTask *>(task);
            if (fileTask) {
                fileTask->closeFile();
                if (!mBitsManager.addTask(fileTask->getUrl(), fileTask->getPath())) {
                    Log(LogLevel::Error, "Error add task to BITS job.");

                    mBitsManager.shutdown();
                    // перейти обратно к схеме скачивания вручную
                    return false;
                }
            }
        }

        // Запускаем на скачивание задание
        if (mBitsManager.resume()) {
            Log(LogLevel::Normal, QString("BITS job '%1' create successful.").arg(bitsJobName()));

            // закрываем updater с кодом - "команда выполняется"
            emit done(CUpdaterErrors::BitsInProgress);
            return true;
        } else {
            Log(LogLevel::Error, QString("Error resume BITS job '%1'.").arg(bitsJobName()));
        }
    } else {
        Log(LogLevel::Error, "Error create BITS job.");
    }

    mBitsManager.shutdown();

    // перейти обратно к схеме скачивания вручную
    return false;
}
#endif

//---------------------------------------------------------------------------
#ifdef Q_OS_WIN32
void Updater::bitsCompleteAllJobs(int &aCount, int &aCountComplete, int &aCountError) {
    auto jobs = mBitsManager.getJobs(bitsJobName());
    aCount = jobs.size();
    aCountComplete = 0;
    aCountError = 0;

    auto complete = [this](const CBITS::SJob &job) -> bool {
        if (!mBitsManager.openJob(job)) {
            Log(LogLevel::Error, QString("BITS job '%1' error open.").arg(job.mName));

            return false;
        }

        if (!mBitsManager.complete()) {
            Log(LogLevel::Error, QString("BITS job '%1' failed complete.").arg(job.mName));

            return false;
        }

        return true;
    };

    foreach (QString jobName, jobs.keys()) {
        // Проверяем состояние таска
        auto job = jobs.value(jobName);

        if (job.isComplete()) {
            Log(LogLevel::Normal, QString("BITS job '%1' download complete.").arg(job.mName));

            if (!complete(job)) {
                aCountError++;
            } else {
                aCountComplete++;
            }
        } else if (job.isFatal()) {
            // Всё равно коммитим то, что удалось скачать
            Log(LogLevel::Error, QString("BITS job '%1' failed.").arg(job.mName));

            complete(job);

            aCountError++;
        } else {
            Log(LogLevel::Normal, QString("BITS job '%1' in progress.").arg(job.mName));
        }
    }
}
#endif

//---------------------------------------------------------------------------
#ifndef Q_OS_WIN32
void Updater::bitsCompleteAllJobs(int &aCount, int &aCountComplete, int &aCountError) {
    aCount = 0;
    aCountComplete = 0;
    aCountError = 0; // BITS not available on this platform
}
#endif

//---------------------------------------------------------------------------
#ifdef Q_OS_WIN32
bool Updater::bitsInProgress() {
    int count = 0;
    int countComplete = 0;
    int countError = 0;

    bitsCompleteAllJobs(count, countError, countComplete);

    if (countError) {
        bitsCleanupOldTasks();

        // возвращаем ошибку скачивания задания
        emit done(CUpdaterErrors::NetworkError);
        return true;
    } else if (count && countComplete == count) {
        // передаем управление дальше на распаковку
        emit downloadAccomplished();
        return true;
    } else if (count) {
        // Скачивание в процессе, просто выходим со статусом "в процессе"
        emit done(CUpdaterErrors::BitsInProgress);
        return true;
    }

    return false;
}
#endif

//---------------------------------------------------------------------------
#ifndef Q_OS_WIN32
bool Updater::bitsInProgress() {
    return false; // BITS not available on this platform
}
#endif

//---------------------------------------------------------------------------
#ifdef Q_OS_WIN32
void Updater::bitsCleanupOldTasks() {
    Log(LogLevel::Normal, QString("Cancel all bits jobs."));

    // Останавливаем все наши таски от предыдущих версий.
    auto jobs = mBitsManager.getJobs(CUpdater::BitsJobNamePrefix);

    foreach (const QString &jobName, jobs.keys()) {
        if (jobName.startsWith(CUpdater::BitsJobNamePrefix) &&
            mBitsManager.openJob(jobs[jobName])) {
            Log(LogLevel::Normal, QString("Cancel old bits job '%1'.").arg(jobName));

            mBitsManager.cancel();
        }
    }
}
#endif

//---------------------------------------------------------------------------
#ifndef Q_OS_WIN32
void Updater::bitsCleanupOldTasks() {
    // BITS not available on this platform - no cleanup needed
}
#endif

//---------------------------------------------------------------------------
QString Updater::bitsJobName() const {
    return CUpdater::BitsJobNamePrefix +
           QString("%1_%2_%3").arg(m_AppId).arg(m_Configuration).arg(m_Version);
}

//---------------------------------------------------------------------------
CUpdaterErrors::Enum Updater::loadComponents(const QByteArray &aContent,
                                             Updater::TComponentList &aComponents,
                                             QString &aRevision) {
    QDomDocument description;

    if (!description.setContent(aContent)) {
        Log(LogLevel::Error, "Failed to parse component description.");

        return CUpdaterErrors::ParseError;
    }

    QDomElement application = description.documentElement();

    QString revision = application.attribute("revision", "");

    if (revision.isEmpty()) {
        Log(LogLevel::Error, "Revision number is missing.");
        return CUpdaterErrors::ParseError;
    }

    aRevision = revision;

    // Все компоненты из файла
    Updater::TComponentList allComponents;

    QRegularExpression leadingSlash("^[\\\\/]");

    // Получаем список компонент.
    for (QDomNode node = application.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement component = node.toElement();

        if (component.tagName() == "component") {
            auto componentType = component.attribute("type");
            auto componentName = component.attribute("name");
            auto componentUrl = component.attribute("url");
            auto componentSize = component.attribute("size").toInt();
            auto componentHash = component.attribute("hash_sha256");
            auto skipExisting = component.attribute("skip_existing");
            auto componentOptional = component.attribute("optional");

            // Получаем список файлов и действий.
            TFileList files;
            QStringList actions;

            for (QDomNode node = component.firstChild(); !node.isNull();
                 node = node.nextSibling()) {
                auto record = node.toElement();

                if (record.tagName() == "file") {
                    files.insert(File(record.attribute("path").replace(leadingSlash, ""),
                                      record.attribute("hash_sha256"),
                                      record.attribute("url"),
                                      record.attribute("size").toInt()));
                    continue;
                }

                if (record.tagName() == "post-action") {
                    auto name = record.attribute("path").replace(leadingSlash, "");
                    // auto url = record.attribute("url");

                    actions.append(name);
                    continue;
                }
            }

            QSharedPointer<Component> newComponent;

            // Создаем нужный класс в зависимости от типа компонента.
            if (componentType == "package") {
                newComponent = QSharedPointer<Component>(new Package(componentName,
                                                                     aRevision,
                                                                     files,
                                                                     actions,
                                                                     componentUrl,
                                                                     componentHash,
                                                                     componentSize));
            } else if (componentType == "folder") {
                newComponent = QSharedPointer<Component>(
                    new Folder(componentName, aRevision, files, actions, componentUrl));
            }

            if (newComponent) {
                newComponent->setOptional(
                    m_OptionalComponents.contains(componentName, Qt::CaseInsensitive) ||
                    componentOptional.contains("true", Qt::CaseInsensitive));
                newComponent->setSkipExisting(skipExisting.contains("true", Qt::CaseInsensitive));

                // если список разрешенных компонент пустой или в нем есть текущая компонента, то
                // добавляем её в список
                if (m_UpdateComponents.isEmpty() ||
                    m_UpdateComponents.contains(componentName, Qt::CaseInsensitive)) {
                    aComponents.append(newComponent);
                }

                allComponents.append(newComponent);

                continue;
            }

            Log(LogLevel::Error,
                QString("Component %1 type is unknown: %2.").arg(componentName).arg(componentType));
        }
    }

    return validateConfiguration(allComponents) ? CUpdaterErrors::OK : CUpdaterErrors::ParseError;
}

//---------------------------------------------------------------------------
void Updater::setAcceptedKeys(const QString &aAcceptedKeys) {
    m_AcceptedKeys = aAcceptedKeys;
}

//---------------------------------------------------------------------------
void Updater::bitsSaveState() {
    QSettings settings(
        QDir(m_WorkingDir + CUpdater::UpdaterConfigurationDir).absoluteFilePath("bits.ini"),
        QSettings::IniFormat);

    settings.beginGroup("bits");
    settings.setValue("job_name", bitsJobName());
    settings.setValue("create_stamp", QDateTime::currentMSecsSinceEpoch());
    settings.setValue("create_stamp_for_user",
                      QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
    settings.endGroup();

    settings.beginGroup("updater");
    settings.setValue("working_dir", m_WorkingDir);
    settings.setValue("config_url", m_ConfigURL);
    settings.setValue("update_url", m_UpdateURL);
    settings.setValue("version", m_Version);
    settings.setValue("app_id", m_AppId);
    settings.setValue("configuration", m_Configuration);
    settings.setValue("ap", m_AP);
    settings.endGroup();

    auto params = qApp->arguments();
    params.takeFirst();

    settings.beginWriteArray("parameters");
    for (int i = 0; i < params.size(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("arg", params[i]);
    }
    settings.endArray();
}

//---------------------------------------------------------------------------
bool Updater::bitsLoadState(QStringList *aParameters) {
    QSettings settings(
        QDir(m_WorkingDir + CUpdater::UpdaterConfigurationDir).absoluteFilePath("bits.ini"),
        QSettings::IniFormat);

    if (settings.status() != QSettings::NoError) {
        return false;
    }

    settings.beginGroup("updater");
    m_WorkingDir = settings.value("working_dir").toString();
    m_ConfigURL = settings.value("config_url").toString();
    m_UpdateURL = settings.value("update_url").toString();
    m_Version = settings.value("version").toString();
    m_AppId = settings.value("app_id").toString();
    m_Configuration = settings.value("configuration").toString();
    m_AP = settings.value("ap").toString();
    settings.endGroup();

    if (aParameters) {
        int count = settings.beginReadArray("parameters");
        for (int i = 0; i < count; i++) {
            settings.setArrayIndex(i);
            aParameters->append(settings.value("arg").toString());
        }
        settings.endArray();
    }

    return !m_AppId.isEmpty() && !m_Configuration.isEmpty() && !m_Version.isEmpty();
}

//---------------------------------------------------------------------------
#ifdef Q_OS_WIN32
bool Updater::bitsIsComplete() {
    Log(LogLevel::Normal, QString("BITS job name: %1.").arg(bitsJobName()));

    auto jobs = mBitsManager.getJobs(bitsJobName());
    int countComplete = 0;

    foreach (QString jobName, jobs.keys()) {
        // Проверяем состояние таска
        auto job = jobs.value(jobName);

        Log(LogLevel::Normal, QString("JOB: %1 has state=%2.").arg(job.mName).arg(job.mState));

        if (job.isComplete()) {
            Log(LogLevel::Normal, QString("BITS job '%1' complete.").arg(job.mName));

            countComplete++;
        }
    }

    return jobs.count() && jobs.count() == countComplete;
}
#endif

//---------------------------------------------------------------------------
#ifndef Q_OS_WIN32
bool Updater::bitsIsComplete() {
    return false; // BITS not available on this platform
}
#endif

//---------------------------------------------------------------------------
#ifdef Q_OS_WIN32
bool Updater::bitsIsError() {
    auto jobs = mBitsManager.getJobs(bitsJobName());
    int badJobs = 0;

    foreach (QString jobName, jobs.keys()) {
        // Проверяем состояние таска
        auto job = jobs.value(jobName);

        if (job.isFatal()) {
            Log(LogLevel::Normal, QString("BITS job '%1' failed.").arg(job.mName));

            if (mBitsManager.openJob(job)) {
                mBitsManager.complete();
            }

            badJobs++;
        }
    }

    if (badJobs) {
        // Если есть плохие, то остальные закрываем в любом случае.

        foreach (QString jobName, jobs.keys()) {
            // Проверяем состояние таска
            auto job = jobs.value(jobName);

            if (!job.isFatal()) {
                if (mBitsManager.openJob(job)) {
                    mBitsManager.complete();
                }
            }
        }
    }

    return badJobs > 0;
}
#endif

//---------------------------------------------------------------------------
#ifndef Q_OS_WIN32
bool Updater::bitsIsError() {
    return false; // BITS not available on this platform
}
#endif

//---------------------------------------------------------------------------
