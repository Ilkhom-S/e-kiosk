/* @file Обработчик команд работы с устройствами/сервисами получения денег. */

#include "Services/CashAcceptorManager.h"

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/IChargeProvider.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <numeric>

#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "FundsService.h"
#include "Services/DatabaseService.h"
#include "Services/DeviceService.h"
#include "Services/PaymentService.h"
#include "Services/PluginService.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CCashAcceptor {
const char CashPaymentMethod[] = "cash";
} // namespace CCashAcceptor

//---------------------------------------------------------------------------
bool CashAcceptorManager::SPaymentData::maxAmountReached() const {
    return !qFuzzyIsNull(maxAmount) &&
           (currentAmount > maxAmount || qFuzzyCompare(currentAmount, maxAmount));
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::SPaymentData::chargeSourceEmpty() const {
    return validators.empty() && chargeProviders.empty();
}

//---------------------------------------------------------------------------
CashAcceptorManager::CashAcceptorManager(IApplication *aApplication)
    : ILogable(CFundsService::LogName), m_Application(aApplication), m_Database(nullptr),
      m_DeviceService(nullptr), m_DisableAmountOverflow(false) {}

//---------------------------------------------------------------------------
CashAcceptorManager::~CashAcceptorManager() = default;

//---------------------------------------------------------------------------
bool CashAcceptorManager::initialize(IPaymentDatabaseUtils *aDatabase) {
    m_Database = aDatabase;

    auto *pluginLoader = PluginService::instance(m_Application)->getPluginLoader();
    QStringList providers = pluginLoader->getPluginList(QRegularExpression(
        QString("%1\\.%2\\..*").arg(PPSDK::Application, PPSDK::CComponents::ChargeProvider)));

    foreach (const QString &path, providers) {
        SDK::Plugin::IPlugin *plugin = pluginLoader->createPlugin(path);
        if (plugin) {
            auto *provider = dynamic_cast<PPSDK::IChargeProvider *>(plugin);
            if (provider) {
                m_ChargeProviders << provider;

                provider->subscribe(SDK::PaymentProcessor::CChargeProvider::StackedSignal,
                                    this,
                                    SLOT(onChargeProviderStacked(SDK::PaymentProcessor::SNote)));
            } else {
                pluginLoader->destroyPlugin(plugin);
            }
        }
    }

    m_DeviceService = DeviceService::instance(m_Application);

    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();

    m_DisableAmountOverflow = settings->getCommonSettings().disableAmountOverflow;

    if (settings->getCurrencySettings().id == -1) {
        toLog(LogLevel::Error, "Currency is not set for funds service!");

        return false;
    }

    // Передаем ид валюты - параметр, необходимый для инициации купюроприемников.
    QVariantMap params;
    params[CHardwareSDK::CashAcceptor::SystemCurrencyId] = settings->getCurrencySettings().id;
    m_DeviceService->setInitParameters(DSDK::CComponents::BillAcceptor, params);
    m_DeviceService->setInitParameters(DSDK::CComponents::CoinAcceptor, params);

    initWorkingParList();

    updateHardwareConfiguration();

    connect(m_DeviceService, SIGNAL(configurationUpdated()), SLOT(updateHardwareConfiguration()));

    return true;
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::shutdown() {
    foreach (DSDK::ICashAcceptor *acceptor, m_DeviceList) {
        m_DeviceService->releaseDevice(acceptor);
    }

    m_DeviceList.clear();

    foreach (SDK::PaymentProcessor::IChargeProvider *provider, m_ChargeProviders) {
        provider->unsubscribe(SDK::PaymentProcessor::CChargeProvider::StackedSignal, this);

        PluginService::instance(m_Application)
            ->getPluginLoader()
            ->destroyPlugin(dynamic_cast<SDK::Plugin::IPlugin *>(provider));
    }

    m_ChargeProviders.clear();
    m_PaymentData.clear();

    return true;
}

//---------------------------------------------------------------------------
QStringList CashAcceptorManager::getPaymentMethods() {
    PPSDK::IPaymentService *ps = m_Application->getCore()->getPaymentService();
    qint64 id = ps->getActivePayment();
    QString procType = ps->getPaymentField(id, PPSDK::CPayment::Parameters::Type).value.toString();

    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();
    QVariantMap chargeAccess = settings->getChargeProviderAccess();

    QSet<QString> result;

    auto checkMethod = [&chargeAccess, &procType](const QString &aName) -> bool {
        return (!chargeAccess.isEmpty() &&
                chargeAccess.value(procType).toStringList().contains(aName)) ||
               (chargeAccess.isEmpty() && !aName.isEmpty());
    };

    foreach (SDK::PaymentProcessor::IChargeProvider *provider, m_ChargeProviders) {
        QString method = provider->getMethod();

        if (checkMethod(method)) {
            result.insert(provider->getMethod());
        }
    }

    // Устройство добавляем в случае, если настройки оплаты для типов процессинга не заданы
    // Или должно быть соответствующее разрешение
    if (!m_DeviceList.isEmpty() &&
        (chargeAccess.isEmpty() || checkMethod(CCashAcceptor::CashPaymentMethod))) {
        // Добавляем способ оплаты наличными, если доступны устройства и настройки его разрешают.
        result.insert(CCashAcceptor::CashPaymentMethod);
    }

    return {result.begin(), result.end()};
}

//---------------------------------------------------------------------------
void CashAcceptorManager::updateHardwareConfiguration() {
    // Получаем список всех доступных устройств.
    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();
    QStringList deviceList = settings->getDeviceList().filter(
        QRegularExpression(QString("(%1|%2)")
                               .arg(DSDK::CComponents::BillAcceptor)
                               .arg(DSDK::CComponents::CoinAcceptor)));

    m_DeviceList.clear();

    foreach (const QString &configurationName, deviceList) {
        auto *device =
            dynamic_cast<DSDK::ICashAcceptor *>(m_DeviceService->acquireDevice(configurationName));

        if (device) {
            m_DeviceList.append(device);

            // Подписываемся на сигнал изменения статуса.
            device->subscribe(
                SDK::Driver::IDevice::StatusSignal,
                this,
                SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

            device->setParList(m_WorkingParList);
        } else {
            toLog(LogLevel::Error,
                  QString("Failed to acquire cash acceptor %1.").arg(configurationName));
        }
    }
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::enable(qint64 aPayment,
                                 const QString &aPaymentMethod,
                                 PPSDK::TPaymentAmount aMaxAmount) {
    if (m_PaymentData && m_PaymentData->paymentId == aPayment) {
        // Пересчитаем максимальную сумму платежа
        if (m_PaymentData->currentAmount == 0) {
            m_PaymentData->maxAmount = aMaxAmount;
        }

        if (m_PaymentData->maxAmountReached()) {
            return false;
        }

        // Продолжаем прием средств в контексте предыдущего платежа.
    } else {
        // Создаем новый контекст приема средств.
        SPaymentData paymentData(aPayment);

        paymentData.maxAmount = aMaxAmount;

        m_PaymentData = QSharedPointer<SPaymentData>(new SPaymentData(paymentData));
    }

    const QString method =
        aPaymentMethod.isEmpty() ? CCashAcceptor::CashPaymentMethod : aPaymentMethod;

    if (method == CCashAcceptor::CashPaymentMethod) {
        // Берем валидатор, который поддерживает нужный набор номиналов.
        foreach (DSDK::ICashAcceptor *acceptor, m_DeviceList) {
            if (!acceptor->isDeviceReady()) {
                toLog(LogLevel::Warning,
                      QString("Payment %1. Cash acceptor %2 is not ready.")
                          .arg(aPayment)
                          .arg(acceptor->getName()));

                continue;
            }

            if (acceptor->setEnable(true)) {
                // Подписываемся на сигналы эскроу и стекеда.
                if (m_DisableAmountOverflow) {
                    acceptor->subscribe(SDK::Driver::ICashAcceptor::EscrowSignal,
                                        this,
                                        SLOT(onEscrowChangeControl(SDK::Driver::SPar)));
                } else {
                    acceptor->subscribe(SDK::Driver::ICashAcceptor::EscrowSignal,
                                        this,
                                        SLOT(onEscrow(SDK::Driver::SPar)));
                }

                acceptor->subscribe(SDK::Driver::ICashAcceptor::StackedSignal,
                                    this,
                                    SLOT(onStacked(SDK::Driver::TParList)));

                m_PaymentData->validators.insert(acceptor);

                toLog(LogLevel::Debug,
                      QString("Payment %2. %1 was added.")
                          .arg(acceptor->getName())
                          .arg(m_PaymentData->paymentId));
            }
        }
    }

    // Готовы принимать средства от остальных поставщиков.
    foreach (PPSDK::IChargeProvider *provider, m_ChargeProviders) {
        if (provider->getMethod() == method && provider->enable(aMaxAmount)) {
            m_PaymentData->chargeProviders.insert(provider);
        }
    }

    if (m_PaymentData->chargeSourceEmpty()) {
        toLog(LogLevel::Error, QString("Payment %1. No funds sources available.").arg(aPayment));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::disable(qint64 aPayment) {
    toLog(LogLevel::Debug, "Disable all cash acceptors.");

    if (!m_PaymentData || m_PaymentData->paymentId != aPayment) {
        // Валидатор уже отключен.
        toLog(LogLevel::Debug, "No need to disable the cash acceptors due to data absent.");

        emit disabled(aPayment);

        return false;
    }

    // Отключаем источники средств.
    foreach (PPSDK::IChargeProvider *provider, m_ChargeProviders) {
        if (provider->disable()) {
            m_PaymentData->chargeProviders.remove(provider);
        }
    }

    // Даем команду на отключение устройств.
    foreach (DSDK::ICashAcceptor *acceptor, m_DeviceList) {
        if (m_PaymentData->validators.contains(acceptor) && !acceptor->setEnable(false)) {
            toLog(LogLevel::Error,
                  QString("Failed to disable cash acceptor %1.")
                      .arg(m_DeviceService->getDeviceConfigName(acceptor)));
        }
    }

    if (m_PaymentData->chargeSourceEmpty()) {
        toLog(LogLevel::Debug, "All cash acceptors were disabled already.");

        // Устройства уже отключены.
        emit disabled(aPayment);
    }

    return true;
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onEscrowChangeControl(const SDK::Driver::SPar &aPar) {
    auto *acceptor = dynamic_cast<DSDK::ICashAcceptor *>(sender());

    if (m_PaymentData && m_PaymentData->validators.contains(acceptor) &&
        !isFixedAmountPayment(m_PaymentData->paymentId) && !allowMoreMoney(aPar.nominal)) {
        toLog(LogLevel::Error,
              QString("Escrow will overflow max amount. Reject %1.").arg(aPar.nominal));

        if (!acceptor->reject()) {
            toLog(LogLevel::Error,
                  QString("Return command failed. Nominal : %1.").arg(aPar.currencyId));
        }

        emit warning(m_PaymentData->paymentId, "#overflow_amount");

        return;
    }

    onEscrow(aPar);
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onEscrow(const DSDK::SPar &aPar) {
    auto *acceptor = dynamic_cast<DSDK::ICashAcceptor *>(sender());

    if (m_PaymentData && m_PaymentData->validators.contains(acceptor)) {
        // TODO по флажку в настройках проверять будущую сумму платежа и выбрасывать, если
        // результирующая сумма больше maxAmount.

        if (m_PaymentData->maxAmountReached()) {
            if (!acceptor->reject()) {
                toLog(LogLevel::Error,
                      QString("Return command failed. Nominal : %1.").arg(aPar.currencyId));
            }
        } else if (!acceptor->stack()) {
            toLog(LogLevel::Error,
                  QString("Stack command failed. Nominal : %1.").arg(aPar.currencyId));
        }
    } else {
        toLog(LogLevel::Error,
              QString("Escrow to unknown payment. Nominal %1 will be rejected.")
                  .arg(aPar.currencyId));

        if (acceptor->reject()) {
            toLog(LogLevel::Error,
                  QString("Return command failed. Nominal : %1.").arg(aPar.currencyId));
        }
    }
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onStacked(DSDK::TParList aNotes) {
    auto *acceptor = dynamic_cast<DSDK::ICashAcceptor *>(sender());

    if (m_PaymentData && m_PaymentData->validators.contains(acceptor)) {
        double amount = std::accumulate(
            aNotes.begin(), aNotes.end(), 0.0, [](double acc, const DSDK::SPar &par) -> double {
                return acc + par.nominal;
            });
        auto paymentId = m_PaymentData->paymentId;

        // Записываем в БД информацию о принятых купюрах.
        if (!m_Database->addPaymentNote(
                paymentId,
                std::accumulate(aNotes.begin(),
                                aNotes.end(),
                                QList<SDK::PaymentProcessor::SNote>(),
                                [](QList<SDK::PaymentProcessor::SNote> &list,
                                   const DSDK::SPar &note) -> QList<SDK::PaymentProcessor::SNote> {
                                    return list << SDK::PaymentProcessor::SNote(
                                               note.cashReceiver ==
                                                       DSDK::ECashReceiver::CoinAcceptor
                                                   ? PPSDK::EAmountType::Coin
                                                   : PPSDK::EAmountType::Bill,
                                               note.nominal,
                                               note.currencyId,
                                               note.serialNumber);
                                }))) {
            toLog(LogLevel::Error,
                  QString("Payment %1. Failed to update payment amount. Total sum: %2.")
                      .arg(paymentId)
                      .arg(amount));
        }

        m_PaymentData->currentAmount += amount;

        emit PPSDK::ICashAcceptorManager::amountUpdated(
            paymentId, m_PaymentData->currentAmount, amount);

        if (m_PaymentData->maxAmountReached()) {
            disable(paymentId);
        }
    } else {
        // Сообщим о средствах, которые не попали в платеж
        QStringList lostMoney;
        foreach (SDK::Driver::SPar par, aNotes) {
            lostMoney.append(QString("%1: %2")
                                 .arg(par.cashReceiver == SDK::Driver::ECashReceiver::CoinAcceptor
                                          ? "Coin"
                                          : "Bill")
                                 .arg(par.nominal));
        }

        toLog(LogLevel::Error,
              QString("Funds was stacked but not add to payment. %1").arg(lostMoney.join("; ")));
    }
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onChargeProviderStacked(const SDK::PaymentProcessor::SNote &aNote) {
    if (m_PaymentData) {
        double amount = aNote.nominal;
        auto paymentId = m_PaymentData->paymentId;

        // Записываем в БД информацию о принятых купюрах.
        if (!m_Database->addPaymentNote(paymentId, aNote)) {
            toLog(LogLevel::Error,
                  QString("Payment %1. Failed to update payment amount. Total sum: %2.")
                      .arg(paymentId)
                      .arg(amount));
        }

        m_PaymentData->currentAmount += amount;

        emit PPSDK::ICashAcceptorManager::amountUpdated(
            paymentId, m_PaymentData->currentAmount, amount);

        if (m_PaymentData->maxAmountReached()) {
            disable(paymentId);
        }
    }
}

//---------------------------------------------------------------------------
void CashAcceptorManager::onStatusChanged(DSDK::EWarningLevel::Enum aLevel,
                                          const QString &aTranslation,
                                          int aStatus) {
    if (aLevel == DSDK::EWarningLevel::Error) {
        if (m_PaymentData) {
            // Прерываем прием средств.
            disable(m_PaymentData->paymentId);

            emit error(m_PaymentData->paymentId, aTranslation);
        }
    } else {
        auto *acceptor = dynamic_cast<DSDK::ICashAcceptor *>(sender());

        // Обработка различных статусов устройства.
        if (aStatus == DSDK::ECashAcceptorStatus::Rejected) {
            // TODO: не обрабатывать непринятые купюры по запрещенным номиналам.
            incrementRejectCount();

            emit activity();
        } else if (aStatus == DSDK::ECashAcceptorStatus::Cheated) {
            emit cheated(m_PaymentData ? m_PaymentData->paymentId : -1);
        } else if (acceptor && aStatus == DSDK::ECashAcceptorStatus::Disabled) {
            // Принят сигнал об отключении купюроприемника.
            toLog(LogLevel::Debug, acceptor->getName() + " is disabled.");

            // Отписываемся от сигналов эскроу и стекеда.
            acceptor->unsubscribe(SDK::Driver::ICashAcceptor::EscrowSignal, this);
            acceptor->unsubscribe(SDK::Driver::ICashAcceptor::StackedSignal, this);

            // Удаляем купюроприемник из списка активных.
            if (m_PaymentData) {
                toLog(LogLevel::Debug,
                      QString("Payment %2. %1 was removed.")
                          .arg(acceptor->getName())
                          .arg(m_PaymentData->paymentId));

                m_PaymentData->validators.remove(acceptor);

                // Если все источники средств отключены - завершаем прием платежа.
                if (m_PaymentData->chargeSourceEmpty()) {
                    toLog(LogLevel::Debug, "All cash acceptors are disabled.");

                    emit disabled(m_PaymentData->paymentId);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
int CashAcceptorManager::getRejectCount() const {
    auto *dbUtils =
        DatabaseService::instance(m_Application)->getDatabaseUtils<IHardwareDatabaseUtils>();
    return dbUtils
        ->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                         PPSDK::CDatabaseConstants::Parameters::RejectCount)
        .toInt();
}

//---------------------------------------------------------------------------
void CashAcceptorManager::incrementRejectCount() {
    auto *dbUtils =
        DatabaseService::instance(m_Application)->getDatabaseUtils<IHardwareDatabaseUtils>();
    dbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                            PPSDK::CDatabaseConstants::Parameters::RejectCount,
                            getRejectCount() + 1);
}

//---------------------------------------------------------------------------
void CashAcceptorManager::initWorkingParList() {
    // Получаем список поддерживаемых номиналов.
    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();
    PPSDK::SCommonSettings commonSettings = settings->getCommonSettings();

    foreach (auto nominal, commonSettings.enabledParNotesList) {
        m_WorkingParList.append(DSDK::SPar(nominal,
                                           settings->getCurrencySettings().id,
                                           DSDK::ECashReceiver::BillAcceptor,
                                           commonSettings.isValid));
    }

    foreach (auto nominal, commonSettings.enabledParCoinsList) {
        m_WorkingParList.append(DSDK::SPar(nominal,
                                           settings->getCurrencySettings().id,
                                           DSDK::ECashReceiver::CoinAcceptor,
                                           commonSettings.isValid));
    }
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::isFixedAmountPayment(qint64 aPayment) {
    PPSDK::IPaymentService *paymentService = PaymentService::instance(m_Application);

    auto providerID =
        paymentService->getPaymentField(aPayment, PPSDK::CPayment::Parameters::Provider);

    if (!providerID.isNull()) {
        auto limits = paymentService->getProvider(providerID.value.toLongLong()).limits;

        if (limits.min == limits.max) {
            return true;
        }

        auto minAmount =
            paymentService->getPaymentField(aPayment, PPSDK::CPayment::Parameters::MinAmount);
        auto maxAmount =
            paymentService->getPaymentField(aPayment, PPSDK::CPayment::Parameters::MaxAmount);

        if (!minAmount.isNull() && !maxAmount.isNull() &&
            qFuzzyCompare(minAmount.value.toDouble(), maxAmount.value.toDouble())) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------
bool CashAcceptorManager::allowMoreMoney(SDK::PaymentProcessor::TPaymentAmount aAmount) {
    QList<PPSDK::IPayment::SParameter> parameters;

    PPSDK::IPaymentService *paymentService = PaymentService::instance(m_Application);

    parameters = paymentService->getPaymentFields(m_PaymentData->paymentId);

    auto amountAll =
        std::find_if(parameters.begin(),
                     parameters.end(),
                     [](const PPSDK::IPayment::SParameter &aParameter) -> bool {
                         return aParameter.name == PPSDK::CPayment::Parameters::AmountAll;
                     });
    Q_ASSERT(amountAll != parameters.end());

    double newAmountAll = amountAll->value.toDouble() + aAmount;

    parameters.clear();
    parameters << PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll, newAmountAll);
    parameters = paymentService->calculateCommission(parameters);

    auto change = std::find_if(parameters.begin(),
                               parameters.end(),
                               [](const PPSDK::IPayment::SParameter &aParameter) -> bool {
                                   return aParameter.name == PPSDK::CPayment::Parameters::Change;
                               });
    Q_ASSERT(change != parameters.end());

    return qFuzzyIsNull(change->value.toDouble());
}

//---------------------------------------------------------------------------
