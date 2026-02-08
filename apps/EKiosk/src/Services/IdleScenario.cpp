/* @file Сценарий простоя. */

// stl

#include "IdleScenario.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QMetaEnum>

#include <SDK/Drivers/CashAcceptor/CashAcceptorStatus.h>
#include <SDK/Drivers/DeviceTypes.h>
#include <SDK/PaymentProcessor/Core/DatabaseConstants.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <algorithm>

#include "EventService.h"
#include "GUIService.h"
#include "SettingsService.h"
#include "System/IApplication.h"

namespace CGUISignals {
const char StartGUI[] = "start_ui";
const char StopGUI[] = "stop_ui";
const char UpdateGUI[] = "update_ui";
const char ScreenPasswordUpdated[] = "screen_password_updated";
} // namespace CGUISignals
#include "TerminalService.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
IdleScenario::IdleScenario(IApplication *aApplication)
    : Scenario("Idle", aApplication->getLog()), m_Application(aApplication),
      m_Command(Command::None), m_Active(false), m_NoGui(false), m_InterfaceLockedTimer(0) {
    m_Application->getCore()->getEventService()->subscribe(
        this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));
}

//---------------------------------------------------------------------------
IdleScenario::~IdleScenario() {
    // Показываем маскирующий экран.
    auto watchServiceClient = TerminalService::instance(m_Application)->getClient();
    watchServiceClient->showSplashScreen();
}

//---------------------------------------------------------------------------
bool IdleScenario::initialize(const QList<GUI::SScriptObject> & /*aScriptObjects*/) {
    return true;
}

//---------------------------------------------------------------------------
void IdleScenario::start(const QVariantMap &aContext) {
    m_NoGui = aContext.value("no_gui").toBool();

    QString startScenario = m_Application->getArguments().value("-start_scenario").toString();
    QString args = m_Application->getArguments().value("-args").toString();

    toLog(LogLevel::Debug, QString("start_scenario=%1, args=%2").arg(startScenario).arg(args));

    QVariantMap parameters;
    if (!startScenario.isEmpty()) {
        parameters.insert("name", startScenario);
        parameters.insert("args", args);

        EventService::instance(m_Application)
            ->sendEvent(PPSDK::EEventType::StartScenario, parameters);
    } else {
        // Показываем экран блокировки
        GUIService::instance(m_Application)->show("SplashScreen", QVariantMap());

        if (!m_NoGui) {
            updateState(CGUISignals::StartGUI, QVariantMap());
        } else {
            EventService::instance(m_Application)
                ->sendEvent(PPSDK::Event(PPSDK::EEventType::PauseGraphics));
            toLog(LogLevel::Normal, QString("DISABLED GUI by interface.ini"));
        }
    }

    m_Active = true;

    // Прячем маскирующий экран.
    auto watchServiceClient = TerminalService::instance(m_Application)->getClient();
    watchServiceClient->hideSplashScreen();
}

//---------------------------------------------------------------------------
void IdleScenario::pause() {
    m_Active = false;

    if (m_NoGui) {
        EventService::instance(m_Application)
            ->sendEvent(PPSDK::Event(PPSDK::EEventType::StartGraphics));
    }
}

//---------------------------------------------------------------------------
void IdleScenario::resume(const QVariantMap & /*aContext*/) {
    if (!m_NoGui) {
        // Показываем экран блокировки
        GUIService::instance(m_Application)->show("SplashScreen", QVariantMap());
    } else {
        EventService::instance(m_Application)
            ->sendEvent(PPSDK::Event(PPSDK::EEventType::PauseGraphics));
    }

    updateState("resume", QVariantMap());

    m_Active = true;
}

//---------------------------------------------------------------------------
void IdleScenario::signalTriggered(const QString &aSignal, const QVariantMap &aArguments) {
    updateState(aSignal, aArguments);
}

//---------------------------------------------------------------------------
bool IdleScenario::canStop() {
    return true;
}

//---------------------------------------------------------------------------
QString IdleScenario::getState() const {
    return QString("main");
}

//---------------------------------------------------------------------------
void IdleScenario::onTimeout() {}

//---------------------------------------------------------------------------
void IdleScenario::execCommand() {
    switch (m_Command) {
    case Command::Autoencashment: {
        // Запускаем автоинкассацию.
        QVariantMap parameters;
        parameters["name"] = "autoencashment";
        EventService::instance(m_Application)
            ->sendEvent(PPSDK::EEventType::StartScenario, parameters);
        break;
    }
    }

    m_Command = Command::None;
}

//---------------------------------------------------------------------------
void IdleScenario::updateState(const QString &aSignal, const QVariantMap &aParameters) {
    auto guiService = GUIService::instance(m_Application);

    // Запускаем сервисное меню.
    if (aSignal == CGUISignals::ScreenPasswordUpdated) {
        QString sequence = aParameters.value("password").toString();
        auto password = SettingsService::instance(m_Application)
                            ->getAdapter<PPSDK::TerminalSettings>()
                            ->getServiceMenuPasswords();

        if (QCryptographicHash::hash(sequence.toLatin1(), QCryptographicHash::Md5)
                .toHex()
                .toLower() == password.passwords[PPSDK::CServiceMenuPasswords::Screen].toLower()) {
            QVariantMap parameters;
            parameters["name"] = "service_menu";
            EventService::instance(m_Application)
                ->sendEvent(PPSDK::EEventType::StartScenario, parameters);
        }

        return;
    }

    // Проверяем наличие необработанной команды.
    if (m_Command != Command::None) {
        execCommand();
        return;
    }

    // Проверяем наличие устройств с ошибкой.
    auto terminalService = TerminalService::instance(m_Application);
    auto faultyDevices = terminalService->getFaultyDevices();
    auto faultyDeviceNames = faultyDevices.keys();

    QStringList cashAcceptors = terminalService->getAcceptorTypes();
    QStringList faultyCashDeviceNames;
    QStringList cashDeviceNames;

    foreach (const QString &name, faultyDeviceNames) {
        if (cashAcceptors.contains(name.section('.', 2, 2))) {
            faultyCashDeviceNames << name;
        }
    }

    QStringList fullDeviceList = terminalService->getDeviceNames();

    foreach (const QString &name, fullDeviceList) {
        if (cashAcceptors.contains(name.section('.', 2, 2))) {
            cashDeviceNames << name;
        }
    }

    auto notHaveDevice = [&fullDeviceList](const QString &aDeviceType) -> bool {
        return std::find_if(fullDeviceList.begin(),
                            fullDeviceList.end(),
                            [&](const QString &aConfigName) -> bool {
                                return aConfigName.section('.', 2, 2) == aDeviceType;
                            }) == fullDeviceList.end();
    };

    bool hasValidatorError =
        QSet<QString>(cashDeviceNames.begin(), cashDeviceNames.end()) ==
        QSet<QString>(faultyCashDeviceNames.begin(), faultyCashDeviceNames.end());

    auto findFaultyDeviceType = [&faultyDeviceNames](const QString &aDeviceType) -> bool {
        return std::find_if(faultyDeviceNames.begin(),
                            faultyDeviceNames.end(),
                            [&](const QString &aConfigName) -> bool {
                                return aConfigName.section('.', 2, 2) == aDeviceType;
                            }) != faultyDeviceNames.end();
    };

    bool hasPrinterError = findFaultyDeviceType(SDK::Driver::CComponents::Printer) ||
                           findFaultyDeviceType(SDK::Driver::CComponents::DocumentPrinter) ||
                           findFaultyDeviceType(SDK::Driver::CComponents::FiscalRegistrator);
    bool hasCardReaderError = findFaultyDeviceType(SDK::Driver::CComponents::CardReader);

    auto settings = SettingsService::instance(m_Application)
                        ->getAdapter<PPSDK::TerminalSettings>()
                        ->getCommonSettings();

    bool hasKeysError = terminalService->isTerminalError(PPSDK::ETerminalError::KeyError);
    bool hasAccountBalanceError =
        terminalService->isTerminalError(PPSDK::ETerminalError::AccountBalanceError);
    bool hasConfigError =
        !settings.isValid || terminalService->isTerminalError(PPSDK::ETerminalError::ConfigError);

    QVariantMap parameters;

    // Обновляем значки на экране блокировки.
    auto insertParam = [&parameters](const QString &aKey, bool aValue) {
        if (aValue) {
            parameters.insert(aKey, aValue);
        }
    };

    insertParam("blocked", terminalService->isLocked());
    insertParam("validator_failure",
                hasValidatorError || (settings.blockOn(PPSDK::SCommonSettings::ValidatorError) &&
                                      notHaveDevice(SDK::Driver::CComponents::BillAcceptor)));
    insertParam("cardreader_failure",
                hasCardReaderError || (settings.blockOn(PPSDK::SCommonSettings::CardReaderError) &&
                                       notHaveDevice(SDK::Driver::CComponents::CardReader)));
    insertParam("printer_failure",
                hasPrinterError || (settings.blockOn(PPSDK::SCommonSettings::PrinterError) &&
                                    notHaveDevice(SDK::Driver::CComponents::Printer) &&
                                    notHaveDevice(SDK::Driver::CComponents::DocumentPrinter) &&
                                    notHaveDevice(SDK::Driver::CComponents::FiscalRegistrator)));
    insertParam("crypt_failure", hasKeysError);
    insertParam("config_failure", hasConfigError);
    insertParam("account_balance_failure", hasAccountBalanceError);

#ifdef TC_USE_TOKEN
    insertParam("token_failure", findFaultyDeviceType(SDK::Driver::CComponents::Token));
#endif

    guiService->notify("SplashScreen", parameters);

    // Удаляем из списка блокировок, если ошибка в настройках указана как некритичная
    auto removeIfNotCritical = [&parameters](const QString &aKey, bool aValue) {
        if (!aValue) {
            parameters.remove(aKey);
        }
    };

    removeIfNotCritical("validator_failure",
                        settings.blockOn(PPSDK::SCommonSettings::ValidatorError));
    removeIfNotCritical("printer_failure", settings.blockOn(PPSDK::SCommonSettings::PrinterError));
    removeIfNotCritical("cardreader_failure",
                        settings.blockOn(PPSDK::SCommonSettings::CardReaderError));
    removeIfNotCritical("account_balance_failure",
                        settings.blockOn(PPSDK::SCommonSettings::AccountBalance));

    if (parameters.isEmpty()) {
        // Критичных ошибок нет. Запускаем главное меню.
        guiService->disable(false);

        EventService::instance(m_Application)
            ->sendEvent(PPSDK::EEventType::StartScenario, QVariantMap());
        EventService::instance(m_Application)
            ->sendEvent(SDK::PaymentProcessor::Event(
                SDK::PaymentProcessor::EEventType::OK, getName(), "OK"));

        if (m_InterfaceLockedTimer) {
            killTimer(m_InterfaceLockedTimer);
            m_InterfaceLockedTimer = 0;

            toLog(LogLevel::Normal, "Interface lock timer killed.");
        }

        terminalService->setTerminalError(PPSDK::ETerminalError::InterfaceLocked, false);
        toLog(LogLevel::Normal, QString("GUI unlocked."));
    } else {
        toLog(LogLevel::Error,
              QString("GUI locked by: %1.").arg(QStringList(parameters.keys()).join(",")));

        // Создаём отложенное выставление сигнала о блокировке интерфейса
        if (m_InterfaceLockedTimer == 0 &&
            !terminalService->isTerminalError(PPSDK::ETerminalError::InterfaceLocked)) {
            m_InterfaceLockedTimer = startTimer(1 * 60 * 1000);

            toLog(LogLevel::Normal, "Interface lock timer started.");
        }
    }
}

//---------------------------------------------------------------------------
void IdleScenario::onEvent(const PPSDK::Event &aEvent) {
    bool processed = true;

    switch (aEvent.getType()) {
    case PPSDK::EEventType::Autoencashment:
        m_Command = Command::Autoencashment;
        break;

    default:
        processed = false;
    }

    // Если команду невозможно выполнить в текущем сценарии - забудем о ней R.I.P.
    if (processed) {
        if (GUIService::instance(m_Application)->canDisable()) {
            GUIService::instance(m_Application)->disable(true);
        } else {
            toLog(LogLevel::Warning,
                  QString("Can't execute IdleScenario command '%1'.")
                      .arg(metaObject()
                               ->enumerator(metaObject()->indexOfEnumerator("Command"))
                               .valueToKey(m_Command)));

            m_Command = Command::None;
        }
    }
}

//---------------------------------------------------------------------------
void IdleScenario::timerEvent(QTimerEvent *aEvent) {
    if (aEvent && aEvent->timerId() == m_InterfaceLockedTimer) {
        toLog(LogLevel::Normal, "Interface lock timer triggered.");

        killTimer(m_InterfaceLockedTimer);
        m_InterfaceLockedTimer = 0;

        // Терминал заблокирован.
        auto terminalService = TerminalService::instance(m_Application);
        terminalService->setTerminalError(PPSDK::ETerminalError::InterfaceLocked, true);
    }
}

//---------------------------------------------------------------------------
