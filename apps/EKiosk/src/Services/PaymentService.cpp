/* @file Сервис, владеющий платёжными потоками. */

#include "Services/PaymentService.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QCryptographicHash>
#include <QtCore/QMutexLocker>
#include <QtCore/QRegularExpression>

#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/HookConstants.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <Crypt/ICryptEngine.h>
#include <boost/foreach.hpp>
#include <utility>

#include "DatabaseUtils/IPaymentDatabaseUtils.h"
#include "Services/CryptService.h"
#include "Services/DatabaseService.h"
#include "Services/EventService.h"
#include "Services/HookService.h"
#include "Services/NetworkService.h"
#include "Services/PluginService.h"
#include "Services/ServiceCommon.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"

#if QT_VERSION < 0x050000
#include <Qt5Port/qt5port.h>
#endif

namespace PPSDK = SDK::PaymentProcessor;

using namespace std::placeholders;

//---------------------------------------------------------------------------
namespace CPaymentService {
/// Название нити, обрабатывающей оффлайн платежи.
const char ThreadName[] = "PaymentThread";

/// Таймаут обработки очереди оффлайн платежей.
const int ProcessOfflineTimeout = 1 * 1000;

/// Таймаут онлайн платежа, когда он гарантированно переводится в BAD
const int OnlinePaymentOvertime = 30 * 60 * 1000;

/// Таймаут обработки очереди оффлайн платежей при отсутствии связи.
const int CheckNetworkConnectionTimeout = 5 * 1000;

/// Тип платежа, в который будет добавляться неизрасходованная сдача.
const char ChangePaymentType[] = "humo";

/// Признак того, что в платеже хранится неизрасходованная сдача.
const char ChangePaymentParam[] = "UNUSED_CHANGE";
} // namespace CPaymentService

//---------------------------------------------------------------------------
PaymentService *PaymentService::instance(IApplication *aApplication) {
    return dynamic_cast<PaymentService *>(
        aApplication->getCore()->getService(CServices::PaymentService));
}

//---------------------------------------------------------------------------
PaymentService::PaymentService(IApplication *aApplication)
    : ILogable("Payments"), m_Application(aApplication), m_Enabled(false), m_DBUtils(nullptr),
      m_CommandIndex(0), m_OfflinePaymentID(-1) {
    qRegisterMetaType<EPaymentCommandResult::Enum>("EPaymentCommandResult");

    m_PaymentThread.setObjectName(CPaymentService::ThreadName);

    m_PaymentTimer.setSingleShot(true);
    m_PaymentTimer.moveToThread(&m_PaymentThread);

    connect(
        &m_PaymentThread, SIGNAL(started()), SLOT(onPaymentThreadStarted()), Qt::DirectConnection);
    connect(
        &m_PaymentThread, SIGNAL(finished()), &m_PaymentTimer, SLOT(stop()), Qt::DirectConnection);

    connect(&m_PaymentTimer, SIGNAL(timeout()), SLOT(onProcessPayments()), Qt::DirectConnection);
}

//---------------------------------------------------------------------------
bool PaymentService::initialize() {
    connect(m_Application->getCore()->getFundsService()->getAcceptor(),
            SIGNAL(amountUpdated(qint64, double, double)),
            SLOT(onAmountUpdated(qint64, double, double)));
    connect(m_Application->getCore()->getFundsService()->getDispenser(),
            SIGNAL(dispensed(double)),
            SLOT(onAmountDispensed(double)));

    m_CommandIndex = 1;

    m_DBUtils = DatabaseService::instance(m_Application)->getDatabaseUtils<IPaymentDatabaseUtils>();
    if (!m_DBUtils) {
        toLog(LogLevel::Error, "Failed to get database utils.");

        return false;
    }

    QStringList factories =
        PluginService::instance(m_Application)
            ->getPluginLoader()
            ->getPluginList(QRegularExpression("PaymentProcessor\\.PaymentFactory\\..*"));

    foreach (const QString &path, factories) {
        SDK::Plugin::IPlugin *plugin =
            PluginService::instance(m_Application)->getPluginLoader()->createPlugin(path);

        auto *factory = dynamic_cast<SDK::PaymentProcessor::IPaymentFactory *>(plugin);
        if (factory) {
            factory->setSerializer(
                [this](auto &&PH1) { return savePayment(std::forward<decltype(PH1)>(PH1)); });

            foreach (const QString &type, factory->getSupportedPaymentTypes()) {
                m_FactoryByType[type] = factory;
            }

            m_Factories << factory;
        } else {
            PluginService::instance(m_Application)->getPluginLoader()->destroyPlugin(plugin);
        }
    }

    // Ищем всех провайдеров с неподдерживаемым типом процессинга
    auto *dealerSettings = SettingsService::instance(m_Application)
                               ->getAdapter<SDK::PaymentProcessor::DealerSettings>();

    foreach (const QString &processingType,
             QSet<QString>(dealerSettings->getProviderProcessingTypes().begin(),
                           dealerSettings->getProviderProcessingTypes().end())
                 .subtract(
                     QSet<QString>(m_FactoryByType.keys().begin(), m_FactoryByType.keys().end()))) {
        // И удаляем их
        foreach (qint64 providerId, dealerSettings->getProviders(processingType)) {
            toLog(LogLevel::Error,
                  QString("Drop provider %1. Unsupported processing type: '%2'.")
                      .arg(providerId)
                      .arg(processingType));

            dealerSettings->disableProvider(providerId);
        }
    }

    return true;
}

//------------------------------------------------------------------------------
void PaymentService::finishInitialize() {
    foreach (auto factory, m_Factories) {
        if (!factory->initialize()) {
            toLog(LogLevel::Error,
                  QString("Failed initialize payment factory plugin for payment types: %1.")
                      .arg(factory->getSupportedPaymentTypes().join(", ")));
        }
    }

    // Запускаем поток на обработку оффлайн платежей.
    m_PaymentThread.start();

    m_Enabled = true;
}

//---------------------------------------------------------------------------
bool PaymentService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool PaymentService::shutdown() {
    m_Enabled = false;

    // Спрашиваем сетевой сервис может ли он закрыться. Тем самым сбрасывая все сетевые задачи.
    m_Application->getCore()->getService(CServices::NetworkService)->canShutdown();

    disconnect(m_Application->getCore()->getFundsService()->getAcceptor());
    disconnect(m_Application->getCore()->getFundsService()->getDispenser());

    m_ActivePaymentSynchronizer.waitForFinished();

    SafeStopServiceThread(&m_PaymentThread, 3000, getLog());

    if (m_ChangePayment) {
        m_ChangePayment.reset();
    }

    setPaymentActive(std::shared_ptr<PPSDK::IPayment>());

    while (!m_Factories.isEmpty()) {
        PluginService::instance(m_Application)
            ->getPluginLoader()
            ->destroyPlugin(dynamic_cast<SDK::Plugin::IPlugin *>(m_Factories.takeFirst()));
    }

    return true;
}

//---------------------------------------------------------------------------
QString PaymentService::getName() const {
    return CServices::PaymentService;
}

//---------------------------------------------------------------------------
const QSet<QString> &PaymentService::getRequiredServices() const {
    static QSet<QString> requiredServices =
        QSet<QString>() << CServices::SettingsService << CServices::EventService
                        << CServices::PluginService << CServices::DatabaseService
                        << CServices::CryptService;

    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap PaymentService::getParameters() const {
    // TODO Заполнить параметры
    QVariantMap parameters;
    parameters[PPSDK::CServiceParameters::Payment::UnprocessedPaymentCount] = "";
    parameters[PPSDK::CServiceParameters::Payment::PaymentsPerDay] = "";

    return parameters;
}

//---------------------------------------------------------------------------
void PaymentService::resetParameters(const QSet<QString> & /*aParameters*/) {}

//---------------------------------------------------------------------------
void PaymentService::setPaymentActive(std::shared_ptr<SDK::PaymentProcessor::IPayment> aPayment) {
    QMutexLocker lock(&m_PaymentLock);

    m_ActivePayment = std::move(aPayment);
}

//---------------------------------------------------------------------------
bool PaymentService::isPaymentActive(const std::shared_ptr<PPSDK::IPayment> &aPayment) {
    QMutexLocker lock(&m_PaymentLock);

    return aPayment == m_ActivePayment;
}

//---------------------------------------------------------------------------
qint64 PaymentService::createPayment(qint64 aProvider) {
    using namespace std::placeholders;

    if (getActivePayment() != -1) {
        setPaymentActive(std::shared_ptr<PPSDK::IPayment>());
    }

    toLog(LogLevel::Normal, QString("Creating payment. Provider: %1.").arg(aProvider));

    PPSDK::SProvider provider = SettingsService::instance(m_Application)
                                    ->getAdapter<SDK::PaymentProcessor::DealerSettings>()
                                    ->getProvider(aProvider);
    if (provider.isNull()) {
        toLog(LogLevel::Error, QString("Failed to get settings for provider %1.").arg(aProvider));

        return -1;
    }

    if (m_FactoryByType.contains(provider.processor.type)) {
        PPSDK::IPaymentFactory *factory = m_FactoryByType[provider.processor.type];

        std::shared_ptr<PPSDK::IPayment> payment(
            factory->createPayment(provider.processor.type),
            [factory](auto &&PH1) { factory->releasePayment(std::forward<decltype(PH1)>(PH1)); });
        if (!payment) {
            toLog(LogLevel::Error, "Failed to create payment object.");

            return -1;
        }

        QList<PPSDK::IPayment::SParameter> parameters;

        parameters << PPSDK::IPayment::SParameter(
                          PPSDK::CPayment::Parameters::ID, m_DBUtils->createDummyPayment(), true)
                   << PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::CreationDate,
                                                  QDateTime::currentDateTime(),
                                                  true)
                   << PPSDK::IPayment::SParameter(
                          PPSDK::CPayment::Parameters::Type, provider.processor.type, true)
                   << PPSDK::IPayment::SParameter(
                          PPSDK::CPayment::Parameters::Provider, provider.id, true)
                   << PPSDK::IPayment::SParameter(
                          PPSDK::CPayment::Parameters::Status, PPSDK::EPaymentStatus::Init, true)
                   << PPSDK::IPayment::SParameter(
                          PPSDK::CPayment::Parameters::Priority, PPSDK::IPayment::Online, true);

        if (!payment->restore(parameters)) {
            toLog(LogLevel::Error, "Failed to initialize payment object.");

            return -1;
        }

        if (payment->getID() == -1) {
            toLog(LogLevel::Error, "Failed to create payment ID.");

            return -1;
        }

        // Добавляем платёж в список активных
        setPaymentActive(payment);

        return payment->getID();
    }

    toLog(LogLevel::Error,
          QString("Failed to find factory for payment type %1.").arg(provider.processor.type));

    return -1;
}

//---------------------------------------------------------------------------
bool PaymentService::savePayment(PPSDK::IPayment *aPayment) {
    if (m_DBUtils->savePayment(aPayment, createSignature(aPayment))) {
        EventService::instance(m_Application)
            ->sendEvent(PPSDK::Event(
                PPSDK::EEventType::PaymentUpdated, CServices::PaymentService, aPayment->getID()));

        return true;
    }
    QString amountAll =
        aPayment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toString();
    QString msg = QString("Payment [%1] error write to database (AMOUNT_ALL=%2).")
                      .arg(aPayment->getInitialSession())
                      .arg(amountAll);

    // Выставить ошибочный статус устройства "терминал"
    m_Application->getCore()->getEventService()->sendEvent(SDK::PaymentProcessor::Event(
        SDK::PaymentProcessor::EEventType::Critical, CServices::DatabaseService, msg));

    return false;
}

//---------------------------------------------------------------------------
qint64 PaymentService::getActivePayment() const {
    return m_ActivePayment ? m_ActivePayment->getID() : -1;
}

//---------------------------------------------------------------------------
void PaymentService::deactivatePayment() {
    if (m_ActivePayment) {
        QMutexLocker lock(&m_PaymentLock);

        savePayment(m_ActivePayment.get());

        m_ActivePayment.reset();
    }

    // Если существует пустой платеж "сдача", нужно его пометить в БД как удаленный
    if (m_ChangePayment && qFuzzyIsNull(getChangeAmount())) {
        resetChange();
    }
}

//---------------------------------------------------------------------------
QString PaymentService::createSignature(PPSDK::IPayment *aPayment, bool aWithCRC /*= true*/) {
    if (!aPayment) {
        toLog(LogLevel::Error, "Failed to create payment signature. No payment specified.");

        return "";
    }

    QString signature;

    PPSDK::SProvider provider = SettingsService::instance(m_Application)
                                    ->getAdapter<SDK::PaymentProcessor::DealerSettings>()
                                    ->getProvider(aPayment->getProvider(false));

    PPSDK::SKeySettings keys = SettingsService::instance(m_Application)
                                   ->getAdapter<PPSDK::TerminalSettings>()
                                   ->getKeys()[provider.processor.keyPair];

    signature += QString("SD: %1, AP: %2, OP: %3\n").arg(keys.sd).arg(keys.ap).arg(keys.op);

    // В подпись включаем все суммы платежа.
    QStringList controlParameters;
    controlParameters << PPSDK::CPayment::Parameters::Amount
                      << PPSDK::CPayment::Parameters::AmountAll
                      << PPSDK::CPayment::Parameters::Change << PPSDK::CPayment::Parameters::Fee;

    signature += "Sums:\n";

    foreach (const QString &parameter, controlParameters) {
        signature += parameter + " : " + aPayment->getParameter(parameter).value.toString() + "\n";
    }

    signature += "Provider fields:\n";

    // Включаем в подпись все поля данных оператора
    QStringList providerFields;

    foreach (const PPSDK::SProviderField &field, provider.fields) {
        if (!field.keepEncrypted()) {
            providerFields << field.id;
        }
    }

    if (providerFields.isEmpty()) {
        // Описания оператора нет, берём названия полей из базы.
        providerFields = aPayment->getParameter(PPSDK::CPayment::Parameters::ProviderFields)
                             .value.toString()
                             .split(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter);
    }

    providerFields.sort();

    foreach (const QString &field, providerFields) {
        signature += field + " : " + aPayment->getParameter(field).value.toString() + "\n";
    }

    // Включаем в подпись основные поля платежа.
    signature += "id\tcreate_date\tinitial_session\tsession\toperator\tstatus\tstep\n";

    signature += QString::number(aPayment->getID());
    signature += "\t";
    signature += aPayment->getCreationDate().toString("yyyy-MM-dd hh:mm:ss.zzz");
    signature += "\t";
    signature += aPayment->getInitialSession();
    signature += "\t";
    signature += aPayment->getSession();
    signature += "\t";
    signature += QString::number(aPayment->getProvider(false));
    signature += "\t";
    signature += QString::number(aPayment->getStatus());
    signature += "\t";
    signature += aPayment->getParameter(PPSDK::CPayment::Parameters::Step).value.toString();

    if (aWithCRC) {
        signature +=
            QString("\n%1:%2")
                .arg(PPSDK::CPayment::Parameters::CRC)
                .arg(aPayment->getParameter(PPSDK::CPayment::Parameters::CRC).value.toString());

#ifdef _DEBUG
    }
#else

#if QT_VERSION < 0x050000
        signature = QString::fromLatin1(
            CCryptographicHash::hash(signature.toUtf8(), CCryptographicHash::Sha256).toHex());
#else
        signature = QString::fromLatin1(
            QCryptographicHash::hash(signature.toUtf8(), QCryptographicHash::Sha256).toHex());
#endif
    } else {
        signature =
            QString::fromUtf8(QCryptographicHash::hash(signature.toUtf8(), QCryptographicHash::Md5)
                                  .toHex()
                                  .toUpper());
    }
#endif

#ifndef _DEBUG
    auto *crypt = CryptService::instance(m_Application)->getCryptEngine();

    QByteArray encodedSignature;

    QString error;

    if (!crypt->encryptLong(-1, signature.toUtf8(), encodedSignature, error)) {
        toLog(LogLevel::Error,
              QString("Payment %1. Failed to encrypt signature. Error: %2.")
                  .arg(aPayment->getID())
                  .arg(error));

        return {};
    }

    return QString::fromUtf8(encodedSignature);
#else
    return signature;
#endif // _DEBUG
}

//---------------------------------------------------------------------------
bool PaymentService::verifySignature(PPSDK::IPayment *aPayment) {
    QString signature =
        aPayment->getParameter(PPSDK::CPayment::Parameters::Signature).value.toString();

    if (signature.isEmpty()) {
        toLog(LogLevel::Error, QString("Payment %1. No signature.").arg(aPayment->getID()));
        return false;
    }

    QStringList runtimeSignatures;

    runtimeSignatures << createSignature(aPayment, true) << createSignature(aPayment, false);

#ifndef _DEBUG
    auto *crypt = CryptService::instance(m_Application)->getCryptEngine();

    QString error;
    QByteArray decodedSignature;

    if (!crypt->decryptLong(-1, signature.toUtf8(), decodedSignature, error)) {
        toLog(LogLevel::Warning,
              QString("Payment %1. Failed to decrypt signature. Error: %2.")
                  .arg(aPayment->getID())
                  .arg(error));

        return false;
    }

    foreach (auto runtimeSignature, runtimeSignatures) {
        QByteArray decodedRuntimeSignature;

        if (!crypt->decryptLong(-1, runtimeSignature.toUtf8(), decodedRuntimeSignature, error)) {
            toLog(LogLevel::Warning,
                  QString("Payment %1. Failed to decrypt runtime signature. Error: %2.")
                      .arg(aPayment->getID())
                      .arg(error));
        } else if (decodedSignature == decodedRuntimeSignature) {
            return true;
        }
    }

    return false;
#else
    return runtimeSignatures.contains(signature);
#endif // _DEBUG
}

//---------------------------------------------------------------------------
std::shared_ptr<PPSDK::IPayment> PaymentService::getPayment(qint64 aID) {
    {
        QMutexLocker lock(&m_PaymentLock);

        // Если спрашиваем активный платёж, не перезагружаем его из базы.
        if (m_ActivePayment && (m_ActivePayment->getID() == aID)) {
            return m_ActivePayment;
        }
    }

    if (aID == -1) {
        toLog(LogLevel::Debug, QString("Payment %1. Payment not exist.").arg(aID));

        return {};
    }

    toLog(LogLevel::Normal, QString("Payment %1. Loading...").arg(aID));

    QList<PPSDK::IPayment::SParameter> parameters = m_DBUtils->getPaymentParameters(aID);

    PPSDK::IPayment::SParameter type =
        PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::Type, parameters);

    if (!type.isNull() && m_FactoryByType.contains(type.value.toString())) {
        std::shared_ptr<PPSDK::IPayment> payment(
            m_FactoryByType[type.value.toString()]->createPayment(type.value.toString()),
            [capture0 = m_FactoryByType[type.value.toString()]](auto &&PH1) {
                capture0->releasePayment(std::forward<decltype(PH1)>(PH1));
            });
        if (!payment) {
            toLog(LogLevel::Normal,
                  QString("Payment %1. Failed to create payment object.").arg(aID));

            return {};
        }

        if (!payment->restore(parameters)) {
            toLog(LogLevel::Normal,
                  QString("Payment %1. Failed to restore payment object.").arg(aID));

            return {};
        }

        if (!verifySignature(payment.get())) {
            toLog(LogLevel::Warning, QString("Payment %1. Cheated.").arg(payment->getID()));

            payment->setStatus(PPSDK::EPaymentStatus::Cheated);

            savePayment(payment.get());
        }

        return payment;
    }

    return {};
}

//---------------------------------------------------------------------------
std::shared_ptr<PPSDK::IPayment> PaymentService::getPayment(const QString &aInitialSession) {
    qint64 id = m_DBUtils->getPaymentByInitialSession(aInitialSession);

    if (id != -1) {
        return getPayment(id);
    }

    return {};
}

//---------------------------------------------------------------------------
PPSDK::SProvider PaymentService::getProvider(qint64 aID) {
    PPSDK::SProvider provider = SettingsService::instance(m_Application)
                                    ->getAdapter<SDK::PaymentProcessor::DealerSettings>()
                                    ->getProvider(aID);
    if (!provider.isNull()) {
        if (m_FactoryByType.contains(provider.processor.type)) {
            provider = m_FactoryByType[provider.processor.type]->getProviderSpecification(provider);
        } else {
            toLog(LogLevel::Error,
                  QString("Provider #%1: has unknown payment processor type: %2.")
                      .arg(aID)
                      .arg(provider.processor.type));
        }
    }

    return provider;
}

//---------------------------------------------------------------------------
PPSDK::IPayment::SParameter PaymentService::getPaymentField(qint64 aPayment, const QString &aName) {
    return PPSDK::IPayment::parameterByName(aName, getPaymentFields(aPayment));
}

//---------------------------------------------------------------------------
TPaymentParameters PaymentService::getPaymentFields(qint64 aPayment) {
    if (aPayment == getActivePayment()) {
        std::shared_ptr<PPSDK::IPayment> payment = getPayment(aPayment);

        return payment ? payment->getParameters() : QList<PPSDK::IPayment::SParameter>();
    }
    return m_DBUtils->getPaymentParameters(aPayment);
}

//------------------------------------------------------------------------------
QMap<qint64, TPaymentParameters> PaymentService::getPaymentsFields(const QList<qint64> &aIds) {
    QMap<qint64, TPaymentParameters> result = m_DBUtils->getPaymentParameters(aIds);

    if (aIds.contains(getActivePayment())) {
        std::shared_ptr<PPSDK::IPayment> payment = getPayment(getActivePayment());

        if (payment) {
            // объединяем два списка параметров
            QMap<QString, SDK::PaymentProcessor::IPayment::SParameter> parameters;

            foreach (auto param, payment->getParameters()) {
                parameters.insert(param.name, param);
            }

            // из базы более старые параметры
            foreach (auto param, result.value(getActivePayment())) {
                if (!parameters.contains(param.name)) {
                    parameters.insert(param.name, param);
                }
            }

            result.insert(getActivePayment(), parameters.values());
        }
    }

    return result;
}

//---------------------------------------------------------------------------
QList<PPSDK::IPayment::SParameter>
PaymentService::calculateCommission(const QList<PPSDK::IPayment::SParameter> &aParameters) {
    if (m_ActivePayment) {
        return m_ActivePayment->calculateCommission(aParameters);
    }

    return {};
}

//---------------------------------------------------------------------------
bool PaymentService::updatePaymentField(qint64 aID,
                                        const PPSDK::IPayment::SParameter &aField,
                                        bool aForceUpdate) {
    QList<PPSDK::IPayment::SParameter> parameters;

    parameters.append(aField);

    return updatePaymentFields(aID, parameters, aForceUpdate);
}

//---------------------------------------------------------------------------
bool PaymentService::updatePaymentFields(
    qint64 aID,
    const QList<SDK::PaymentProcessor::IPayment::SParameter> &aFields,
    bool aForceUpdate) {
    QMutexLocker lock(&m_OfflinePaymentLock);

    if (m_OfflinePaymentID != aID) {
        doUpdatePaymentFields(aID, getPayment(aID), aFields);
    } else if (aForceUpdate) {
        // m_OfflinePaymentLock гарантирует что мы попали на запись параметра ДО сохранения оффлайн
        // платежа в БД сохраняем параметр в объект, обслуживаемый в оффлайне
        doUpdatePaymentFields(aID, m_OfflinePayment, aFields, aForceUpdate);
        // тут же сохраняем объект в базу напрямую
        doUpdatePaymentFields(aID, getPayment(aID), aFields, aForceUpdate);
    } else {
        lock.unlock();

        // Создаем команду на обновление поля.
        QMutexLocker lock(&m_CommandMutex);

        auto command = [aID, aFields](PaymentService *aService) -> EPaymentCommandResult::Enum {
            aService->doUpdatePaymentFields(aID, aService->getPayment(aID), aFields);
            aService->m_PaymentHaveUnsavedParameters.remove(aID);
            return EPaymentCommandResult::OK;
        };

        m_Commands << qMakePair(
            m_CommandIndex++,
            std::function<EPaymentCommandResult::Enum(PaymentService *)>(command));
        m_PaymentHaveUnsavedParameters.insert(aID);
    }

    // TODO: убрать возвращаемое значение.
    return true;
}

//---------------------------------------------------------------------------
void PaymentService::processPaymentStep(qint64 aPayment,
                                        SDK::PaymentProcessor::EPaymentStep::Enum aStep,
                                        bool aBlocking) {
    if (aBlocking) {
        std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));
        if (!payment) {
            toLog(LogLevel::Error, QString("Payment %1. Failed to perform step.").arg(aPayment));

            emit stepCompleted(aPayment, aStep, true);
        } else {
            if (aStep == PPSDK::EPaymentStep::Pay) {
                payment->setCompleteDate(QDateTime::currentDateTime());
            }

            bool result = payment->perform_Step(aStep);

            savePayment(payment.get());

            emit stepCompleted(aPayment, aStep, !result);
        }
    } else {
        m_ActivePaymentSynchronizer.addFuture(QtConcurrent::run(
            [this, aPayment, aStep]() { processPaymentStep(aPayment, aStep, true); }));
    }
}

//---------------------------------------------------------------------------
bool PaymentService::convertPayment(qint64 aPayment, const QString &aTargetType) {
    if (m_FactoryByType.contains(aTargetType)) {
        std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

        if (payment) {
            if (m_FactoryByType[aTargetType]->convertPayment(aTargetType, payment.get())) {
                savePayment(payment.get());

                if (isPaymentActive(payment)) {
                    QMutexLocker lock(&m_PaymentLock);

                    // Заменяем активный платёж на новый экземпляр с другим типом процессинга
                    m_ActivePayment.reset();
                    setPaymentActive(getPayment(aPayment));
                }

                toLog(LogLevel::Normal,
                      QString("Payment %1. Converted to type %2.").arg(aPayment).arg(aTargetType));

                return true;
            }
        }
    }

    toLog(LogLevel::Error,
          QString("Payment %1. Failed convert to %2 type.").arg(aPayment).arg(aTargetType));

    return false;
}

//---------------------------------------------------------------------------
bool PaymentService::processPayment(qint64 aPayment, bool aOnline) {
    std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

    if (!payment) {
        toLog(LogLevel::Error, QString("Payment %1 not available.").arg(aPayment));
        return false;
    }

    // Блокируем проведение платежа с "финальными" статусами
    switch (payment->getStatus()) {
    case PPSDK::EPaymentStatus::Init:
    case PPSDK::EPaymentStatus::ReadyForCheck:
    case PPSDK::EPaymentStatus::ProcessError:
        break;
    case PPSDK::EPaymentStatus::Completed: // А сюда можем попасть досрочно, если обработка платежа
                                           // происходит в плагине.
        return true;
    default:
        toLog(LogLevel::Normal,
              QString("Payment %1 has status %2. Process deny.")
                  .arg(aPayment)
                  .arg(payment->getStatus()));
        return false;
    }

    if (!aOnline) {
        toLog(LogLevel::Normal, QString("Payment %1. Offline mode.").arg(aPayment));

        payment->setPriority(PPSDK::IPayment::Offline);
        payment->setStatus(PPSDK::EPaymentStatus::ReadyForCheck);
        auto stamp = QDateTime::currentDateTime();
        payment->setNextTryDate(stamp);
        payment->setCompleteDate(stamp);

        savePayment(payment.get());
    } else {
        toLog(LogLevel::Normal, QString("Payment %1. Online mode.").arg(aPayment));

        processPaymentStep(aPayment, SDK::PaymentProcessor::EPaymentStep::Pay, false);
    }

    return true;
}

//---------------------------------------------------------------------------
bool PaymentService::cancelPayment(qint64 aPayment) {
    bool result = false;

    std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

    if (payment) {
        result = payment->cancel() && savePayment(payment.get());
    }

    return result;
}

//---------------------------------------------------------------------------
bool PaymentService::stopPayment(qint64 aPayment, int aError, const QString &aErrorMessage) {
    bool result = false;

    std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

    if (payment) {
        toLog(LogLevel::Normal,
              QString("Payment %1. Stopped because of '%2'. Error code:%3.")
                  .arg(aPayment)
                  .arg(aErrorMessage)
                  .arg(aError));

        payment->setStatus(PPSDK::EPaymentStatus::BadPayment);
        payment->setParameter(
            PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ServerError, aError, true));
        payment->setParameter(PPSDK::IPayment::SParameter(
            PPSDK::CPayment::Parameters::ErrorMessage, aErrorMessage, true));
        result = savePayment(payment.get());
    }

    return result;
}

//---------------------------------------------------------------------------
bool PaymentService::removePayment(qint64 aPayment) {
    bool result = false;

    std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

    if (payment) {
        result = payment->remove() && savePayment(payment.get());
    }

    return result;
}
//---------------------------------------------------------------------------
bool PaymentService::canProcessPaymentOffline(qint64 aPayment) {
    std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

    if (payment) {
        return payment->canProcessOffline();
    }

    return false;
}

//---------------------------------------------------------------------------
void PaymentService::hangupProcessing() {
    QMetaObject::invokeMethod(this, "onProcessPayments", Qt::QueuedConnection);
}

//---------------------------------------------------------------------------
void PaymentService::doUpdatePaymentFields(
    qint64 aID,
    const std::shared_ptr<PPSDK::IPayment> &aPayment,
    const QList<SDK::PaymentProcessor::IPayment::SParameter> &aFields,
    bool aForce) {
    if (!aPayment) {
        toLog(LogLevel::Error, QString("Payment %1. Failed to update parameters.").arg(aID));
        return;
    }

    foreach (const PPSDK::IPayment::SParameter &parameter, aFields) {
        toLog(LogLevel::Normal,
              QString("Payment %1. %2pdating parameter: name '%3', value '%4'.")
                  .arg(aID)
                  .arg(aForce ? "Force u" : "U")
                  .arg(parameter.name)
                  .arg(parameter.crypted ? "** CRYPTED **" : parameter.value.toString()));

        if ((parameter.name == PPSDK::CPayment::Parameters::Amount) ||
            (parameter.name == PPSDK::CPayment::Parameters::AmountAll) ||
            (parameter.name == PPSDK::CPayment::Parameters::Change) ||
            (parameter.name == PPSDK::CPayment::Parameters::Fee) ||
            (parameter.name == PPSDK::CPayment::Parameters::ID)) {
            toLog(LogLevel::Error,
                  QString("Payment %1. Cannot update money related field manually.").arg(aID));

            continue;
        }

        aPayment->setParameter(parameter);
    }

    if (!savePayment(aPayment.get())) {
        toLog(LogLevel::Error, QString("Payment %1. Failed to save updated payment.").arg(aID));
    }
}

//---------------------------------------------------------------------------
void PaymentService::onAmountUpdated(qint64 aPayment, double /*aTotalAmount*/, double aAmount) {
    std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

    if (payment) {
        // Обновляем информацию о внесённых средствах, платёж должен пересчитать суммы, включая
        // сдачу.
        double amountAll =
            payment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble() +
            payment->getParameter(PPSDK::CPayment::Parameters::Change).value.toDouble() + aAmount;

        payment->setParameter(PPSDK::IPayment::SParameter(
            PPSDK::CPayment::Parameters::AmountAll, QString::number(amountAll, 'f', 2), true));

        if (!savePayment(payment.get())) {
            QString msg = QString("Payment [%1] error write to database; AMOUNT_ALL=%2;")
                              .arg(payment->getInitialSession())
                              .arg(amountAll, 0, 'f', 2);

            m_Application->getCore()->getTerminalService()->sendFeedback(CServices::PaymentService,
                                                                         msg);
        }

        // Получаем изменение сдачи.
        double change = payment->getParameter(PPSDK::CPayment::Parameters::Change).value.toDouble();

        // Если сдача не 0, сохраняем.
        if (!qFuzzyIsNull(change)) {
            setChangeAmount(change, payment);
        }

        emit amountUpdated(aPayment);
    }
}

//---------------------------------------------------------------------------
void PaymentService::onAmountDispensed(double aAmount) {
    if (qFuzzyIsNull(aAmount)) {
        toLog(LogLevel::Warning, QString("Dispensed zero sum %1.").arg(aAmount, 0, 'f', 2));

        return;
    }

    if (m_ChangePayment) {
        double amountAll =
            m_ChangePayment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble();

        if (amountAll >= aAmount) {
            setChangeAmount(amountAll - aAmount, m_ActivePayment);

            toLog(LogLevel::Normal,
                  QString("Dispensed %1. Change %2")
                      .arg(aAmount, 0, 'f', 2)
                      .arg(amountAll - aAmount, 0, 'f', 2));
        } else {
            // Выдали денег больше чем есть в сдаче
            toLog(LogLevel::Fatal,
                  QString("Dispensed %1, but change payment contain only %2.")
                      .arg(aAmount, 0, 'f', 2)
                      .arg(amountAll, 0, 'f', 2));

            setChangeAmount(0, m_ActivePayment);
        }
    } else {
        // Ругаемся, т.к. выдали денег, а сдачи реально нет.
        toLog(LogLevel::Error,
              QString("Dispensed %1, but change payment is NULL.").arg(aAmount, 0, 'f', 2));
    }
}

//---------------------------------------------------------------------------
void PaymentService::onPaymentThreadStarted() {
    m_PaymentTimer.start(CPaymentService::ProcessOfflineTimeout);
}

//---------------------------------------------------------------------------
bool PaymentService::processPaymentInternal(const std::shared_ptr<PPSDK::IPayment> &aPayment) {
    // Пропускаем платежи с неизрасходованной сдачей.
    if (m_ChangePayment && (m_ChangePayment->getID() == aPayment->getID())) {
        m_DBUtils->suspendPayment(aPayment->getID(), 15);

        return false;
    }

    // Пропускаем платёж, если он активен.
    if (isPaymentActive(aPayment) && (aPayment->getPriority() == PPSDK::IPayment::Online)) {
        m_DBUtils->suspendPayment(aPayment->getID(), 15);

        return false;
    }

    // Платеж можно удалить только в том случае, если в нем нет денег и купюр/монет
    if (aPayment->isNull() && getPaymentNotes(aPayment->getID()).empty()) {
        toLog(LogLevel::Normal, QString("Payment %1. Is null.").arg(aPayment->getID()));

        m_DBUtils->removePayment(aPayment->getID());

        return false;
    }

    // Запоминаем id платежа, находящегося в обработке.
    {
        QMutexLocker lock(&m_OfflinePaymentLock);
        m_OfflinePaymentID = aPayment->getID();
        m_OfflinePayment = aPayment;
    }

    // Проверка на неиспользованный остаток
    if ((aPayment->getStatus() == PPSDK::EPaymentStatus::Init) &&
        !aPayment->getParameter(CPaymentService::ChangePaymentParam).isNull()) {
        aPayment->setStatus(PPSDK::EPaymentStatus::LostChange);
    }

    if (aPayment->canProcessOffline()) {
        aPayment->setPriority(PPSDK::IPayment::Offline);
        aPayment->process();
    } else {
        toLog(LogLevel::Normal,
              QString("Payment %1. Online payment can't process with offline mode.")
                  .arg(aPayment->getID()));

        if (aPayment->getStatus() == PPSDK::EPaymentStatus::ProcessError) {
            aPayment->setStatus(PPSDK::EPaymentStatus::BadPayment);
        } else if (aPayment->getCreationDate().msecsTo(QDateTime::currentDateTime()) >
                   CPaymentService::OnlinePaymentOvertime) {
            aPayment->setStatus(PPSDK::EPaymentStatus::BadPayment);
        } else {
            toLog(LogLevel::Warning,
                  QString("Suspending online payment %1.").arg(aPayment->getID()));

            aPayment->setNextTryDate(QDateTime::currentDateTime().addSecs(5 * 60)); // 5min
        }
    }

    // Отпускаем платеж.
    {
        QMutexLocker lock(&m_OfflinePaymentLock);

        m_OfflinePaymentID = -1;
        m_OfflinePayment.reset();

        // Сохраняем платёж внутри защищенного блока для избежания записи параметров offline платежа
        savePayment(aPayment.get());
    }

    /// сообщаем об окончании процесса обработки платежа
    emit stepCompleted(aPayment->getID(),
                       SDK::PaymentProcessor::EPaymentStep::Pay,
                       aPayment->getStatus() != PPSDK::EPaymentStatus::Completed);

    return (aPayment->getStatus() == PPSDK::EPaymentStatus::Completed);
}

//---------------------------------------------------------------------------
void PaymentService::onProcessPayments() {
    // Выгружаем старые платежи раз в день.
    if (m_LastBackupDate.addDays(1) <= QDateTime::currentDateTime()) {
        m_DBUtils->backupOldPayments();

        m_LastBackupDate = QDateTime::currentDateTime();
    }

    // Блокируем offline проведение платежей до установления связи
    if (!m_Application->getCore()->getNetworkService()->isConnected(true)) {
        toLog(LogLevel::Warning, "Waiting network connection for payment processing.");

        m_PaymentTimer.start(CPaymentService::CheckNetworkConnectionTimeout);

        return;
    }

    QList<qint64> payments = m_DBUtils->getPaymentQueue();

    if (!payments.empty() != 0) {
        qint64 id = payments.takeFirst();

        if (m_Enabled) {
            std::shared_ptr<PPSDK::IPayment> payment(getPayment(id));

            // Если платеж не прогрузился, останавливаем его обработку на 15 минут.
            if (!payment) {
                toLog(LogLevel::Warning, QString("Suspending bad payment %1.").arg(id));

                m_DBUtils->suspendPayment(id, 15);
            } else {
                processPaymentInternal(payment);
            }
        }
    }

    QMutexLocker lock(&m_CommandMutex);

    // Обрабатываем очередь команд.
    foreach (auto &command, m_Commands) {
        emit paymentCommandComplete(command.first, command.second(this));
    }

    m_Commands.clear();

    m_PaymentTimer.start(CPaymentService::ProcessOfflineTimeout);
}

//---------------------------------------------------------------------------
int PaymentService::registerForcePaymentCommand(const QString &aInitialSession,
                                                const QVariantMap &aParameters) {
    QMutexLocker lock(&m_CommandMutex);

    auto command = [aInitialSession,
                    aParameters](PaymentService *aService) -> EPaymentCommandResult::Enum {
        if (!aService->m_Enabled) {
            return EPaymentCommandResult::Error;
        }

        auto payment = aService->getPayment(aInitialSession);

        if (!payment) {
            return EPaymentCommandResult::NotFound;
        }

        foreach (const QString &parameter, aParameters.keys()) {
            // берем значения атрибутов crypted и external у предыдущего значения параметра
            auto oldParameter = payment->getParameter(parameter);

            payment->setParameter(PPSDK::IPayment::SParameter(parameter,
                                                              aParameters.value(parameter),
                                                              true,
                                                              oldParameter.crypted,
                                                              oldParameter.external));
        }

        payment->setParameter(
            PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::NumberOfTries, 1, true));
        payment->setParameter(PPSDK::IPayment::SParameter(
            PPSDK::CPayment::Parameters::NextTryDate, QDateTime::currentDateTime(), true));
        payment->setStatus(PPSDK::EPaymentStatus::ProcessError);

        return aService->processPaymentInternal(payment) ? EPaymentCommandResult::OK
                                                         : EPaymentCommandResult::Error;
    };

    m_Commands << qMakePair(m_CommandIndex++,
                            std::function<EPaymentCommandResult::Enum(PaymentService *)>(command));

    auto parametersToString = [aParameters]() -> QString {
        QStringList list;

        foreach (const QString &parameter, aParameters.keys()) {
            list << QString("%1=%2").arg(parameter).arg(aParameters.value(parameter).toString());
        }

        return list.join(";");
    };

    toLog(LogLevel::Normal,
          QString("Register command 'process payment %1'. Internal command id: %2. Parameters: %3")
              .arg(aInitialSession)
              .arg(m_CommandIndex)
              .arg(parametersToString()));

    return m_Commands.last().first;
}

//---------------------------------------------------------------------------
int PaymentService::registerRemovePaymentCommand(const QString &aInitialSession) {
    QMutexLocker lock(&m_CommandMutex);

    auto command = [aInitialSession](PaymentService *aService) -> EPaymentCommandResult::Enum {
        qint64 id = aService->m_DBUtils->getPaymentByInitialSession(aInitialSession);

        if (id < 0) {
            return EPaymentCommandResult::NotFound;
        }

        return aService->removePayment(id) ? EPaymentCommandResult::OK
                                           : EPaymentCommandResult::Error;
    };

    m_Commands << qMakePair(m_CommandIndex++,
                            std::function<EPaymentCommandResult::Enum(PaymentService *)>(command));

    toLog(LogLevel::Normal,
          QString("Register command 'remove payment %1'. Internal command id: %2.")
              .arg(aInitialSession)
              .arg(m_CommandIndex));

    return m_Commands.last().first;
}

//---------------------------------------------------------------------------
PPSDK::SBalance PaymentService::getBalance() {
    return m_DBUtils->getBalance();
}

//---------------------------------------------------------------------------
PPSDK::EncashmentResult::Enum PaymentService::perform_Encashment(const QVariantMap &aParameters,
                                                                 PPSDK::SEncashment &aEncashment) {
    PPSDK::SBalance balance = m_DBUtils->getBalance();

    {
        QMutexLocker lock(&m_CommandMutex);

        // не можем выполнить инкассацию в случае несохраненных в БД данных
        if (!balance.notPrintedPayments.intersect(m_PaymentHaveUnsavedParameters).isEmpty()) {
            return PPSDK::EncashmentResult::TryLater;
        }
    }

    aEncashment = m_DBUtils->perform_Encashment(aParameters);

    return aEncashment.isValid() ? PPSDK::EncashmentResult::OK : PPSDK::EncashmentResult::Error;
}

//---------------------------------------------------------------------------
PPSDK::SEncashment PaymentService::getLastEncashment() {
    auto encashments = m_DBUtils->getLastEncashments(1);

    return encashments.isEmpty() ? PPSDK::SEncashment() : encashments.at(0);
}

//---------------------------------------------------------------------------
QList<PPSDK::SEncashment> PaymentService::getEncashmentList(int aDepth) {
    return m_DBUtils->getLastEncashments(aDepth);
}

//---------------------------------------------------------------------------
double PaymentService::getChangeAmount() {
    if (m_ChangePayment) {
        return m_ChangePayment->getParameter(PPSDK::CPayment::Parameters::AmountAll)
            .value.toDouble();
    }

    return 0.0;
}

//---------------------------------------------------------------------------
void PaymentService::moveChangeToPayment(qint64 aPayment) {
    if (m_ChangePayment) {
        double change = getChangeAmount();

        if (!qFuzzyIsNull(change)) {
            std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));
            if (payment) {
                toLog(
                    LogLevel::Normal,
                    QString("Payment %1. Using change: %2.").arg(aPayment).arg(change, 0, 'f', 2));

                double amountAll =
                    payment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble() +
                    change;

                payment->setParameter(
                    PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll,
                                                QString::number(amountAll, 'f', 2),
                                                true,
                                                false));

                m_ChangePayment->setParameter(PPSDK::IPayment::SParameter(
                    PPSDK::CPayment::Parameters::AmountAll,
                    payment->getParameter(PPSDK::CPayment::Parameters::Change).value,
                    true));

                // Использовал мошенническую сдачу - сам мошенник
                auto originalPayment = getPayment(getChangeSessionRef());
                auto cheatedParameter = (originalPayment ? originalPayment : m_ChangePayment)
                                            ->getParameter(PPSDK::CPayment::Parameters::Cheated);

                if (!cheatedParameter.isNull()) {
                    cheatedParameter.updated = true;
                    payment->setParameter(cheatedParameter);
                }

                savePayment(m_ChangePayment.get());
                savePayment(payment.get());

                double newChange =
                    payment->getParameter(PPSDK::CPayment::Parameters::Change).value.toDouble();
                toLog(LogLevel::Normal,
                      QString("Payment %1. Updating change: %2.")
                          .arg(aPayment)
                          .arg(newChange, 0, 'f', 2));

                emit amountUpdated(aPayment);
                emit changeUpdated(newChange);
            }
        }
    }
}

//---------------------------------------------------------------------------
void PaymentService::movePaymentToChange(qint64 aPayment) {
    std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));
    if (payment) {
        double change =
            getChangeAmount() +
            payment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble();

        if (!qFuzzyIsNull(change)) {
            if (setChangeAmount(change, payment)) {
                toLog(LogLevel::Normal,
                      QString("Payment %1. Move amount to change: %2.")
                          .arg(aPayment)
                          .arg(change, 0, 'f', 2));

                payment->setParameter(
                    PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll,
                                                QString::number(0.00, 'f', 2),
                                                true,
                                                false));

                savePayment(payment.get());
            }
        }
    }
}

//---------------------------------------------------------------------------
void PaymentService::resetChange() {
    // Если есть активный платёж со сдачей, отправляем его на мониторинг и создаём новый.
    if (m_ChangePayment) {
        bool remove = qFuzzyIsNull(getChangeAmount());
        qint64 changePaymentID = m_ChangePayment->getID();

        auto originalPayment = getPayment(getChangeSessionRef());
        auto cheatedParameter = (originalPayment ? originalPayment : m_ChangePayment)
                                    ->getParameter(PPSDK::CPayment::Parameters::Cheated);

        toLog(LogLevel::Normal,
              QString("Payment %1. Reset %3 change amount: %2.")
                  .arg(changePaymentID)
                  .arg(getChangeAmount(), 0, 'f', 2)
                  .arg(cheatedParameter.isNull() ? "" : "CHEATED"));

        // Если есть платёж "сдача" с нулевой суммой, то не оставляем его в БД "как есть", а
        // помечаем как удалённый.
        m_ChangePayment->setStatus(remove ? PPSDK::EPaymentStatus::Deleted
                                          : PPSDK::EPaymentStatus::LostChange);

        savePayment(m_ChangePayment.get());

        m_ChangePayment.reset();

        if (remove) {
            m_DBUtils->removePayment(changePaymentID);
        }
    }

    emit changeUpdated(0.0);
}

//---------------------------------------------------------------------------
QString PaymentService::getChangeSessionRef() {
    if (m_ChangePayment) {
        return m_ChangePayment->getParameter(PPSDK::CPayment::Parameters::OriginalPayment)
            .value.toString()
            .split(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter)
            .takeFirst();
    }

    return {};
}

//---------------------------------------------------------------------------
bool PaymentService::setChangeAmount(double aChange,
                                     const std::shared_ptr<PPSDK::IPayment> &aPaymentSource) {
    if (!m_ChangePayment) {
        if (m_FactoryByType.contains(CPaymentService::ChangePaymentType)) {
            m_ChangePayment = std::shared_ptr<PPSDK::IPayment>(
                m_FactoryByType[CPaymentService::ChangePaymentType]->createPayment(
                    CPaymentService::ChangePaymentType),
                [capture0 = m_FactoryByType[CPaymentService::ChangePaymentType]](auto &&PH1) {
                    capture0->releasePayment(std::forward<decltype(PH1)>(PH1));
                });

            if (m_ChangePayment) {
                QList<PPSDK::IPayment::SParameter> parameters;

                parameters
                    << PPSDK::IPayment::SParameter(
                           PPSDK::CPayment::Parameters::ID, m_DBUtils->createDummyPayment(), true)
                    << PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::CreationDate,
                                                   QDateTime::currentDateTime(),
                                                   true)
                    << PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Type,
                                                   CPaymentService::ChangePaymentType,
                                                   true)
                    << PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Provider, -1, true)
                    << PPSDK::IPayment::SParameter(
                           PPSDK::CPayment::Parameters::Status, PPSDK::EPaymentStatus::Init, true)
                    << PPSDK::IPayment::SParameter(
                           PPSDK::CPayment::Parameters::Priority, PPSDK::IPayment::Offline, true)
                    << PPSDK::IPayment::SParameter(CPaymentService::ChangePaymentParam, 1, true);

                if (!m_ChangePayment->restore(parameters)) {
                    toLog(LogLevel::Error, "Failed to register storage for the change.");

                    m_ChangePayment.reset();

                    return false;
                }
            } else {
                toLog(LogLevel::Error, "Failed to create storage for the change.");

                return false;
            }
        } else {
            toLog(LogLevel::Error,
                  "Failed to create storage for the change. Required plugin is missing.");

            return false;
        }
    }

    // В сдачу засовываем все данные из родительского платежа
    if (aPaymentSource) {
        QMutexLocker lock(&m_PaymentLock);

        QStringList paramValues(
            aPaymentSource->getParameter(PPSDK::CPayment::Parameters::InitialSession)
                .value.toString());

        foreach (
            auto param,
            aPaymentSource->getParameter(PPSDK::CPayment::Parameters::ProviderFields)
                .value.toString()
                .split(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter, Qt::SkipEmptyParts)) {
            paramValues << aPaymentSource->getParameter(param).value.toString();
        }

        // Если платеж с подозрением на мошенничество, сдача тоже мошенническая
        auto cheatedParameter = aPaymentSource->getParameter(PPSDK::CPayment::Parameters::Cheated);
        if (!cheatedParameter.isNull()) {
            cheatedParameter.updated = true;
            m_ChangePayment->setParameter(cheatedParameter);
        }

        m_ChangePayment->setParameter(PPSDK::IPayment::SParameter(
            PPSDK::CPayment::Parameters::OriginalPayment,
            paramValues.join(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter),
            true));

        m_ChangePayment->setParameter(
            PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ProviderFields,
                                        PPSDK::CPayment::Parameters::OriginalPayment,
                                        true));
    }

    m_ChangePayment->setParameter(PPSDK::IPayment::SParameter(
        PPSDK::CPayment::Parameters::AmountAll, QString::number(aChange, 'f', 2), true));

    toLog(LogLevel::Normal,
          QString("Payment %1. Change: %2.").arg(getActivePayment()).arg(aChange, 0, 'f', 2));

    emit changeUpdated(aChange);

    return savePayment(m_ChangePayment.get());
}

//---------------------------------------------------------------------------
QList<PPSDK::SNote> PaymentService::getPaymentNotes(qint64 aID) const {
    return m_DBUtils->getPaymentNotes(aID);
}

//---------------------------------------------------------------------------
QList<qint64> PaymentService::getPayments(const QSet<PPSDK::EPaymentStatus::Enum> &aStates) {
    return m_DBUtils->getPayments(aStates);
}

//---------------------------------------------------------------------------
QList<qint64> PaymentService::findPayments(const QDate &aDate, const QString &aPhoneNumber) {
    return m_DBUtils->findPayments(aDate, aPhoneNumber);
}

//------------------------------------------------------------------------------
QMap<qint64, quint32> PaymentService::getStatistic() const {
    return m_DBUtils->getStatistic();
}

//------------------------------------------------------------------------------
