/* @file Менеджер запуcка задач по раcпиcанию. */

#include <QtCore/QDir>
#include <QtCore/QMetaType>
#include <QtCore/QSet>
#include <QtCore/QString>

#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <Services/SchedulerService.h>
#include <Services/ServiceCommon.h>
#include <Services/ServiceNames.h>
#include <SysUtils/ISysUtils.h>
#include <System/IApplication.h>
#include <System/SettingsConstants.h>
#include <random>

namespace PPSDK = SDK::PaymentProcessor;

namespace CScheduler {
const QString ConfigName = "/data/scheduler.ini";
const QString UserConfigName = "/user/scheduler_config.ini";
const QString ThreadName = "SchedulerThread";
const QString LogName = "Scheduler";
const QString TimeFormat = "hh:mm";
const QString DateTimeFormat = "yyyy.MM.dd hh:mm:ss";

const int StartTimeIfExpired =
    10 * 60; // таймаут в cекундах для cлучая, еcли мы пропуcтили запуcк задачи

namespace Config {
const QString Type = "type";
const QString Params = "params";
const QString Time = "time";     // конкретное время запуcка задачи
const QString Period = "period"; // Еcли time не задано, то берётcя период запуcка в cекундах
const QString TriggeredOnStart = "triggered_on_start"; // Запускать первый раз сразу при старте
const QString RepeatCountIfFail = "repeat_count_if_fail";
const QString TimeThreshold =
    "time_threshold"; // максимальный порог времени, добавляемый рандомно к времени запуска задачи
const QString RetryTimeout =
    "retry_timeout";                  // таймаут повторного запуска в случае неуспеха в cекундах
const QString OnlyOnce = "only_once"; // запускать задачу только один раз

const QString StartupTime = "startup";     // запускать после каждого старта ПО
const QString AfterFirstRun = "first_run"; // задача запускающаяся 1 раз после первой установки
} // namespace Config

namespace UserConfig {
const QString LastExecute = "last_execute";
const QString FailExecuteCounter = "fail_execute_counter";
} // namespace UserConfig

const QString AutoUpdateTaskName = "AutoUpdate";
const QString DisplayOnOffTaskName = "DisplayOnOff";
} // namespace CScheduler

//---------------------------------------------------------------------------
SchedulerService *SchedulerService::instance(IApplication *aApplication) {
    return static_cast<SchedulerService *>(
        aApplication->getCore()->getService(CServices::SchedulerService));
}

//---------------------------------------------------------------------------
SchedulerService::SchedulerService(IApplication *aApplication)
    : m_Application(aApplication), ILogable(CScheduler::LogName) {
    m_Thread.setObjectName(CScheduler::ThreadName);

    moveToThread(&m_Thread);

    // для запуcка таймеров вcех заданий
    connect(&m_Thread, SIGNAL(started()), this, SLOT(scheduleAll()));
}

//---------------------------------------------------------------------------
SchedulerService::~SchedulerService() {}

//---------------------------------------------------------------------------
bool SchedulerService::initialize() {
    QSettings settings(ISysUtils::rm_BOM(IApplication::toAbsolutePath(
                           IApplication::getWorkingDirectory() + CScheduler::ConfigName)),
                       QSettings::IniFormat);
    QSettings userSettings(ISysUtils::rm_BOM(IApplication::toAbsolutePath(
                               IApplication::getWorkingDirectory() + CScheduler::UserConfigName)),
                           QSettings::IniFormat);

    foreach (QString taskName, settings.childGroups()) {
        settings.beginGroup(taskName);
        userSettings.beginGroup(taskName);

        Item item(taskName, settings, userSettings);

        if (!m_Factory.contains(item.type()) && !m_ExternalTasks.contains(item.name())) {
            toLog(LogLevel::Error,
                  QString("[%1]: Unknown task type '%2'.").arg(item.name()).arg(item.type()));
        } else if (!item.isOK()) {
            toLog(LogLevel::Error, QString("[%1]: Invalid configuration. Skipped.").arg(taskName));
        } else {
            m_Items.insert(item.name(), item);
            toLog(LogLevel::Normal, QString("[%1]: Loaded.").arg(taskName));
        }
        settings.endGroup();
        userSettings.endGroup();
    }

    setupAutoUpdate();
    setupDisplayOnOff();

    toLog(LogLevel::Normal, "Scheduler service initialized.");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, RAND_MAX);
    // Seed with time-based value for reproducibility in this context
    QTime currentTime = QTime::currentTime();
    gen.seed(unsigned(currentTime.hour() + currentTime.minute() + currentTime.second() +
                      currentTime.msec()));

    return true;
}

//---------------------------------------------------------------------------
void SchedulerService::setupAutoUpdate() {
    PPSDK::TerminalSettings *terminalSettings = static_cast<PPSDK::TerminalSettings *>(
        m_Application->getCore()->getSettingsService()->getAdapter(
            PPSDK::CAdapterNames::TerminalAdapter));

    QTime startTime = terminalSettings->autoUpdate();

    if (startTime.isValid() && !startTime.isNull()) {
        toLog(LogLevel::Normal,
              QString("Enabled auto update at '%1'.")
                  .arg(startTime.toString(CScheduler::TimeFormat)));

        // удаляем все задачи обновления
        foreach (auto task, m_Items.values()) {
            if (task.type() == "RunUpdater") {
                m_Items.remove(task.name());
                toLog(LogLevel::Normal, QString("Removed task [%1].").arg(task.name()));
            }
        }

        QSettings settings;
        settings.beginGroup(CScheduler::AutoUpdateTaskName);
        settings.setValue(CScheduler::Config::Type, "RunUpdater");
        settings.setValue(CScheduler::Config::Time, startTime.toString(CScheduler::TimeFormat));
        // что бы терминалы с одинаковыми настройками не ломанулись обновляться все одновременно
        // (анти DDOS)
        settings.setValue(CScheduler::Config::TimeThreshold, 3600);
        settings.setValue(CScheduler::Config::RepeatCountIfFail, 2);
        settings.setValue(CScheduler::Config::RetryTimeout, 3600);

        QSettings userSettings(IApplication::toAbsolutePath(IApplication::getWorkingDirectory() +
                                                            CScheduler::UserConfigName),
                               QSettings::IniFormat);
        userSettings.beginGroup(CScheduler::AutoUpdateTaskName);

        Item item(CScheduler::AutoUpdateTaskName, settings, userSettings);
        m_Items.insert(item.name(), item);
        toLog(LogLevel::Normal, QString("[%1]: Loaded.").arg(item.name()));
    }
}

//------------------------------------------------------------------------------
SchedulerService::Item::Item()
    : m_Period(0), m_TriggeredOnStart(false), m_TimeThreshold(0), m_RepeatCountIfFail(0),
      m_RetryTimeout(0), m_OnlyOnce(false), m_FailExecuteCounter(0) {}

//------------------------------------------------------------------------------
SchedulerService::Item::Item(const QString &aName,
                             const QSettings &aSettings,
                             const QSettings &aUserSettings)
    : m_Name(aName), m_FailExecuteCounter(0), m_OnlyOnce(false) {
    m_Type = aSettings.value(CScheduler::Config::Type).toString();

    QVariant params = aSettings.value(CScheduler::Config::Params);
    m_Params = params.typeId() == QMetaType::QStringList ? params.toStringList().join(",")
                                                         : params.toString();

    m_Period = aSettings.value(CScheduler::Config::Period, "0").toInt();
    m_TriggeredOnStart = aSettings.value(CScheduler::Config::TriggeredOnStart, "false")
                             .toString()
                             .compare("true", Qt::CaseInsensitive) == 0;
    m_RepeatCountIfFail = aSettings.value(CScheduler::Config::RepeatCountIfFail, "0").toInt();
    m_TimeThreshold = aSettings.value(CScheduler::Config::TimeThreshold, "0").toInt();
    m_RetryTimeout = aSettings.value(CScheduler::Config::RetryTimeout, "-1").toInt();

    m_LastExecute =
        QDateTime::fromString(aUserSettings.value(CScheduler::UserConfig::LastExecute).toString(),
                              CScheduler::DateTimeFormat);
    m_FailExecuteCounter =
        aUserSettings.value(CScheduler::UserConfig::FailExecuteCounter, 0).toInt();

    QString timeStr = aSettings.value(CScheduler::Config::Time, "-1").toString();
    if (timeStr == CScheduler::Config::StartupTime) {
        m_Time = QTime::currentTime().addSecs(60);
        m_LastExecute = QDateTime::currentDateTime().addDays(-2);
        m_OnlyOnce = true;
    } else if (timeStr == CScheduler::Config::AfterFirstRun) {
        m_OnlyOnce = true;
        m_Period = -1;
        if (m_LastExecute.isValid() && !m_LastExecute.isNull()) {
            m_LastExecute = QDateTime::currentDateTime().addSecs(-20);
            m_Time = QTime::currentTime().addSecs(-30);
        } else {
            m_Time = QTime::currentTime().addSecs(60 * 60);
        }
    } else {
        m_Time = QTime::fromString(timeStr, CScheduler::TimeFormat);
        m_OnlyOnce = aSettings.value(CScheduler::Config::OnlyOnce, false).toBool();
    }
}

//------------------------------------------------------------------------------
bool SchedulerService::Item::isOK() const {
    return m_Time.isValid() || m_Period > 0;
}

//------------------------------------------------------------------------------
bool SchedulerService::Item::execute(SDK::PaymentProcessor::ITask *aTask, ILog *aLog) {
    if (isOK() && aTask) {
        LOG(aLog, LogLevel::Normal, QString("[%1]: Execute").arg(m_Name));

        if (m_FailExecuteCounter) {
            LOG(aLog,
                LogLevel::Normal,
                QString("[%1]: Restart #%2 time.").arg(m_Name).arg(m_FailExecuteCounter));
        }

        m_LastExecute = QDateTime::currentDateTime();

        aTask->execute();

        return true;
    } else {
        LOG(aLog,
            LogLevel::Error,
            QString("[%1]: Error create object '%2'.").arg(m_Name).arg(m_Type));

        return false;
    }
}

//------------------------------------------------------------------------------
QTimer *SchedulerService::Item::createTimer() {
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);

    // если последний запуск неудачный, то проверяем нужно ли перезапустить задачу
    if (m_FailExecuteCounter && m_FailExecuteCounter <= m_RepeatCountIfFail &&
        m_RetryTimeout >= 0) {
        timer->setInterval(m_RetryTimeout * 1000);
        return timer;
    }

    m_FailExecuteCounter = 0;

    if (m_Time.isValid()) // запуcк задачи в определенное время
    {
        QDate today = QDate::currentDate();
        QTime now = QTime::currentTime();

        int intervalToTomorrowStart =
            now.secsTo(QTime(23, 59, 59, 999)) + QTime(0, 0, 0, 1).secsTo(m_Time);

        if (m_LastExecute.date() < today) // cегодня не запуcкали
        {
            if (m_Time <= now) {
                // пропуcтили время запуcка
                timer->setInterval(qMin(CScheduler::StartTimeIfExpired, intervalToTomorrowStart) *
                                   1000);
            } else {
                // не пропуcтили, запуcкаем как положено
                timer->setInterval(now.secsTo(m_Time) * 1000);
            }
        } else {
            // cегодня уже запуcкали - выcтавляем таймер на завтра
            timer->setInterval(intervalToTomorrowStart * 1000);
        }
    } else // запуcк задачи через интервалы времени
    {
        if (m_Period > 0) {
            if (m_TriggeredOnStart) {
                timer->setInterval(60 * 1000);

                m_TriggeredOnStart = false;
            } else {
                timer->setInterval(m_Period * 1000);
            }
        } else {
            // невалидный период запуcка
            delete timer;
            timer = nullptr;
        }
    }

    if (timer && m_TimeThreshold > 0) {
        // добавляем рандомное время для запуска задачи
        timer->setInterval(timer->interval() + (rand() * m_TimeThreshold / RAND_MAX) * 1000);
    }

    return timer;
}

//------------------------------------------------------------------------------
void SchedulerService::Item::complete(bool aComplete) {
    if (aComplete) {
        m_FailExecuteCounter = 0;

        if (m_OnlyOnce) {
            // делаем задачу невалидной, что бы больше не запускалась
            m_Time = QTime();
            m_Period = -1;
        }
    } else {
        ++m_FailExecuteCounter;
    }
}

//------------------------------------------------------------------------------
void SchedulerService::finishInitialize() {
    m_Thread.start();
}

//------------------------------------------------------------------------------
void SchedulerService::scheduleAll() {
    foreach (auto key, m_Items.keys()) {
        schedule(m_Items[key]);
    }
}

//------------------------------------------------------------------------------
bool SchedulerService::schedule(SchedulerService::Item &aItem) const {
    QTimer *timer = aItem.createTimer();
    if (timer) {
        timer->setObjectName(aItem.name());

        toLog(LogLevel::Normal,
              QString("[%1] scheduled to '%2'.")
                  .arg(aItem.name())
                  .arg(QDateTime::currentDateTime()
                           .addMSecs(timer->interval())
                           .toString(CScheduler::DateTimeFormat)));

        connect(timer, SIGNAL(timeout()), this, SLOT(execute()));
        timer->start();
        return true;
    } else {
        if (!aItem.onlyOnce()) {
            toLog(LogLevel::Error, QString("Error of scheduling [%1].").arg(aItem.name()));
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void SchedulerService::execute() {
    QTimer *timer = dynamic_cast<QTimer *>(sender());
    if (timer) {
        timer->stop();

        if (m_Items.contains(timer->objectName())) {
            SchedulerService::Item &item = m_Items[timer->objectName()];
            timer->deleteLater();

            SDK::PaymentProcessor::ITask *task;

            // Проверим, что таск может быть пользовательским
            if (m_ExternalTasks[item.name()]) {
                task = m_ExternalTasks[item.name()];
            } else {
                task = m_Factory[item.type()](item.name(), CScheduler::LogName, item.params());
            }

            if (task) {
                {
                    QWriteLocker locker(&m_Lock);

                    m_WorkingTasks.insert(item.name(), task);
                }

                task->subscribeOnComplete(this, SLOT(onTaskComplete(const QString &, bool)));

                if (!item.execute(task, getLog())) {
                    onTaskComplete(item.name(), false);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
void SchedulerService::onTaskComplete(const QString &aName, bool aComplete) {
    SDK::PaymentProcessor::ITask *task = nullptr;

    {
        QWriteLocker locker(&m_Lock);

        task = m_WorkingTasks.value(aName);
        m_WorkingTasks.remove(aName);
    }

    if (task && task != m_ExternalTasks[aName]) {
        try {
            QObject *taskObject = dynamic_cast<QObject *>(task);

            if (taskObject) {
                taskObject->deleteLater();
            } else {
                delete task;
            }

            task = nullptr;
        } catch (...) {
        }
    }

    m_Items[aName].complete(aComplete);

    if (aComplete) {
        toLog(LogLevel::Normal, QString("[%1]: Done.").arg(aName));
    } else {
        toLog(LogLevel::Error, QString("[%1]: Error executing.").arg(aName));
    }

    saveLastExecute(m_Items[aName], aComplete);

    schedule(m_Items[aName]);
}

//------------------------------------------------------------------------------
void SchedulerService::saveLastExecute(Item &aItem, bool aComplete) {
    QSettings settings(IApplication::toAbsolutePath(IApplication::getWorkingDirectory() +
                                                    CScheduler::UserConfigName),
                       QSettings::IniFormat);

    settings.beginGroup(aItem.name());
    settings.setValue(CScheduler::UserConfig::LastExecute,
                      aItem.lastExecute().toString(CScheduler::DateTimeFormat));
    settings.setValue(CScheduler::UserConfig::FailExecuteCounter, aItem.failCount());
    settings.endGroup();
}

//---------------------------------------------------------------------------
bool SchedulerService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool SchedulerService::shutdown() {
    {
        QReadLocker locker(&m_Lock);

        foreach (auto task, m_WorkingTasks.values()) {
            task->cancel();
        }
    }

    SafeStopServiceThread(&m_Thread, 3000, getLog());

    return true;
}

//---------------------------------------------------------------------------
QString SchedulerService::getName() const {
    return CServices::SchedulerService;
}

//---------------------------------------------------------------------------
const QSet<QString> &SchedulerService::getRequiredServices() const {
    // TODO Как отслеживать зависимости тасков от нужных сервисов?
    static QSet<QString> requiredServices =
        QSet<QString>() << CServices::SettingsService << CServices::EventService
                        << CServices::TerminalService << CServices::RemoteService
                        << CServices::NetworkService;

    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap SchedulerService::getParameters() const {
    return QVariantMap();
}

//---------------------------------------------------------------------------
void SchedulerService::resetParameters(const QSet<QString> &) {}

//---------------------------------------------------------------------------
void SchedulerService::setupDisplayOnOff() {
    PPSDK::TerminalSettings *terminalSettings = static_cast<PPSDK::TerminalSettings *>(
        m_Application->getCore()->getSettingsService()->getAdapter(
            PPSDK::CAdapterNames::TerminalAdapter));

    QString energySave = terminalSettings->energySave();

    if (energySave.split(";", Qt::SkipEmptyParts).size() >= 2) {
        toLog(LogLevel::Normal, QString("Enabled energy saving: %1.").arg(energySave));

        QSettings settings;
        settings.beginGroup(CScheduler::DisplayOnOffTaskName);
        settings.setValue(CScheduler::Config::Type, "OnOffDisplay");
        settings.setValue(CScheduler::Config::Period, "300");
        settings.setValue(CScheduler::Config::Params, energySave);

        QSettings userSettings(IApplication::toAbsolutePath(IApplication::getWorkingDirectory() +
                                                            CScheduler::UserConfigName),
                               QSettings::IniFormat);
        userSettings.beginGroup(CScheduler::DisplayOnOffTaskName);

        Item item(CScheduler::DisplayOnOffTaskName, settings, userSettings);
        m_Items.insert(item.name(), item);
        toLog(LogLevel::Normal, QString("[%1]: Loaded.").arg(item.name()));
    }
}

//---------------------------------------------------------------------------
