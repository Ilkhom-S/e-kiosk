/* @file Сервис для работы с устройствами приема наличных. */

#include "Services/FundsService.h"

#include <QtCore/QRegularExpression>

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/IChargeProvider.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <numeric>

#include "DatabaseUtils/IHardwareDatabaseUtils.h"

namespace CFundsService {
const char LogName[] = "Funds";
} // namespace CFundsService
#include "Services/CashAcceptorManager.h"
#include "Services/CashDispenserManager.h"
#include "Services/DatabaseService.h"
#include "Services/DeviceService.h"
#include "Services/PluginService.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
FundsService *FundsService::instance(IApplication *aApplication) {
    return static_cast<FundsService *>(
        aApplication->getCore()->getService(CServices::FundsService));
}

//---------------------------------------------------------------------------
FundsService::FundsService(IApplication *aApplication)
    : ILogable(CFundsService::LogName), m_Application(aApplication),
      m_CashDispenserManager(nullptr), m_CashAcceptorManager(nullptr) {}

//---------------------------------------------------------------------------
FundsService::~FundsService() {}

//---------------------------------------------------------------------------
bool FundsService::initialize() {
    if (!m_CashAcceptorManager) {
        m_CashAcceptorManager = new CashAcceptorManager(m_Application);
        m_CashAcceptorManager->setParent(this);
    }

    if (!m_CashDispenserManager) {
        m_CashDispenserManager = new CashDispenserManager(m_Application);
        m_CashDispenserManager->setParent(this);
    }

    auto database =
        DatabaseService::instance(m_Application)->getDatabaseUtils<IPaymentDatabaseUtils>();

    return m_CashAcceptorManager->initialize(database) &&
           m_CashDispenserManager->initialize(database);
}

//------------------------------------------------------------------------------
void FundsService::finishInitialize() {}

//---------------------------------------------------------------------------
bool FundsService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool FundsService::shutdown() {
    if (m_CashAcceptorManager) {
        m_CashAcceptorManager->shutdown();
        delete m_CashAcceptorManager;
        m_CashAcceptorManager = nullptr;
    }

    if (m_CashDispenserManager) {
        m_CashDispenserManager->shutdown();
        delete m_CashDispenserManager;
        m_CashDispenserManager = nullptr;
    }

    return true;
}

//---------------------------------------------------------------------------
QString FundsService::getName() const {
    return CServices::FundsService;
}

//---------------------------------------------------------------------------
const QSet<QString> &FundsService::getRequiredServices() const {
    static QSet<QString> requiredServices = QSet<QString>() << CServices::SettingsService
                                                            << CServices::DeviceService
                                                            << CServices::DatabaseService;

    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap FundsService::getParameters() const {
    QVariantMap parameters;

    parameters[PPSDK::CServiceParameters::Funds::RejectCount] =
        m_CashAcceptorManager->getRejectCount();

    return parameters;
}

//---------------------------------------------------------------------------
void FundsService::resetParameters(const QSet<QString> &aParameters) {
    if (aParameters.contains(PPSDK::CServiceParameters::Funds::RejectCount)) {
        auto dbUtils =
            DatabaseService::instance(m_Application)->getDatabaseUtils<IHardwareDatabaseUtils>();
        dbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                PPSDK::CDatabaseConstants::Parameters::RejectCount,
                                0);
    }
}

//------------------------------------------------------------------------------
QString FundsService::getState() const {
    // Получаем список всех доступных устройств.
    PPSDK::TerminalSettings *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();
    QStringList deviceList = settings->getDeviceList().filter(
        QRegularExpression(QString("(%1|%2)")
                               .arg(DSDK::CComponents::BillAcceptor)
                               .arg(DSDK::CComponents::CoinAcceptor)));

    QStringList result;

    foreach (const QString &configurationName, deviceList) {
        DSDK::IDevice *device =
            DeviceService::instance(m_Application)->acquireDevice(configurationName);
        if (device) {
            QStringList dd = device->getDeviceConfiguration()
                                 .value(CHardwareSDK::DeviceData)
                                 .toString()
                                 .split("\n");
            if (!dd.isEmpty()) {
                QVariantMap ddMap;
                foreach (QString param, dd) {
                    if (param.isEmpty())
                        continue;

                    QStringList pp = param.split(":");
                    if (!pp.isEmpty()) {
                        ddMap.insert(pp.first().trimmed(), pp.last().trimmed());
                    }
                }

                result << DeviceService::instance(m_Application)
                              ->getDeviceConfiguration(configurationName)
                              .value(CHardwareSDK::ModelName)
                              .toString()
                       << ddMap.value(CHardwareSDK::SerialNumber).toString()
                       << ddMap.value("firmware").toString();
            }
        }
    }

    return result.join(";");
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ICashAcceptorManager *FundsService::getAcceptor() const {
    return m_CashAcceptorManager;
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ICashDispenserManager *FundsService::getDispenser() const {
    return m_CashDispenserManager;
}

//---------------------------------------------------------------------------
