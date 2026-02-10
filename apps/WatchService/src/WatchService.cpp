/* @file Реализация сторожевого сервиса как обычного приложения. */

#include "WatchService.h"

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtGui/QSessionManager>

#include <Common/BasicApplication.h>
#include <Common/SleepHelper.h>

#include <SysUtils/ISysUtils.h>
#include <WatchServiceClient/Constants.h>
#include <boost/optional.hpp>

#include "CpuSpeed.h"

namespace CStartMode {
const char Normal[] = "normal";
const char Exclusive[] = "exclusive";
} // namespace CStartMode

#if 0 // #40592 Пока выключаем данную опцию
#include "processenumerator.h"
#endif

// (No STL includes needed)

//----------------------------------------------------------------------------
namespace CWatchService {
const char ExtensionsSection[] = "extensions";
const char UseCustom_Background[] = "use_custom_background";
const char DisableStatusBar[] = "disable_status_bar";

const int ReInitializeTimeout = 7 * 1000;
const int ReInitializeFailMaxCount = 85;
const int ContinueExecutionExitCode = 54321;
} // namespace CWatchService

//----------------------------------------------------------------------------
SModule::SModule()
    : restartCount(0), process(nullptr), autoStart(false), startPriority(0), closePriority(0),
      afterStartDelay(0), needToStart(false), previousNeedToStart(false), gui(false),
      firstPingTimeout(0), maxStartCount(0), noResponseCount(0),
      killTimeout(WatchService::defaultKillTimeout()), killOnStartCount(0) {}

//----------------------------------------------------------------------------
void SModule::touch() {
    lastUpdate = QDateTime::currentDateTime();
    killOnStartCount = 0;
}

//----------------------------------------------------------------------------
bool SModule::kill() {
    bool result = true;

    if (process) {
        process->kill();

        // Ждём завершения...
        result = process->waitForFinished();
    }

    if (lastUpdate == initDate) {
        killOnStartCount++;
    } else {
        killOnStartCount = 0;
    }

    return result;
}

//----------------------------------------------------------------------------
int SModule::getFirstPingTimeout() const {
    return qMin(firstPingTimeout + killOnStartCount * CWatchService::FirstPingTimeoutIncrement,
                CWatchService::FirstPingTimeoutMax);
}

//----------------------------------------------------------------------------
WatchService::WatchService()
    : ILogable(BasicApplication::getInstance()->getLog()->getName()),
      m_SplashScreen(getLog()->getName()), m_CloseAction(ECloseAction::None),
      m_ScreenProtectionEnabled(true), m_FirstRun(true), m_InitializeFailedCounter(0) {
    // Use new Qt5/6 compatible signal/slot syntax
    connect(this, &WatchService::screenUnprotected, &m_SplashScreen, &SplashScreen::showFullScreen);
    connect(this, &WatchService::screenProtected, &m_SplashScreen, &SplashScreen::hide);
    connect(this, &WatchService::stateChanged, &m_SplashScreen, &SplashScreen::setState);
    connect(this, &WatchService::stateReset, &m_SplashScreen, &SplashScreen::removeStates);
    connect(&m_SplashScreen, &SplashScreen::clicked, this, &WatchService::onScreenActivity);
    connect(&m_Timer, &QTimer::timeout, this, &WatchService::onCheckModules);
    connect(&m_ScreenActivityTimer, &QTimer::timeout, this, &WatchService::onScreenActivityTimeout);

    initialize();
}

//----------------------------------------------------------------------------
WatchService::~WatchService() {}

//----------------------------------------------------------------------------
void WatchService::initialize() {
    emit screenUnprotected();

    // Настраиваем защитный экран
    QString custom_Background = BasicApplication::getInstance()
                                    ->getSettings()
                                    .value(QString(CWatchService::ExtensionsSection) + "/" +
                                           CWatchService::UseCustom_Background)
                                    .toString();
    if (!custom_Background.isEmpty()) {
        if (!QDir::isAbsolutePath(custom_Background)) {
            custom_Background = QDir::cleanPath(
                QDir::toNativeSeparators(BasicApplication::getInstance()->getWorkingDirectory() +
                                         QDir::separator() + custom_Background));
        }

        m_SplashScreen.setCustom_Background(custom_Background);

        toLog(LogLevel::Normal,
              QString("Using custom background image: %1.").arg(custom_Background));
    }

    // FIXME: временно для #21549
    /*bool statusBarDisabled =
    BasicApplication::getInstance()->getSettings().value(QString(CWatchService::ExtensionsSection) +
            "/" + CWatchService::DisableStatusBar).toBool();

    if (statusBarDisabled)
    {
            m_SplashScreen.setStatusBarEnabled(false);

            toLog(LogLevel::Normal, "Status bar disabled.");
    }*/

    toLog(LogLevel::Normal, QString("CPU speed: %1 MHz.").arg(cpuSpeed()));

    // Загружаем конфигурацию
    loadConfiguration();

    if (m_Modules.isEmpty()) {
        toLog(LogLevel::Error, "Module list is empty, it is nothing to watch.");

        return;
    }

    // В случае перезагрузки с параметрами, пропихиваем их в модули
    if (!m_RestartParameters.isEmpty()) {
        for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
            it->params = m_RestartParameters;
        }
    }

    // Поднимаем сервер
    m_Server = QSharedPointer<IMessageQueueServer>(
        createMessageQueueServer(CWatchService::MessageQueue, getLog()));

    m_Server->subscribeOnMessageReceived(this);

    if (!m_Server->init()) {
        toLog(LogLevel::Fatal, "Cannot init service server.");

        if (m_InitializeFailedCounter >= CWatchService::ReInitializeFailMaxCount) {
            toLog(LogLevel::Fatal, "Reinitialize fail counter reached. Reboot system.");

            doReboot();
        } else {
            // Use new Qt5/6 compatible signal/slot syntax with lambda
            QTimer::singleShot(
                CWatchService::ReInitializeTimeout, this, [this]() { reinitialize(); });

            m_InitializeFailedCounter++;
        }
    } else {
        // Запускаем таймер
        m_Timer.setInterval(CWatchService::CheckInterval);
        m_Timer.start();

        // Запускаем таймер проверки использования памяти
        m_CheckMemoryTimer = QSharedPointer<QTimer>(new QTimer(this));
        connect(
            m_CheckMemoryTimer.data(), &QTimer::timeout, this, &WatchService::checkProcessMemory);
        m_CheckMemoryTimer->start(CWatchService::CheckMemoryUsageTimeout * 60 * 1000);

        m_FirstRun = false;

        m_TimeChangeListener = QSharedPointer<TimeChangeListener>(new TimeChangeListener(this));
        connect(m_TimeChangeListener.data(),
                &TimeChangeListener::timeChanged,
                this,
                &WatchService::onTimeChanged);

        // Сбрасываем
        m_RestartParameters.clear();

#if 0 // #40592 Пока выключаем данную опцию
		if (!m_ForbiddenModules.isEmpty())
		{
			m_CheckForbiddenTimer = QSharedPointer<QTimer>(new QTimer(this));
			connect(m_CheckForbiddenTimer.data(), &QTimer::timeout, this, &WatchService::checkForbiddenModules);
			m_CheckForbiddenTimer->start(m_CheckForbiddenTimeout);

			QMetaObject::invokeMethod(this, "checkForbiddenModules", Qt::QueuedConnection);
		}
#endif
    }

    connect(qApp,
            &QApplication::commitDataRequest,
            this,
            &WatchService::closeBySystem_Request,
            Qt::DirectConnection);
}

//----------------------------------------------------------------------------
void WatchService::closeBySystem_Request(QSessionManager &aSessionManager) {
    QByteArray msg;

    if (m_CloseAction == ECloseAction::None) {
        // блокируем остановку системы.
        aSessionManager.cancel();

        toLog(LogLevel::Normal, "System shutdown request.");

        // останавливаем систему самостоятельно
        msg = (CWatchService::Fields::Sender + "=SYSTEM;" + CWatchService::Fields::Type + "=" +
               CWatchService::Commands::Shutdown)
                  .toLatin1();
    }

    if (!msg.isEmpty()) {
        messageReceived(msg);
    }
}

//----------------------------------------------------------------------------
void WatchService::reinitialize() {
    toLog(LogLevel::Normal, "Reinitialize...");

    m_Timer.stop();
    m_CheckMemoryTimer.clear();
    m_TimeChangeListener.clear();

    if (m_Server) {
        m_Server->stop();
        m_Server.clear();
    }

    initialize();
}

//----------------------------------------------------------------------------
void WatchService::loadConfiguration() {
    toLog(LogLevel::Normal,
          QString("Application dir: %1").arg(QCoreApplication::applicationDirPath()));

    toLog(LogLevel::Normal, "Loading configuration.");

    m_Modules.clear();

    QSettings &settings = BasicApplication::getInstance()->getSettings();

    QMap<QString, QString> moduleOptions;

    if (m_FirstRun) {
        foreach (const QString &arg, QCoreApplication::arguments()) {
            QRegularExpression re("-(\\w+)_options=(.+)");
            QRegularExpressionMatch match = re.match(arg);

            if (match.hasMatch()) {
                moduleOptions[match.captured(1)] = match.captured(2);
            }
        }
    }

    // Загрузим информацию о модулях
    foreach (QString group, settings.childGroups()) {
        if (group.toLower().indexOf("module") != -1) {
            SModule info;
            info.name = settings.value(group + "/name").toString();
            info.file = settings.value(group + "/file")
                            .toString()
                            .replace("{WS_DIR}",
                                     QCoreApplication::applicationDirPath(),
                                     Qt::CaseInsensitive);
            info.workingDirectory = settings.value(group + "/workingdirectory")
                                        .toString()
                                        .replace("{WS_DIR}",
                                                 QCoreApplication::applicationDirPath(),
                                                 Qt::CaseInsensitive);
            info.restartCount = 0;
            info.startMode = settings.value(group + "/startmode").toString().toLower();
            info.autoStart = settings.value(group + "/autostart").toBool();
            info.process = 0;
            info.initDate = QDateTime::currentDateTime();
            info.lastUpdate = info.initDate;
            info.startPriority = settings.value(group + "/priority").toUInt();
            info.closePriority = settings.value(group + "/close_priority").toUInt();
            info.afterStartDelay = settings.value(group + "/afterstartdelay", "0").toUInt();
            info.needToStart = info.autoStart;
            info.previousNeedToStart = false;
            info.maxStartCount = settings.value(group + "/maxstartcount", "0").toUInt();
            info.params.clear();
            info.gui = settings.value(group + "/gui", "false").toBool();
            info.firstPingTimeout =
                settings.value(group + "/firstpingtimeout", CWatchService::FirstPingTimeoutDefault)
                    .toUInt();
            info.arguments = moduleOptions[info.name];
            info.killTimeout =
                qMax(settings.value(group + "/kill_timeout", defaultKillTimeout()).toInt(),
                     defaultKillTimeout());

            m_Modules[info.name] = info;

            toLog(LogLevel::Normal,
                  QString("Module with name %1 (%2) has been added to the watch service list.")
                      .arg(info.name)
                      .arg(info.file));
        }
    }

#if 0 // #40592 Пока выключаем данную опцию
	QSettings userSettings(ISysUtils::rm_BOM(BasicApplication::getInstance()->getWorkingDirectory() + "/user/user.ini"), QSettings::IniFormat);
	userSettings.setIniCodec("UTF-8");

	if (userSettings.value("watchdog/taboo_enabled").toString() == "true")
	{
		settings.beginGroup("taboo");
		m_ForbiddenModules = settings.value("applications", "").toStringList();
		m_CheckForbiddenTimeout = settings.value("check_timeout", 60000).toInt();
		settings.endGroup();
	}
#endif
}

//----------------------------------------------------------------------------
void WatchService::onMessageReceived(QByteArray aMessage) {
    messageReceived(aMessage);
}

//----------------------------------------------------------------------------
void WatchService::checkAutoStartModules() {
    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        if (it->autoStart && !it->needToStart) {
            it->needToStart = true;
            it->restartCount = 0;

            onCheckModules();
        }
    }
}

//----------------------------------------------------------------------------
void WatchService::messageReceived(const QByteArray &aMessage) {
    QStringList param_List = QString::fromUtf8(aMessage.data(), aMessage.size()).split(";");
    if (!param_List.isEmpty()) {
        QString sender;
        QString type;
        QString params;
        QString module;

        foreach (QString param, param_List) {
            if (param.indexOf(CWatchService::Fields::Sender) != -1) {
                sender = param.right(param.length() - param.indexOf("=") - 1);
            } else if (param.indexOf(CWatchService::Fields::Type) != -1) {
                type = param.right(param.length() - param.indexOf("=") - 1);
            } else if (param.indexOf(CWatchService::Fields::Params) != -1) {
                params = param.right(param.length() - param.indexOf("=") - 1);
            } else if (param.indexOf(CWatchService::Fields::Module) != -1) {
                module = param.right(param.length() - param.indexOf("=") - 1);
            }
        }

        // Запуск модуля
        if (type == CWatchService::Commands::StartModule) {
            TModules::iterator it = m_Modules.find(module);
            if (it != m_Modules.end()) {
                bool exclusive = false;

                if (it.value().startMode == "exclusive") {
                    exclusive = true;

                    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
                        it->previousNeedToStart = it->needToStart;
                        it->needToStart = false;
                    }
                }

                it.value().needToStart = true;
                it.value().restartCount = 0;
                it.value().params = params;

                it.value().commandStack.push_back(CWatchService::Commands::StartModule);

                toLog(LogLevel::Normal,
                      QString("Module %1 sent 'start_module' command (module: %2, params: %3, "
                              "exclusive: %4).")
                          .arg(sender)
                          .arg(module)
                          .arg(params)
                          .arg(QVariant(exclusive).toString()));

                onCheckModules();
            }
        }
        // Выключение модуля
        else if (type == CWatchService::Commands::CloseModule) {
            TModules::iterator it = m_Modules.find(module);
            if (it != m_Modules.end()) {
                it.value().needToStart = false;

                it.value().commandStack.push_back(CWatchService::Commands::CloseModule);

                toLog(LogLevel::Normal,
                      QString("Module %1 sent 'close_module' command (target module: %2).")
                          .arg(sender)
                          .arg(module));

                if (it.value().autoStart) {
                    toLog(
                        LogLevel::Normal,
                        QString("Module %1 is auto started. Re run it after 10 min.").arg(module));

                    // принудительная остановка модуля с признаком авто запуска возможна только на
                    // 10 минут
                    QTimer::singleShot(10 * 60 * 1000, this, [this]() { checkAutoStartModules(); });
                }

                onCheckModules();
            } else {
                sendCommandToUknownModule(CWatchService::Commands::Close, module);
            }
        }
        // Перезапуск всех модулей
        else if (type == CWatchService::Commands::Restart) {
            toLog(LogLevel::Normal,
                  QString("Module %1 has sent restart command. Parameters='%2' Starting restart "
                          "sequence...")
                      .arg(sender)
                      .arg(params));

            m_RestartParameters = params;

            for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
                it->needToStart = false;
            }

            m_CloseAction = ECloseAction::Restart;

            closeModules();
        }
        // Рассылаем всем модулям команду закрыть лог файлы
        else if (type == CWatchService::Commands::CloseLogs) {
            toLog(LogLevel::Normal,
                  QString("Module %1 has sent close all logs command. Notifies modules...")
                      .arg(sender));

            m_Server->sendMessage(QString("sender=%1;type=%2")
                                      .arg(CWatchService::Name)
                                      .arg(CWatchService::Commands::CloseLogs)
                                      .toUtf8());

            ILog::logRotateAll();
        }
        // Завершаем работу всех модулей
        else if (type == CWatchService::Commands::Close) {
            toLog(LogLevel::Normal,
                  QString("Module %1 has sent close command. Closing modules...").arg(sender));

            for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
                it->needToStart = false;
            }

            m_CloseAction = ECloseAction::None;

            doCloseModules(sender);
        }
        // Завершаем работу сервиса
        else if (type == CWatchService::Commands::Exit) {
            toLog(LogLevel::Normal,
                  QString("Module %1 has sent exit command. Closing service...").arg(sender));

            for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
                it->needToStart = false;
            }

            m_CloseAction = ECloseAction::Exit;

#if 0 // #40592 Пока выключаем данную опцию
			startTerminatedModules();
#endif
            closeModules();
        }
        // Перезагрузка системы
        else if (type == CWatchService::Commands::Reboot) {
            toLog(LogLevel::Normal,
                  QString("Module %1 has sent reboot command. Starting reboot sequence...")
                      .arg(sender));

            doReboot();
        }
        // Выключение системы
        else if (type == CWatchService::Commands::Shutdown) {
            toLog(LogLevel::Normal,
                  QString("Module %1 has sent shutdown command. Closing modules...").arg(sender));

            for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
                it->needToStart = false;
            }

            m_CloseAction = ECloseAction::Shutdown;

            closeModules();
        } else if (type == CWatchService::Commands::ShowSplashScreen) {
            toLog(LogLevel::Normal, QString("Screen protection enabled by module %1.").arg(sender));

            enableScreenProtection(true);
        } else if (type == CWatchService::Commands::HideSplashScreen) {
            toLog(LogLevel::Normal,
                  QString("Screen protection disabled by module %1.").arg(sender));

            enableScreenProtection(false);
        } else if (type == CWatchService::Commands::SetState) {
            toLog(LogLevel::Normal,
                  QString("Module %1 has sent state %2.").arg(sender).arg(params));

            emit stateChanged(sender, params);
        } else if (type == CWatchService::Commands::ResetState) {
            toLog(LogLevel::Normal, QString("Module %1 has reset state.").arg(sender));

            emit stateReset(sender);
        }

        TModules::iterator it = m_Modules.find(sender);
        if (it != m_Modules.end()) {
            it.value().lastAnswer = aMessage;
            it.value().touch();
        }
    }
}

//----------------------------------------------------------------------------
void WatchService::sendCommandToUknownModule(const QString &aCommand, const QString &aModule) {
    toLog(LogLevel::Normal,
          QString("Sending %1 command to unknown module %2.").arg(aCommand).arg(aModule));

    m_Server->sendMessage(
        QString("sender=watch_service;target=%1;type=%2").arg(aModule).arg(aCommand).toUtf8());
}

//----------------------------------------------------------------------------
void WatchService::onTimeChanged(qint64 aTimeOffset) {
    toLog(LogLevel::Normal,
          QString("System time changed. Offset %2 sec.").arg(aTimeOffset / 100., 0, 'f', 3));

    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        it->initDate = it->initDate.addMSecs(aTimeOffset);
        it->lastUpdate = it->lastUpdate.addMSecs(aTimeOffset);
    }
}

//----------------------------------------------------------------------------
void WatchService::onCheckModules() {
    checkScreenProtection();

    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        if (!it->commandStack.isEmpty()) {
            if (it->commandStack.first() == CWatchService::Commands::StartModule) {
                it->needToStart = true;
                it->commandStack.removeFirst();
            } else if (it->commandStack.first() == CWatchService::Commands::CloseModule) {
                it->needToStart = false;
                it->commandStack.removeFirst();
            }
        }

        if (it->needToStart) {
            if (!(it.value().process)) {
                it.value().process = new QProcess(this);
                connect(it.value().process,
                        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        this,
                        &WatchService::onModuleFinished);
            }

            if (it.value().process->state() == QProcess::NotRunning) {
                if (!canRun(*it)) {
                    // Если есть модули с большим приоритетом, то отложим пока свой запуск
                    continue;
                }

                if (it->maxStartCount > 0) {
                    if (it->restartCount >= it->maxStartCount) {
                        toLog(LogLevel::Warning,
                              QString("Module %1 has exceeded start count limit.").arg(it.key()));

                        it->needToStart = false;

                        // Если завершил работу модуль с эксклюзивными правами, то восстановим
                        // работу остальных модулей...
                        if (it->startMode.toLower() == "exclusive") {
                            for (TModules::iterator restoreIt = m_Modules.begin();
                                 restoreIt != m_Modules.end();
                                 ++restoreIt) {
                                if ((it.key() != restoreIt.key()) &&
                                    (restoreIt->startMode.toLower() != "exclusive")) {
                                    restoreIt->needToStart = restoreIt->previousNeedToStart;
                                }
                            }
                        }

                        continue;
                    }
                }

                startModule(*it);
            } else {
                if (!it.value().needToStart) {
                    // Модуль необходимо закрыть
                    closeModule(it.value(), true);

                    continue;
                }

                if (it.value().initDate == it.value().lastUpdate) {
                    // Запустили приложение и ждём пока оно проинициализируется...
                    if (it.value().initDate.addSecs(it.value().getFirstPingTimeout()) >
                        QDateTime::currentDateTime()) {
                        // Пока что не превысили лимит до первого пинга...
                        continue;
                    } else {
                        toLog(LogLevel::Debug,
                              QString("Module %1 has exceeded first time ping timeout.")
                                  .arg(it.key()));
                    }
                }

                // Проверка на отклик модуля
                if (it.value().lastUpdate.addSecs(it.value().killTimeout) <
                    QDateTime::currentDateTime()) {
                    // Вырубаем модуль только после второго несоответствия
                    if (it->noResponseCount > 0) {
                        toLog(LogLevel::Warning,
                              QString("Module %1 has exceeded ping timeout. Killing module...")
                                  .arg(it.key()));

                        // Отклика не было больше допущенного времени... Убиваем процесс и поднимаем
                        // его снова.
                        it.value().kill();

                        it->noResponseCount = 0;

                        if (it.value().killOnStartCount) {
                            toLog(LogLevel::Warning,
                                  QString("Module %1 killed during startup. New first ping timeout "
                                          "%2 sec.")
                                      .arg(it.key())
                                      .arg(it.value().getFirstPingTimeout()));
                        }
                    } else {
                        ++it->noResponseCount;
                    }
                } else {
                    it->noResponseCount = 0;
                }
            }
        } else {
            closeModule(*it, true);
        }
    }
}

//----------------------------------------------------------------------------
void WatchService::onModuleFinished(int aExitCode, QProcess::ExitStatus aExitStatus) {
    bool noModulesRunning = true;

    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        if ((it->process) && (it->process->state() == QProcess::Running)) {
            noModulesRunning = false;

            break;
        }
    }

    if (noModulesRunning) {
        this->enableScreenProtection(true);
    }

    checkScreenProtection();

    QProcess *process = dynamic_cast<QProcess *>(sender());
    if (process) {
        for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
            if (it.value().process == process) {
                toLog(LogLevel::Warning,
                      QString("Module %1 has %2 with exit code %3.")
                          .arg(it.key())
                          .arg(aExitStatus == QProcess::CrashExit ? "crashed" : "exited")
                          .arg(aExitCode));

                if (aExitCode != CWatchService::ContinueExecutionExitCode) {
                    // Уведомляем о закрытии модуля
                    m_Server->sendMessage(QString("type=%1;sender=%2")
                                              .arg(CWatchService::Notification::ModuleClosed)
                                              .arg(it.key())
                                              .toUtf8());
                }

                emit stateReset(it.key());

                break;
            }
        }
    }
}

//----------------------------------------------------------------------------
bool WatchService::canRun(const SModule &aModule) {
    if (aModule.startPriority == -1) {
        // Модуль ни от чего не зависит
        return true;
    }

    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        if (aModule.startMode == "exclusive") {
            if ((it->process) && (it->process->state() == QProcess::Running)) {
                return false;
            }
        } else {
            if ((it->startPriority < aModule.startPriority) && (it->needToStart)) {
                if ((!it->process) || (it->process->state() == QProcess::NotRunning)) {
                    return false;
                }

                if (it->initDate == it->lastUpdate) {
                    // Мы запустили приложение, но ответа пока не было и значит
                    // оно ещё не успело проинициализироваться...
                    return false;
                }
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------
bool WatchService::canTerminate(const SModule &aModule) {
    if (aModule.closePriority == -1) {
        // Модуль ни от чего не зависит
        return true;
    }

    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        if (it->closePriority < aModule.closePriority) {
            if ((it->process) && (it->process->state() == QProcess::Running)) {
                return false;
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------
void WatchService::closeModules() {
    doCloseModules(CWatchService::Modules::WatchService);
}

//----------------------------------------------------------------------------
void WatchService::doCloseModules(QString aSender) {
    toLog(LogLevel::Normal, "Closing modules.");

    if (m_Timer.isActive()) {
        m_Timer.stop();
    }

    bool canClose = true;

    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        if (!closeModule(*it)) {
            toLog(LogLevel::Warning, QString("Cannot close module: %1.").arg(it.key()));

            if (canClose) {
                QTimer::singleShot(1000, this, [this]() { closeModules(); });

                canClose = false;
            }
        }
    }

    if (aSender == CWatchService::Modules::Updater) {
        // Выгружаем 'свободные' модули только по команде от апдейтера.
        m_Server->sendMessage("sender=watch_service;type=close");
    } else {
        // Иначе выгружаем сам апдейтер, т.к. он может быть запущен не из под WatchService
        m_Server->sendMessage(QString("sender=watch_service;target=%1;type=close")
                                  .arg(CWatchService::Modules::Updater)
                                  .toUtf8());
    }

    if (canClose) {
        toLog(LogLevel::Normal, "All running modules have been closed.");

        modulesClosed();
    }
}

//----------------------------------------------------------------------------
void WatchService::modulesClosed() {
    closeAction();
}

//----------------------------------------------------------------------------
void WatchService::checkScreenProtection() {
    if (!m_ScreenProtectionEnabled && (m_SplashScreen.isVisible())) {
        emit screenProtected();
    }

    if (m_ScreenProtectionEnabled) {
        for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
            if ((it->process) && (it->process->state() == QProcess::Running) &&
                (it->initDate < it->lastUpdate)) {
                if (it->gui) {
                    emit screenProtected();

                    return;
                }
            }
        }

        emit screenUnprotected();
    }
}

//----------------------------------------------------------------------------
bool WatchService::closeModule(SModule &aModule, bool aIgnorePriority) {
    if ((aModule.process) && (aModule.process->state() == QProcess::Running)) {
        aModule.needToStart = false;

        // Процесс должен быть завершён только тогда, когда завершились все процессы с большим
        // приоритетом
        if (canTerminate(aModule) || aIgnorePriority) {
            bool needToKill = false;

            if (aModule.name.toLower() == CWatchService::Modules::Updater) {
                needToKill = (aModule.lastUpdate.addSecs(CWatchService::SlowPC::KillModuleTimeout) <
                              QDateTime::currentDateTime());
            } else {
                needToKill = (aModule.lastUpdate.addSecs(aModule.killTimeout) <
                              QDateTime::currentDateTime());
            }

            if (needToKill) {
                toLog(LogLevel::Warning,
                      QString("Module %1 has been killed because close time has been exceeded.")
                          .arg(aModule.name));

                if (aModule.kill()) {
                    return true;
                }
            }

            toLog(LogLevel::Normal, QString("Sending close event to module %1.").arg(aModule.name));

            m_Server->sendMessage(
                QString("sender=watch_service;target=%1;type=close").arg(aModule.name).toUtf8());

            return false;
        }

        toLog(
            LogLevel::Normal,
            QString("Can't terminate %1 cause module with higher close priority is still running.")
                .arg(aModule.name));

        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
void WatchService::onScreenActivity(int aArea) {
    m_ScreenActivityTimer.start(CWatchService::ScreenActivityTimeout);

    if (m_Server) {
        m_ClickSequence += QString("%1").arg(aArea);
        QString hash = QCryptographicHash::hash(m_ClickSequence.toLatin1(), QCryptographicHash::Md5)
                           .toHex()
                           .toLower();
        m_Server->sendMessage(
            QString("sender=watch_service;type=screen_activity;%1").arg(hash).toLatin1());
        toLog(LogLevel::Normal, QString("Clicked sequence: %1.").arg(m_ClickSequence));
    }
}

//----------------------------------------------------------------------------
void WatchService::onScreenActivityTimeout() {
    m_ScreenActivityTimer.stop();
    m_ClickSequence.clear();
}

//----------------------------------------------------------------------------
void WatchService::enableScreenProtection(bool aEnabled) {
    m_ScreenProtectionEnabled = aEnabled;

    checkScreenProtection();
}

//----------------------------------------------------------------------------
QString WatchService::translateError(QProcess::ProcessError aError) {
    switch (aError) {
    case QProcess::FailedToStart:
        return QString(
            "The process failed to start. Either the invoked program is missing, or you may have "
            "insufficient permissions to invoke the program.");

    case QProcess::Crashed:
        return QString("The process crashed some time after starting successfully.");

    case QProcess::Timedout:
        return QString("The last waitFor...() function timed out. The state of QProcess is "
                       "unchanged, and you can try "
                       "calling waitFor...() again.");

    case QProcess::WriteError:
        return QString("An error occurred when attempting to write to the process. For example, "
                       "the process may not be "
                       "running, or it may have closed its input channel.");

    case QProcess::ReadError:
        return QString("An error occurred when attempting to read from the process. For example, "
                       "the process may "
                       "not be running.");

    case QProcess::UnknownError:
        return QString("An unknown error occurred.");

    default:
        return QString("An unknown error occurred.");
    }
}

//----------------------------------------------------------------------------
void WatchService::closeAction() {
    if (m_CloseAction == ECloseAction::None) {
        toLog(LogLevel::Normal, "Setting 5 min timeout to run modules again.");

        QTimer::singleShot(5 * 60 * 1000, this, [this]() { reinitialize(); });
    } else if (m_CloseAction == ECloseAction::Restart) {
        toLog(LogLevel::Normal, "Restarting service...");

        m_CloseAction = ECloseAction::None;

        reinitialize();
    } else if (m_CloseAction == ECloseAction::Reboot) {
        toLog(LogLevel::Normal, "Rebooting system...");

        /// TODO : отслеживать провал перезагрузки
        ISysUtils::system_Reboot();

        QCoreApplication::instance()->quit();
    } else if (m_CloseAction == ECloseAction::Exit) {
        toLog(LogLevel::Normal, "Stopping service...");

        QCoreApplication::instance()->quit();
    } else if (m_CloseAction == ECloseAction::Shutdown) {
        toLog(LogLevel::Normal, "Shutting down the terminal...");

        /// TODO : отслеживать провал выключения
        ISysUtils::system_Shutdown();

        QCoreApplication::instance()->quit();
    }
}

//----------------------------------------------------------------------------
void WatchService::startModule(SModule &aModule) {
    QString runCommand = QString("\"%1\" %2")
                             .arg(aModule.file)
                             .arg(aModule.params.isEmpty() ? aModule.arguments : aModule.params);

    toLog(LogLevel::Normal,
          QString("Starting module %1... Executing command %2.").arg(aModule.name).arg(runCommand));

    aModule.process->setWorkingDirectory(aModule.workingDirectory);
    aModule.process->start(runCommand);

    if (aModule.process->waitForStarted()) {
        toLog(LogLevel::Normal,
              QString("Module %1 has been successfully %2.")
                  .arg(aModule.name)
                  .arg(aModule.restartCount > 0 ? "restarted" : "started"));

        ++(aModule.restartCount);
        aModule.initDate = aModule.lastUpdate = QDateTime::currentDateTime();
        aModule.arguments.clear(); // параметры из командной строки передаются только один раз (для
                                   // запуска сервисного меню/первоначальной настройки)
    } else {
        toLog(LogLevel::Error,
              QString("Error occured while executing module %1. Code: %2 (%3).")
                  .arg(aModule.name)
                  .arg(aModule.process->error())
                  .arg(translateError(aModule.process->error())));
    }

    // Ждём указанный таймаут после попытки запуска...
    toLog(LogLevel::Normal, QString("Waiting %1 ms...").arg(aModule.afterStartDelay));

    SleepHelper::msleep(aModule.afterStartDelay);

    // Заполняем структуру использования памяти после старта процесса
    QTimer::singleShot(2 * 60 * 1000, this, [this]() { checkProcessMemory(); });
}

//----------------------------------------------------------------------------
void WatchService::checkProcessMemory() {
    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        SModule &module = *it;

        if (module.process && module.process->state() == QProcess::Running) {
            ISysUtils::MemoryInfo mi;
            if (ISysUtils::getProcessMemoryUsage(mi, module.process)) {
                if (module.memoryUsage.processUsed > 0) {
                    qint64 diff = mi.processUsed - module.memoryUsage.processUsed;
                    double diffPercent = 100. * qAbs(diff) / module.memoryUsage.processUsed;

                    toLog(diffPercent < 25. ? LogLevel::Normal : LogLevel::Warning,
                          QString("[%1] %2 Kb. Diff %3 Kb")
                              .arg(module.name)
                              .arg(mi.processUsed / 1024., 0, 'f', 2)
                              .arg(diff / 1024., 0, 'f', 2));
                } else {
                    // первая проверка объёма памяти
                    module.memoryUsage = mi;

                    toLog(LogLevel::Normal,
                          QString("[%1] %2 Kb")
                              .arg(module.name)
                              .arg(module.memoryUsage.processUsed / 1024., 0, 'f', 2));
                }
            }
        }
    }

    ISysUtils::MemoryInfo mi;
    if (ISysUtils::getProcessMemoryUsage(mi)) {
        toLog(LogLevel::Normal,
              QString("Physical memory. Total %1 Mb, used %2 Mb")
                  .arg(mi.total / 1024. / 1024., 0, 'f', 2)
                  .arg(mi.totalUsed / 1024. / 1024., 0, 'f', 2));
    }
}

//----------------------------------------------------------------------------
void WatchService::doReboot() {
    for (TModules::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it) {
        it->needToStart = false;
    }

    m_CloseAction = ECloseAction::Reboot;

    closeModules();
}

//----------------------------------------------------------------------------
int WatchService::defaultKillTimeout() {
    return cpuSpeed() < CWatchService::SlowPC::Threshold ? CWatchService::SlowPC::KillModuleTimeout
                                                         : CWatchService::KillModuleTimeout;
}

#if 0 // #40592 Пока выключаем данную опцию
//----------------------------------------------------------------------------
void WatchService::checkForbiddenModules()
{
	ProcessEnumerator processes;

	foreach(auto moduleName, m_ForbiddenModules)
	{
		for (auto it = processes.begin(); it != processes.end(); )
		{
			// Поиск процесса по его имени
			it = std::find_if(it, processes.end(), [&moduleName](const ProcessEnumerator::ProcessInfo & aPInfo) -> bool {
				return moduleName.size() && aPInfo.path.contains(moduleName, Qt::CaseInsensitive); });

			if (it != processes.end())
			{
				toLog(LogLevel::Normal, QString("Process '%1' will be killed.").arg(moduleName));

				quint32 error = 0;
				if (processes.kill(it->pid, error))
				{
					if (!m_TerminatedModules.contains(it->path, Qt::CaseInsensitive))
					{
						m_TerminatedModules.push_back(it->path);
					}
				}
				else
				{
					toLog(LogLevel::Error, QString("Failed kill process '%1' ID: %2 Error: %3.").arg(moduleName).arg(it->pid).arg(error));
				}

				it++;
			}
		}
	}
}

//----------------------------------------------------------------------------
void WatchService::startTerminatedModules()
{
	m_CheckMemoryTimer->stop();

	foreach(auto program, m_TerminatedModules)
	{
		toLog(LogLevel::Normal, QString("Restart process '%1'.").arg(program));

		QProcess::startDetached(program);
	}

	m_TerminatedModules.clear();
}
#endif
