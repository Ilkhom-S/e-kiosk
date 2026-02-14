/* @file Класс, реализующий приложение для системы обновления. */

#include "UpdaterApp.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QMovie>
#include <QtWidgets/QApplication>

#include <Common/BasicApplication.h>
#include <Common/Version.h>

#include <SDK/PaymentProcessor/Core/IRemoteService.h>

#include <SysUtils/ISysUtils.h>
#include <WatchServiceClient/Constants.h>
#include <singleapplication.h>

namespace Opt {
const char WorkDir[] = "workdir";
const char Server[] = "server";
const char UpdateUrl[] = "update-url";
const char Version[] = "version";
const char Configuration[] = "conf";
const char Command[] = "command";
const char Application[] = "application";
const char CommandID[] = "id";
const char Proxy[] = "proxy";
const char Components[] = "components";
const char PointID[] = "point";
const char MD5[] = "md5";
const char NoRestart[] = "no-restart";
const char DestinationSubdirs[] = "destination-subdir";
const char AcceptKeys[] = "accept-keys";
const char WithoutBITS[] = "no-bits";
} // namespace Opt

namespace Command {
const char Config[] = "config";
const char UserPack[] = "userpack";
const char Update[] = "update";
const char Integrity[] = "integrity";
const char Bits[] = "bits";
} // namespace Command

//---------------------------------------------------------------------------
UpdaterApp::UpdaterApp(int aArgc, char **aArgv)
    : BasicQtApplication<SafeQApplication>(CUpdater::Name, Humo::getVersion(), aArgc, aArgv),
      m_State(CUpdaterApp::Download) {
#ifdef Q_OS_WIN
    CatchUnhandledExceptions();
#endif

    m_WatchServiceClient = QSharedPointer<IWatchServiceClient>(
        ::createWatchServiceClient(CWatchService::Modules::Updater));

    m_WatchServiceClient->subscribeOnCloseCommandReceived(this);
    m_WatchServiceClient->subscribeOnDisconnected(this);
}

//---------------------------------------------------------------------------
UpdaterApp::~UpdaterApp() {
    m_WatchServiceClient->stop();

    SDK::PaymentProcessor::IRemoteService::EStatus status;

    // Обновляем отчет.
    switch (m_ResultCode_) {
    case CUpdaterApp::ExitCode::NoError:
        status = SDK::PaymentProcessor::IRemoteService::OK;
        break;

    case CUpdaterApp::ExitCode::ContinueExecution:
    case CUpdaterApp::ExitCode::WorkInProgress:
        status = SDK::PaymentProcessor::IRemoteService::Executing;
        break;

    default:
        status = SDK::PaymentProcessor::IRemoteService::Error;
    }

    m_ReportBuilder.setStatusDescription(m_ResultDescription);
    m_ReportBuilder.setStatus(status);
}

//---------------------------------------------------------------------------
QString UpdaterApp::getArgument(const char *aName,
                                const QString &aDefaultValue /*= QString()*/) const {
    auto arguments = getArguments();

    auto index = arguments.indexOf(QString("--%1").arg(aName), Qt::CaseInsensitive);
    return index < 0 ? aDefaultValue : arguments.value(index + 1, aDefaultValue);
};

//---------------------------------------------------------------------------
void UpdaterApp::onCloseCommandReceived() {
    getLog()->write(LogLevel::Normal,
                    QString("Receive close signal from watch service. State=%1.").arg(m_State));

    if (m_State == CUpdaterApp::Download) {
        getLog()->write(LogLevel::Normal,
                        "Receive close signal from watch service. Update aborted.");

        setResultCode(CUpdaterApp::ExitCode::Aborted);

        errorExit();
    }
}

//---------------------------------------------------------------------------
void UpdaterApp::onDisconnected() {
    switch (m_State) {
    case CUpdaterApp::Deploy:
        getLog()->write(LogLevel::Normal, "Disconnected from watch service. Go to deploy.");
        QMetaObject::invokeMethod(m_Updater, "deploy", Qt::QueuedConnection);
        break;

    case CUpdaterApp::Finish:
        getLog()->write(LogLevel::Normal, "Disconnected from watch service.");
        break;

    default:
        getLog()->write(LogLevel::Normal, "Disconnected from watch service. Try reconnect.");

        if (!m_WatchServiceClient->isConnected()) {
            if (!m_WatchServiceClient->start()) {
                // не удалось подключиться, пробуем еще раз
                QTimer::singleShot(1000, this, SLOT(onDisconnected()));
            } else {
                getLog()->write(LogLevel::Normal, "Connect to watch service OK.");
            }
        }
        break;
    }
}

//---------------------------------------------------------------------------
QString UpdaterApp::getWorkingDirectory() const {
    // Парсим аргументы командной строки.
    return getArgument(Opt::WorkDir, BasicQtApplication<SafeQApplication>::getWorkingDirectory());
}

//---------------------------------------------------------------------------
QStringList configStringList(const QVariant &aValue, const char *aDelimeter = ",") {
    switch (aValue.typeId()) {
    case QMetaType::QStringList:
        return aValue.toStringList();

    default:
        return aValue.toString().split(aDelimeter, Qt::SkipEmptyParts);
    }
}

//---------------------------------------------------------------------------
QStringList UpdaterApp::exceptionDirs() const {
    return QStringList() << "logs" << "backup" << "user" << "update" << "receipts" << "ad";
}

//---------------------------------------------------------------------------
void UpdaterApp::run() {
    // Парсим аргументы командной строки и конфигурацию.
    QSettings &settings = getSettings();

    QString workingDir = getArgument(Opt::WorkDir);
    // переходим в рабочую папку, как было указано в параметрах командной строки
    if (!workingDir.isEmpty() && !QDir::setCurrent(workingDir)) {
        getLog()->write(LogLevel::Error, QString("Error change current dir: %1.").arg(workingDir));
    }

    QString server = getArgument(Opt::Server);
    QString updateBaseUrl = getArgument(Opt::UpdateUrl, server);
    QString version = getArgument(Opt::Version);
    QString configuration = getArgument(Opt::Configuration);
    QString command = getArgument(Opt::Command);
    QString app = getArgument(Opt::Application);
    QString cmdId = getArgument(Opt::CommandID);
    QString proxy = getArgument(Opt::Proxy);
    QStringList components = getArgument(Opt::Components).split(",", Qt::SkipEmptyParts);
    QString pointId = getArgument(Opt::PointID);
    QString acceptKeys = getArgument(Opt::AcceptKeys);
    QString md5 = getArgument(Opt::MD5);
    // Параметр запрещающий перезагрузку ТК после скачивания файла
    bool woRestart = getArgument(Opt::NoRestart).compare("true", Qt::CaseInsensitive) == 0;
    // подпапка, в которую распаковываем архив
    QString subdir = getArgument(Opt::DestinationSubdirs);
    bool woBITS = getArgument(Opt::WithoutBITS).compare("true", Qt::CaseInsensitive) == 0;

    // Отключение BITS через updater.ini
    Q_UNUSED(woBITS);
    Q_UNUSED(settings);

    // Создаем файл с отчетом.
    m_ReportBuilder.open(cmdId, server, md5);
    m_ReportBuilder.setStatus(SDK::PaymentProcessor::IRemoteService::Executing);

    // Проверка на запуск второй копии программы (SingleApplication).
    auto *appInstance = qobject_cast<SingleApplication *>(SingleApplication::instance());
    if (appInstance && appInstance->isSecondary()) {
        getLog()->write(LogLevel::Warning, "Another instance is already running.");
        setResultCode(CUpdaterApp::ExitCode::SecondInstance);
        return;
    }

    if (command == Command::Bits) {
        m_Updater = new Updater(this);
        m_Updater->setWorkingDir(workingDir);
        // For BITS command, treat secondary instance as update in progress
        if (appInstance->isSecondary()) {
            setResultCode(CUpdaterApp::ExitCode::NoError, tr("Update in progress"));
            return;
        }
        CUpdaterApp::ExitCode::Enum result = bitsCheckStatus(false);
        if (result == CUpdaterApp::ExitCode::ContinueExecution) {
            setResultCode(CUpdaterApp::ExitCode::NoError, tr("Update in progress"));
        } else {
            setResultCode(result);
        }
        return;
    }

    // ...existing code...

    // Запускаем клиент сторожевого сервиса.
    if (!m_WatchServiceClient->start()) {
        getLog()->write(LogLevel::Error, "Cannot connect to watch service.");
        setResultCode(CUpdaterApp::ExitCode::NoWatchService);
#ifndef _DEBUG
        return;
#endif
    }

    auto *tooLongDownloadTimer = new QTimer(this);
    connect(tooLongDownloadTimer, SIGNAL(timeout()), this, SLOT(tooLondToDownload()));
    tooLongDownloadTimer->start(CUpdaterApp::MaxDownloadTime);

    m_Updater = new Updater(server, updateBaseUrl, version, app, configuration, pointId);

    m_Updater->setParent(this);
    m_Updater->setProxy(proxy);
    m_Updater->setAcceptedKeys(acceptKeys);
#ifdef Q_OS_WIN32
    m_Updater->useBITS(!woBITS, settings.value("bits/priority", CBITS::HIGH).toInt());
#endif
    connect(m_Updater, SIGNAL(progress(int)), &m_ReportBuilder, SLOT(setProgress(int)));

    // Создаем файл отчета.
    if (command == Command::Config) {
        // Устанавливаем рабочую папку.
        m_Updater->setWorkingDir("./user");
        m_Updater->setMD5(md5);

        connect(m_Updater,
                SIGNAL(done(CUpdaterErrors::Enum)),
                SLOT(onConfigReady(CUpdaterErrors::Enum)));
        connect(m_Updater, SIGNAL(deployment()), SLOT(onDeployment()));

        // Команда на закачку конфигов.
        m_Updater->downloadPackage();
    } else if (command == Command::UserPack) {
        m_Updater->setWorkingDir(subdir.isEmpty() ? "." : QString("./%1").arg(subdir));
        m_Updater->setMD5(md5);

        connect(m_Updater,
                SIGNAL(done(CUpdaterErrors::Enum)),
                woRestart ? SLOT(onPackReady(CUpdaterErrors::Enum))
                          : SLOT(onConfigReady(CUpdaterErrors::Enum)));
        connect(m_Updater, SIGNAL(deployment()), SLOT(onDeployment()));

        // Команда на закачку конфигов.
        m_Updater->downloadPackage();
    } else if (command == Command::Update) {
        if (workingDir.isEmpty()) {
            if (reRunFrom_TempDirectory()) {
                getLog()->write(
                    LogLevel::Normal,
                    QString("Run updater from temp path: '%1' is OK.").arg(getUpdaterTempDir()));
                setResultCode(CUpdaterApp::ExitCode::ContinueExecution);
                return;
            }
            getLog()->write(
                LogLevel::Error,
                QString("Failed run updater from temp path: '%1'.").arg(getUpdaterTempDir()));
            setResultCode(CUpdaterApp::ExitCode::ErrorRunFrom_TempDir);
            return;

        } // Устанавливаем рабочую папку.
        m_Updater->setWorkingDir(workingDir);

        // Добавляем директории в список исключений. Файлы в этих папках не участвуют в обновлении.
        m_Updater->addExceptionDirs(exceptionDirs());
        m_Updater->addExceptionDirs(configStringList(settings.value("directory/ignore")));

        m_Updater->addComponentForUpdate(components);
        m_Updater->setOptionalComponents(configStringList(settings.value("component/optional")));
        m_Updater->setConfigurationRequiredFiles(
            configStringList(settings.value("validator/required_files")));

        // Подписываемся на нужные сигналы от движка обновлений.
        connect(m_Updater, SIGNAL(updateSystem_IsWaiting()), this, SLOT(updateSystem_IsWaiting()));
        connect(m_Updater, SIGNAL(downloadAccomplished()), this, SLOT(onDownloadComplete()));
        connect(m_Updater,
                SIGNAL(done(CUpdaterErrors::Enum)),
                this,
                SLOT(onUpdateComplete(CUpdaterErrors::Enum)));

        QMetaObject::invokeMethod(m_Updater, "runUpdate", Qt::QueuedConnection);
    } else if (command == Command::Integrity) {
        // Устанавливаем рабочую папку.
        m_Updater->setWorkingDir("./");

        // Добавляем директории в список исключений. Файлы в этих папках не участвуют в обновлении.
        m_Updater->addExceptionDirs(exceptionDirs());
        m_Updater->addExceptionDirs(configStringList(settings.value("directory/ignore")));

        m_Updater->setOptionalComponents(configStringList(settings.value("component/optional")));
        m_Updater->setConfigurationRequiredFiles(
            configStringList(settings.value("validator/required_files")));

        int result = m_Updater->checkIntegrity();

        if (result < 0) {
            setResultCode(CUpdaterApp::ExitCode::UnknownCommand);
        } else if (result > 0) {
            setResultCode(CUpdaterApp::ExitCode::FailIntegrity);
            m_ReportBuilder.setStatusDescription(tr("#error_check_integrity").arg(result));
        } else {
            setResultCode(CUpdaterApp::ExitCode::NoError);
        }

        return;
    } else {
        getLog()->write(LogLevel::Error, QString("Unknown command: %1.").arg(command));
        setResultCode(CUpdaterApp::ExitCode::UnknownCommand);
        return;
    }

    int result = exec();
    if (result != m_ResultCode_) {
        setResultCode(static_cast<CUpdaterApp::ExitCode::Enum>(result));
    }
}

//---------------------------------------------------------------------------
CUpdaterApp::ExitCode::Enum UpdaterApp::bitsCheckStatus(bool aAlreadyRunning) {
    getLog()->write(LogLevel::Normal,
                    QString("Check bits status '%1'...").arg(qApp->applicationFilePath()));

    QStringList parameters;

    if (!m_Updater->bitsLoadState(&parameters)) {
        getLog()->write(LogLevel::Error, QString("Failed load bits.ini."));
        return CUpdaterApp::ExitCode::ParseError;
    }

    if (m_Updater->bitsIsComplete()) {
        getLog()->write(LogLevel::Normal, QString("BITS jobs are complete. Restart for deploy."));

        if (!aAlreadyRunning) {
            int stub = 0;
            m_Updater->bitsCompleteAllJobs(stub, stub, stub);

            // Restart for deploy
            if (!QProcess::startDetached(qApp->applicationFilePath(), parameters)) {
                getLog()->write(
                    LogLevel::Fatal,
                    QString("Couldn't start updater from '%1'.").arg(qApp->applicationFilePath()));
                return CUpdaterApp::ExitCode::UnknownError;
            }
        } else {
#ifdef Q_OS_WIN
            QString commanLine = QDir::toNativeSeparators(qApp->applicationFilePath());
            QString arguments =
                QString("--command bits --workdir %1").arg(m_Updater->getWorkingDir());
            QString workDir = QDir::toNativeSeparators(m_Updater->getWorkingDir());

            try {
                // Schedule restart myself
                PhishMe::AddScheduledTask(L"TC_Updater",
                                          commanLine.toStdWString(),
                                          arguments.toStdWString(),
                                          workDir.toStdWString(),
                                          CUpdaterApp::BITSCompleteTimeout);

                getLog()->write(
                    LogLevel::Normal,
                    QString("Do create updater restart scheduler task OK. Timeout: %1 min.")
                        .arg(CUpdaterApp::BITSCompleteTimeout / 60));
            } catch (std::exception &ex) {
                // Something seriously went wrong inside ScheduleTask
                getLog()->write(
                    LogLevel::Fatal,
                    QString("Didn't create updater restart scheduler task '%1'. Reason: %2.")
                        .arg(commanLine)
                        .arg(QString::fromLocal8Bit(ex.what())));

                return CUpdaterApp::ExitCode::UnknownError;
            }
#endif
        }

        return CUpdaterApp::ExitCode::NoError;
    }

    if (m_Updater->bitsIsError()) {
        getLog()->write(LogLevel::Error,
                        QString("BITS jobs are failed. Restart for download w/o bits."));

        // Restart for download w/o bits
        parameters << "--no-bits" << "true";
#ifdef Q_OS_WIN
        QString commanLine = QDir::toNativeSeparators(qApp->applicationFilePath());
        QString arguments = parameters.join(" ");
        QString workDir = QDir::toNativeSeparators(m_Updater->getWorkingDir());

        try {
            // Schedule restart myself
            PhishMe::AddScheduledTask(L"TC_Updater",
                                      commanLine.toStdWString(),
                                      arguments.toStdWString(),
                                      workDir.toStdWString(),
                                      CUpdaterApp::BITSErrorRestartTimeout);

            getLog()->write(LogLevel::Normal,
                            QString("Do create updater restart scheduler task OK. Timeout: %1 min.")
                                .arg(CUpdaterApp::BITSErrorRestartTimeout / 60));
        } catch (std::exception &ex) {
            // Something seriously went wrong inside ScheduleTask
            getLog()->write(
                LogLevel::Fatal,
                QString("Didn't create updater restart scheduler task '%1'. Reason: %2.")
                    .arg(commanLine)
                    .arg(QString::fromLocal8Bit(ex.what())));
        }
#endif
        return CUpdaterApp::ExitCode::NetworkError;
    }

    getLog()->write(LogLevel::Normal, QString("BITS continue execution."));
    return CUpdaterApp::ExitCode::ContinueExecution;
}

//---------------------------------------------------------------------------
void UpdaterApp::updateSystem_IsWaiting() {
    auto *updater = dynamic_cast<Updater *>(sender());

    int nextUpdateTimeout = 10 * 60;

    getLog()->write(LogLevel::Normal, QString("Run update in %1 sec.").arg(nextUpdateTimeout));

    QTimer::singleShot(nextUpdateTimeout * 1000, updater, SLOT(runUpdate()));
}

//---------------------------------------------------------------------------
void UpdaterApp::onDeployment() {
    m_State = CUpdaterApp::Deploy;
}

//---------------------------------------------------------------------------
void UpdaterApp::onDownloadComplete() {
    onDeployment();

    getLog()->write(LogLevel::Normal, "Download complete. Close modules...");

    // Останавливаем controller.exe (watch_service_controller)
    m_WatchServiceClient->closeModule(CWatchService::Modules::WatchServiceController);

    // Останавливаем ekiosk.exe
    m_WatchServiceClient->subscribeOnModuleClosed(this);
    m_WatchServiceClient->closeModule(CWatchService::Modules::PaymentProcessor);

    getLog()->write(LogLevel::Normal,
                    QString("Waiting PaymentProcessor exit... (%1 sec)")
                        .arg(CUpdaterApp::ErrorExitTimeout / 1000));

    // На всякий случай, если клиент не закроется в течении 15 минут
    startErrorTimer();
}

//---------------------------------------------------------------------------
void UpdaterApp::startErrorTimer() {
    m_ErrorStopTimer = new QTimer(this);
    m_ErrorStopTimer->setSingleShot(true);
    connect(m_ErrorStopTimer.data(), SIGNAL(timeout()), this, SLOT(onFailStopClient()));
    m_ErrorStopTimer->start(CUpdaterApp::ErrorExitTimeout);
}

//---------------------------------------------------------------------------
void UpdaterApp::stopErrorTimer() {
    if (m_ErrorStopTimer) {
        m_ErrorStopTimer->stop();
        m_ErrorStopTimer->deleteLater();
    }
}

//---------------------------------------------------------------------------
void UpdaterApp::onFailStopClient() {
    getLog()->write(LogLevel::Error, "Fail stop EKiosk.");

    m_WatchServiceClient->startModule(CWatchService::Modules::PaymentProcessor);

    QTimer::singleShot(10, this, SLOT(errorExit()));
}

//---------------------------------------------------------------------------
void UpdaterApp::tooLondToDownload() {
    getLog()->write(LogLevel::Error, "Too long to downloading update. Try to close updater.");

    if (m_State == CUpdaterApp::Download) {
        errorExit();
    } else {
        getLog()->write(LogLevel::Fatal, "Fail to close. Updater in deploy stage.");
    }
}

//---------------------------------------------------------------------------
void UpdaterApp::onModuleClosed(const QString &aModuleName) {
    if (aModuleName == CWatchService::Modules::PaymentProcessor) {
        // отсоединяемся от сигнала о закрытии модулей
        disconnect(this, SLOT(onModuleClosed(const QString &)));

        getLog()->write(LogLevel::Normal,
                        "PaymentProcessor closed. Wait to close watch service...");

        stopErrorTimer();

        // Показываем экран блокировки.
        m_SplashScreen.showMaximized();
        m_SplashScreen.showFullScreen();

        // Останавливаем Watch-Service.
        m_WatchServiceClient->stopService();
        // после остановки WatchService мы попадём в onDisconnected()
    }
}

//---------------------------------------------------------------------------
void UpdaterApp::onUpdateComplete(CUpdaterErrors::Enum aError) {
    if (m_State == CUpdaterApp::Deploy) {
        // Запускаем controller
        if (!QProcess::startDetached(getWorkingDirectory() + QDir::separator() +
                                     CWatchService::Modules::WatchServiceController +
                                     getExecutableExtension())) {
            getLog()->write(LogLevel::Error, "Failed to launch controller app.");
        }

        // Заново запускаем watchservice и убеждаемся, что он успешно запустился.
        auto *startService = new QProcess();
        startService->start(CWatchService::Modules::WatchService);

        if (!startService->waitForStarted()) {
            // Фатальная ошибка.
            getLog()->write(LogLevel::Fatal,
                            "Failed to launch watch service. Updater showing splash screen.");

            // Не выходим из приложения.
            return;
        }
    }

    // Завершаем работу приложения через 10 секунд, что бы watchdog успел отобразить окно блокировки
    delayedExit(10, aError);
}

//---------------------------------------------------------------------------
void UpdaterApp::errorExit() {
    if (m_ResultCode_ == 0) {
        m_ResultCode_ = CUpdaterApp::ExitCode::UnknownError;
    }

    getQtApplication().exit(m_ResultCode_);
}

//---------------------------------------------------------------------------
int UpdaterApp::getResultCode() const {
    return m_ResultCode_;
}

//---------------------------------------------------------------------------
void UpdaterApp::onConfigReady(CUpdaterErrors::Enum aError) {
    // Удаляем из конфигурации шаблоны чеков
    cleanDir(QDir::currentPath() + QDir::separator() + "user" + QDir::separator() + "templates");
    QDir(QDir::currentPath() + QDir::separator() + "user").rmdir("templates");

    // Перезапускаем ПО.
    m_WatchServiceClient->restartService(QStringList());

    // Завершаем работу приложения через 5 секунд, что бы watchdog успел получить сигнал
    // перезагрузки ТК
    delayedExit(5, aError);
}

//---------------------------------------------------------------------------
void UpdaterApp::onPackReady(CUpdaterErrors::Enum aError) {
    delayedExit(1, aError);
}

//---------------------------------------------------------------------------
void UpdaterApp::delayedExit(int aTimeout, CUpdaterErrors::Enum aError) {
    m_State = CUpdaterApp::Finish;

    setResultCode(aError);

    getLog()->write((aError != 0U) ? LogLevel::Error : LogLevel::Normal,
                    QString("Closing after %1 seconds.").arg(aTimeout));

    if (m_ResultCode_ != 0) {
        QTimer::singleShot(aTimeout * 1000, this, SLOT(errorExit()));
    } else {
        QTimer::singleShot(aTimeout * 1000, qApp, SLOT(quit()));
    }
}

//---------------------------------------------------------------------------
QString UpdaterApp::getUpdaterTempDir() const {
    const QString tempDirName = "ekiosk_updater_temp";

    QDir tempDir = QDir::tempPath();

    if (!tempDir.exists(tempDirName)) {
        if (!tempDir.mkdir(tempDirName)) {
            getLog()->write(
                LogLevel::Fatal,
                QString("Error mkdir '%1' in '%2'.").arg(tempDirName).arg(QDir::tempPath()));
            return {};
        }
    }

    tempDir.cd(tempDirName);
    return tempDir.canonicalPath();
}

//---------------------------------------------------------------------------
QString UpdaterApp::getExecutableExtension() const {
#ifdef Q_OS_WIN
    return ".exe";
#else
    return {};
#endif
}

//---------------------------------------------------------------------------
QString UpdaterApp::getLibraryExtension() const {
#ifdef Q_OS_WIN
    return ".dll";
#elif defined(Q_OS_MAC)
    return ".dylib";
#else
    return ".so";
#endif
}

//---------------------------------------------------------------------------
bool UpdaterApp::reRunFrom_TempDirectory() {
    getLog()->write(LogLevel::Normal,
                    QString("Trying run updater from temp path: '%1'.").arg(getUpdaterTempDir()));
    if (CopyToTempPath()) {
        QString program =
            QDir(getUpdaterTempDir())
                .absoluteFilePath(QFileInfo(QCoreApplication::arguments()[0]).fileName());
        QStringList arguments = QStringList()
                                << getArguments() << "--workdir" << getWorkingDirectory();

        {
            QString programSettingsFile =
                QDir(getUpdaterTempDir())
                    .absoluteFilePath(QFileInfo(QCoreApplication::arguments()[0]).baseName()) +
                ".ini";
            QSettings tempSettings(ISysUtils::rm_BOM(programSettingsFile), QSettings::IniFormat);

            tempSettings.setValue("common/working_directory", getWorkingDirectory());
        }
        // удаляем первый аргумент с именем файла
        arguments.takeFirst();

        return QProcess::startDetached(program, arguments, getUpdaterTempDir());
    }

    return false;
}

//---------------------------------------------------------------------------
bool UpdaterApp::CopyToTempPath() {
    QString tempDirPath = getUpdaterTempDir();
    if (tempDirPath.isEmpty()) {
        return false;
    }

    getLog()->write(LogLevel::Normal, QString("Clean directory '%1'.").arg(tempDirPath));
    // чистим папку
    if (!cleanDir(tempDirPath)) {
        return false;
    }

    QFileInfo updaterFileInfo = QFileInfo(getArguments()[0]);

    QStringList needFiles = QStringList()
                            << updaterFileInfo.fileName()
                            << updaterFileInfo.absoluteDir().entryList(
                                   QStringList() << updaterFileInfo.baseName() + "*.qm")
                            << updaterFileInfo.absoluteDir().entryList(
                                   QStringList() << updaterFileInfo.baseName() + ".*") // for pdb
                            << "7za.exe"
                            << "Qt5Core.dll"
                            << "Qt5Gui.dll"
                            << "Qt5Widgets.dll"
                            << "Qt5Xml.dll"
                            << "Qt5Network.dll"
                            << "ssleay32.dll"
                            << "libeay32.dll"
                            << "icudt51.dll"
                            << "icuin51.dll"
                            << "icuuc51.dll"
                            << "libEGL.dll"
                            << "libGLESv2.dll";

    foreach (auto file,
             QDir(QCoreApplication::applicationDirPath()).entryInfoList(needFiles, QDir::Files)) {
        QString fileName = QFileInfo(file).fileName();
        QString dstFileName = tempDirPath + QDir::separator() + fileName;
        getLog()->write(LogLevel::Normal, QString("Copy: '%1'.").arg(fileName));

        if (!QFile::copy(QCoreApplication::applicationDirPath() + QDir::separator() + fileName,
                         dstFileName)) {
            getLog()->write(
                LogLevel::Fatal,
                QString("Error copy from '%1' to '%2'.").arg(file.fileName()).arg(dstFileName));
            return false;
        }
    }

    QDir(tempDirPath).mkdir("platforms");
    QDir(tempDirPath).mkdir("imageformats");

    return copyFiles(updaterFileInfo.path() + QDir::separator() + "platforms",
                     "*" + getLibraryExtension(),
                     tempDirPath + QDir::separator() + "platforms") &&
           copyFiles(updaterFileInfo.path() + QDir::separator() + "imageformats",
                     "*" + getLibraryExtension(),
                     tempDirPath + QDir::separator() + "imageformats");
}

//---------------------------------------------------------------------------
bool UpdaterApp::copyFiles(const QString &from, const QString &mask, const QString &to) {
    QDir fromDir = QDir(from);

    Q_FOREACH (QString file, fromDir.entryList(QStringList() << mask)) {
        QString dstFileName = to + QDir::separator() + QFileInfo(file).fileName();
        getLog()->write(LogLevel::Normal, QString("Copy: '%1'.").arg(file));
        if (!QFile::copy(fromDir.filePath(file), dstFileName)) {
            getLog()->write(LogLevel::Fatal,
                            QString("Error copy from '%1' to '%2'.").arg(file).arg(dstFileName));
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------
void UpdaterApp::qtMessageHandler(QtMsgType aType,
                                  const QMessageLogContext &aContext,
                                  const QString &aMessage) {
    static ILog *log = ILog::getInstance(CUpdater::Name);

    log->write(LogLevel::Normal, QString("QtMessages: %1").arg(aMessage));
}

//---------------------------------------------------------------------------
bool UpdaterApp::cleanDir(const QString &dirName) {
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH (auto info,
                   dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden |
                                         QDir::AllDirs | QDir::Files,
                                     QDir::DirsFirst)) {
            if (info.isDir()) {
                result = cleanDir(info.absoluteFilePath()) && dir.rmdir(info.absoluteFilePath());
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                getLog()->write(LogLevel::Fatal,
                                QString("Error remove '%1'.").arg(info.absoluteFilePath()));
                return result;
            }
        }
    }

    return result;
}

//---------------------------------------------------------------------------
void UpdaterApp::setResultCode(CUpdaterErrors::Enum aError,
                               const QString aMessage /*= QString()*/) {
    m_ResultDescription = aMessage;

    switch (aError) {
    case CUpdaterErrors::OK:
        setResultCode(CUpdaterApp::ExitCode::NoError, aMessage);
        break;

    case CUpdaterErrors::UnknownError:
        setResultCode(CUpdaterApp::ExitCode::UnknownError, aMessage);
        break;

    case CUpdaterErrors::NetworkError:
        setResultCode(CUpdaterApp::ExitCode::NetworkError, aMessage);
        break;

    case CUpdaterErrors::ParseError:
        setResultCode(CUpdaterApp::ExitCode::ParseError, aMessage);
        break;

    case CUpdaterErrors::DeployError:
        setResultCode(CUpdaterApp::ExitCode::DeployError, aMessage);
        break;

    case CUpdaterErrors::UpdateBlocked:
        setResultCode(CUpdaterApp::ExitCode::Blocked, aMessage);
        break;

    case CUpdaterErrors::BitsInProgress:
        setResultCode(CUpdaterApp::ExitCode::NoError, tr("Update in progress"));
        break;

    default:

        break;
    }

    updateErrorDescription();
}

//---------------------------------------------------------------------------
void UpdaterApp::setResultCode(CUpdaterApp::ExitCode::Enum aExitCode,
                               const QString aMessage /*= QString()*/) {
    m_ResultCode_ = aExitCode;
    m_ResultDescription = aMessage;

    updateErrorDescription();
}

//---------------------------------------------------------------------------
void UpdaterApp::updateErrorDescription() {
    static QMap<int, QString> descriptions;

    if (descriptions.isEmpty()) {
        descriptions.insert(CUpdaterApp::ExitCode::ErrorRunFrom_TempDir,
                            tr("#error_run_from_temp_dir"));
        descriptions.insert(CUpdaterApp::ExitCode::NoWatchService,
                            tr("#error_connection_to_watchdog"));
        descriptions.insert(CUpdaterApp::ExitCode::UnknownCommand, tr("#error_unknown_command"));
        descriptions.insert(CUpdaterApp::ExitCode::SecondInstance, tr("#error_second_instance"));
        descriptions.insert(CUpdaterApp::ExitCode::UnknownError, tr("#error_unknown"));
        descriptions.insert(CUpdaterApp::ExitCode::NetworkError, tr("#error_network"));
        descriptions.insert(CUpdaterApp::ExitCode::ParseError, tr("#error_parse_response"));
        descriptions.insert(CUpdaterApp::ExitCode::DeployError, tr("#error_deploy"));
        descriptions.insert(CUpdaterApp::ExitCode::Aborted, tr("#error_aborted"));
        descriptions.insert(CUpdaterApp::ExitCode::Blocked, tr("#error_update_blocked"));
        descriptions.insert(CUpdaterApp::ExitCode::FailIntegrity, tr("#error_check_integrity"));
        descriptions.insert(CUpdaterApp::ExitCode::WorkInProgress, tr("#work_in_progress"));
    }

    if (m_ResultDescription.isEmpty() && descriptions.contains(m_ResultCode_)) {
        m_ResultDescription = descriptions.value(m_ResultCode_);
    }
}

//---------------------------------------------------------------------------
