/* @file Прокси класс для работы с платежами в скриптах. */

// stl

#include <QtCore/QDateTime>
#include <QtCore/QScopedPointer>

#include <algorithm>

#pragma push_macro("foreach")
#undef foreach
#include <boost/noncopyable.hpp>
#pragma pop_macro("foreach")
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IDatabaseService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Humo/ErrorCodes.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Security.h>
#include <SDK/PaymentProcessor/Scripting/PaymentService.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <DatabaseProxy/IDatabaseQuery.h>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <utility>

namespace PPSDK = SDK::PaymentProcessor;

using TPtreeOperators = boost::property_tree::basic_ptree<std::string, std::string>;

//---------------------------------------------------------------------------
namespace SDK {
namespace PaymentProcessor {
namespace EncashmentParameter {
const char StackerID[] = "STACKER_ID";
} // namespace EncashmentParameter
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
namespace CMultistage {
const QString Type = "multistage";

const QString Step = "MULTISTAGE_STEP";            /// текущий шаг
const QString StepFields = "MULTISTAGE_FIELDS_%1"; /// список полей для конкретного шага
const QString History = "MULTISTAGE_HISTORY";      /// история шагов
} // namespace CMultistage

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
ScriptArray *ProviderField::getEnum_Items() {
    auto *list = new ScriptArray(this);

    foreach (const SProviderField::SEnum_Item &item, m_Field.enum_Items) {
        list->append(new Enum_Item(item, list));
    }

    return list;
}

//------------------------------------------------------------------------------
Provider::Provider(SProvider aProvider, QObject *aParent)
    : QObject(aParent), m_Provider(std::move(aProvider)) {
    QObjectList fields;

    foreach (const SProviderField &field, m_Provider.fields) {
        fields << new ProviderField(field, this);
    }

    m_Fields["0"] = fields;
}

//------------------------------------------------------------------------------
bool Provider::isCheckStepSettingsOK() const {
    QStringList limits;
    limits << m_Provider.limits.min << m_Provider.limits.max << m_Provider.limits.externalMin
           << m_Provider.limits.externalMax;

    QRegularExpression rx("\\{(.+)\\}");

    QStringList limits2;
    foreach (QString l, limits) {
        QRegularExpressionMatchIterator i = rx.globalMatch(l);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            limits2 << match.captured(1);
        }
    }

    QStringList responseFields;

    foreach (SProvider::SProcessingTraits::SRequest::SField f,
             m_Provider.processor.requests.value("CHECK").responseFields) {
        responseFields << f.name;
    }

    QSet<QString> limitsSet(limits2.begin(), limits2.end());
    QSet<QString> responseFieldsSet(responseFields.begin(), responseFields.end());

    if (!limitsSet.intersect(responseFieldsSet).isEmpty()) {
        return m_Provider.processor.showAddInfo || !m_Provider.processor.skipCheck;
    }
    return true;
}

//------------------------------------------------------------------------------
QString Provider::applySecurityFilter(const QString aId,
                                      const QString &aValueRaw,
                                      const QString &aValueDisplay) const {
    PPSDK::SecurityFilter filter(m_Provider, PPSDK::SProviderField::SecuritySubsystem::Display);

    QString masked = aValueRaw;

    if (filter.haveFilter(aId)) {
        masked = filter.apply(aId, aValueRaw);
    } else {
        masked = aValueDisplay;
    }

    return masked;
}

//------------------------------------------------------------------------------
QString Provider::xmlFields2Json(const QString &aXmlFields) {
    TPtreeOperators ptFields;
    const TPtreeOperators emptyTree;
    TProviderFields fields;

    QByteArray buffer = aXmlFields.toUtf8();
    std::stringstream stream(std::string(buffer.data(), buffer.size()));

    try {
        boost::property_tree::read_xml(stream, ptFields);
    } catch (boost::property_tree::xml_parser_error &e) {
        qDebug() << QString("xmlFields2Json: XML parser error: %1.")
                        .arg(QString::fromStdString(e.message()));

        return {};
    }

    std::function<void(SProviderField::TEnum_Items &, const TPtreeOperators &)>
        loadProviderEnumItems;
    loadProviderEnumItems = [&](SProviderField::TEnum_Items &aItemList,
                                const TPtreeOperators &aTree) {
        auto searchBounds = aTree.equal_range("item");

        for (auto itemIt = searchBounds.first; itemIt != searchBounds.second; ++itemIt) {
            SProviderField::SEnum_Item item;

            auto attr = itemIt->second.get_child("<xmlattr>");

            item.title = attr.get<QString>("name");
            item.value = attr.get<QString>("value", QString());
            item.id = attr.get<QString>("id", QString());
            item.sort = attr.get<int>("sort", 65535);

            loadProviderEnumItems(item.subItems, itemIt->second);

            aItemList << item;
        }

        std::stable_sort(aItemList.begin(),
                         aItemList.end(),
                         [](const SProviderField::SEnum_Item &a,
                            const SProviderField::SEnum_Item &b) { return a.sort < b.sort; });
    };

    const auto &addFieldsTree = ptFields.get_child("add_fields", emptyTree);
    BOOST_FOREACH (const auto &fieldIt, addFieldsTree) {
        try {
            SProviderField field;

            if (fieldIt.first == "field") {
                auto attr = fieldIt.second.get_child("<xmlattr>");

                field.id = attr.get<QString>("id");
                field.type = attr.get<QString>("type");
                field.keyboardType = attr.get<QString>("keyboard_type", QString());
                field.isRequired = attr.get<bool>("required", true);
                field.sort = attr.get<int>("sort", 65535);
                field.minSize = attr.get<int>("min_size", -1);
                field.maxSize = attr.get<int>("max_size", -1);
                field.language = attr.get<QString>("lang", QString());
                field.letterCase = attr.get<QString>("case", QString());
                field.behavior = attr.get<QString>("behavior", QString());
                field.defaultValue = attr.get<QString>("default_value", QString());

                field.title = fieldIt.second.get<QString>("name");
                field.comment = fieldIt.second.get<QString>("comment", QString());
                field.extendedComment = fieldIt.second.get<QString>("extended_comment", QString());
                field.mask = fieldIt.second.get<QString>("mask", QString());
                field.isPassword = fieldIt.second.get<bool>("mask.<xmlattr>.password", false);
                field.format = fieldIt.second.get<QString>("format", QString());

                field.url = fieldIt.second.get<QString>("url", QString());
                field.html = fieldIt.second.get<QString>("html", QString());
                field.backButton = fieldIt.second.get<QString>("back_button", QString());
                field.forwardButton = fieldIt.second.get<QString>("forward_button", QString());

                field.dependency = fieldIt.second.get<QString>("dependency", QString());

                loadProviderEnumItems(field.enum_Items,
                                      fieldIt.second.get_child("enum", emptyTree));

                fields << field;
            }
        } catch (std::runtime_error &error) {
            qDebug() << error.what();
        }
    }

    return SProvider::fields2Json(fields);
}

//------------------------------------------------------------------------------
QVariant Provider::getFields() {
    auto *paymentService = qobject_cast<PaymentService *>(parent());

    if (!paymentService) {
        return QVariant::fromValue(m_Fields["0"]);
    }

    QString currentStep = paymentService->currentStep();

    if (m_Provider.processor.type == CMultistage::Type) {
        TProviderFields fields;
        if (paymentService->loadFieldsForStep(fields)) {
            QObjectList resultList;

            foreach (const SProviderField &field, fields) {
                resultList << new ProviderField(field, this);
            }

            m_Fields[currentStep] = resultList;
        }
    }

    return QVariant::fromValue(m_Fields[currentStep]);
}

//------------------------------------------------------------------------------
PaymentService::PaymentService(ICore *aCore)
    : m_Core(aCore), m_PaymentService(m_Core->getPaymentService()),
      m_ProviderWithExternalLimits(-1), m_ForcePayOffline(false), m_Directory(nullptr),
      m_DealerSettings(nullptr) {
    connect(m_PaymentService,
            SIGNAL(stepCompleted(qint64, int, bool)),
            SLOT(onStepCompleted(qint64, int, bool)));
    connect(m_PaymentService, SIGNAL(amountUpdated(qint64)), SIGNAL(amountUpdated(qint64)));
    connect(m_PaymentService, SIGNAL(changeUpdated(double)), SIGNAL(changeUpdated(double)));

    // Используем reinterpret_cast вместо dynamic_cast для обхода проблемы с множественным
    // наследованием
    void *dirPtr =
        reinterpret_cast<void *>(aCore->getSettingsService()->getAdapter(CAdapterNames::Directory));
    m_Directory = reinterpret_cast<PPSDK::Directory *>(dirPtr);

    void *dealerPtr = reinterpret_cast<void *>(
        aCore->getSettingsService()->getAdapter(CAdapterNames::DealerAdapter));
    m_DealerSettings = reinterpret_cast<PPSDK::DealerSettings *>(dealerPtr);

    void *terminalPtr = reinterpret_cast<void *>(
        aCore->getSettingsService()->getAdapter(CAdapterNames::TerminalAdapter));
    auto *terminalSettings = reinterpret_cast<PPSDK::TerminalSettings *>(terminalPtr);

    if (terminalSettings) {
        m_CommonSettings = terminalSettings->getCommonSettings();
    } else {
        qCritical() << "PaymentService::PaymentService - TerminalSettings adapter is NULL!";
    }
}

//------------------------------------------------------------------------------
qint64 PaymentService::create(qint64 aProvider) {
    return m_PaymentService->createPayment(aProvider);
}

//------------------------------------------------------------------------------
qint64 PaymentService::getActivePaymentID() {
    return m_PaymentService->getActivePayment();
}

//------------------------------------------------------------------------------
qint64 PaymentService::getLastPaymentID() {
    auto payments = m_PaymentService->getBalance().payments;

    while (!payments.isEmpty()) {
        qint64 payment = payments.takeLast();

        // Проверяем что платеж это не сдача
        if (m_PaymentService->getPaymentField(payment, PPSDK::CPayment::Parameters::Provider)
                .value.toLongLong() > 0) {
            return payment;
        }
    }

    return -1;
}

//------------------------------------------------------------------------------
QVariant PaymentService::getParameter(const QString &aName) {
    return m_PaymentService->getPaymentField(getActivePaymentID(), aName).value;
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::getParameters() {
    return getParameters(getActivePaymentID());
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::calculateCommission(const QVariantMap &aParameters) {
    QList<PPSDK::IPayment::SParameter> input;

    foreach (auto p, aParameters.keys()) {
        input << PPSDK::IPayment::SParameter(p, aParameters.value(p));
    }

    if (m_PaymentService) {
        QVariantMap result;

        foreach (auto p, m_PaymentService->calculateCommission(input)) {
            result.insert(p.name, p.value);
        }

        return result;
    }

    return {};
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::calculateLimits(const QString &aAmount, bool aFixedAmount) {
    // TODO Код из paymentbase::calculateLimits

    PPSDK::DealerSettings *dealerSettings = dynamic_cast<PPSDK::DealerSettings *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));

    double minAmount = aAmount.toDouble();
    double maxAmount = aAmount.toDouble();

    const qint64 providerID = getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong();
    const auto provider = m_PaymentService->getProvider(providerID);
    double systemMax = provider.limits.system.toDouble();
    systemMax = maxAmount > systemMax ? systemMax : maxAmount;

    if (!aFixedAmount) {
        // Корректируем максимальный платеж в зависимости от системного лимита
        if (qFuzzyIsNull(maxAmount) || qFuzzyIsNull(systemMax)) {
            // Если какой-либо из лимитов не задан, то берем тот, который задан.
            maxAmount = qMax(systemMax, maxAmount);
        } else {
            // Иначе берем нижнюю границу.
            maxAmount = qMin(systemMax, maxAmount);
        }
    }

    // Проверяем значение системного лимита
    if (qFuzzyIsNull(systemMax)) {
        systemMax = maxAmount;
    }

    // Если провайдер требует округления - применим округление к пограничным значениям
    if (provider.processor.rounding) {
        maxAmount = qCeil(maxAmount);
        if (!aFixedAmount) {
            minAmount = qCeil(minAmount);
        }
    }

    // Формируем массив заполненных полей для подсчёта комиссий.
    QVariantMap credentials;

    foreach (const PPSDK::SProviderField &field, provider.fields) {
        credentials.insert(field.id, getParameter(field.id).toString());
    }

    PPSDK::Commission com(dealerSettings->getCommission(providerID, credentials, maxAmount));

    auto calcAmountAll = [&com, &provider](double aAmount) -> double {
        double amountAll = 0.0;

        if (com.getType() == PPSDK::Commission::Percent) {
            double amountAllLimit1 = 0.0;

            if (provider.processor.feeType == PPSDK::SProvider::FeeByAmount ||
                com.getBase() == PPSDK::Commission::Amount) {
                amountAllLimit1 = aAmount * (1 + 0.01 * com.getValue());
            } else {
                amountAllLimit1 = aAmount / (1 - 0.01 * com.getValue());
            }

            amountAll = amountAllLimit1;

            if (!qFuzzyIsNull(com.getMinCharge())) {
                double amountAllLimit2 = aAmount + com.getMinCharge();
                amountAll = amountAllLimit1 > amountAllLimit2 ? amountAllLimit1 : amountAllLimit2;
            }

            if (!qFuzzyIsNull(com.getMaxCharge())) {
                double amountAllLimit3 = aAmount + com.getMaxCharge();
                amountAll = amountAll > amountAllLimit3 ? amountAllLimit3 : amountAll;
            }
        } else {
            amountAll = aAmount + com.getValue();
        }

        return amountAll;
    };

    auto calcAmountAllByAmountAll = [&com, &provider](double &aAmount) -> double {
        double amountAll = 0.0;

        if (com.getType() == PPSDK::Commission::Percent && !qFuzzyIsNull(com.getValue())) {
            double amountAllLimit1 = 0.0;

            if (provider.processor.feeType == PPSDK::SProvider::FeeByAmount ||
                com.getBase() == PPSDK::Commission::Amount) {
                amountAllLimit1 = aAmount / (1 + 0.01 * com.getValue());
            } else {
                amountAllLimit1 = aAmount * (1 - 0.01 * com.getValue());
            }

            amountAll = amountAllLimit1;
            double fee = aAmount - amountAllLimit1;

            if (!qFuzzyIsNull(com.getMinCharge()) && fee < com.getMinCharge()) {
                double amountAllLimit2 = aAmount - com.getMinCharge();
                amountAll = amountAllLimit1 > amountAllLimit2 ? amountAllLimit1 : amountAllLimit2;
            }

            if (!qFuzzyIsNull(com.getMaxCharge()) && fee > com.getMaxCharge()) {
                double amountAllLimit3 = aAmount - com.getMaxCharge();
                amountAll = qMax(amountAllLimit3, amountAll);
            }
        } else if (com.getType() == PPSDK::Commission::Absolute && !qFuzzyIsNull(com.getValue())) {
            aAmount -= com.getValue();
            amountAll = aAmount;
        } else if (!qFuzzyIsNull(com.getMinCharge())) {
            amountAll = aAmount - com.getMinCharge();
        }

        return amountAll;
    };

    double localAmountAllLimit = calcAmountAll(maxAmount);
    double maxAmountAll = 0.0;

    if (aFixedAmount) {
        // если лимиты равны и комиссия больше системного лимита, то пропускаем эти лимиты дальше.
        maxAmountAll = localAmountAllLimit;
    } else if (localAmountAllLimit > systemMax) {
        maxAmountAll = systemMax;
        maxAmount = calcAmountAllByAmountAll(maxAmount);
    } else {
        maxAmountAll = qMax(localAmountAllLimit, maxAmount);
    }

    // Округлим результат после 2ого знака чтобы не было расхождений в копейках
    maxAmountAll = static_cast<double>(qRound64(maxAmountAll * 100)) / 100.0;

    // Если провайдер требует округления - применим округление к пограничным значениям
    if (provider.processor.rounding) {
        maxAmountAll = qCeil(maxAmountAll);
    }

    QVariantMap result;

    result.insert(PPSDK::CPayment::Parameters::MaxAmount, maxAmount);
    result.insert(PPSDK::CPayment::Parameters::MaxAmountAll, maxAmountAll);

    return result;
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::getParameters(qint64 aPaymentId) {
    QVariantMap result;

    if (m_PaymentService) {
        foreach (auto parameter, m_PaymentService->getPaymentFields(aPaymentId)) {
            result.insert(parameter.name, parameter.value);
        }
    }

    return result;
}

//------------------------------------------------------------------------------
QObject *PaymentService::getProvider() {
    return getProvider(getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong());
}

//------------------------------------------------------------------------------
QObject *PaymentService::getProvider(qint64 aID) {
    foreach (auto pp, m_ProviderProviders) {
        auto ppRef = pp.toStrongRef();
        if (!ppRef.isNull() && ppRef->getId().contains(aID)) {
            return new Provider(updateSkipCheckFlag(ppRef->getProvider(aID)), this);
        }
    }

    return new Provider(updateSkipCheckFlag(m_PaymentService->getProvider(aID)), this);
}

//------------------------------------------------------------------------------
QObject *PaymentService::getMNPProvider() {
    return new Provider(updateSkipCheckFlag(m_DealerSettings->getMNPProvider(
                            getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong(),
                            getParameter(PPSDK::CPayment::Parameters::MNPGatewayIn).toLongLong(),
                            getParameter(PPSDK::CPayment::Parameters::MNPGatewayOut).toLongLong())),
                        this);
}

//------------------------------------------------------------------------------
QObject *PaymentService::getProviderByGateway(qint64 aCID) {
    auto providers = m_DealerSettings->getProvidersByCID(aCID);

    return new Provider(
        providers.isEmpty() ? PPSDK::SProvider() : updateSkipCheckFlag(providers.at(0)), this);
}

//------------------------------------------------------------------------------
QObject *PaymentService::getProviderForNumber(qint64 aNumber) {
    auto *result = new ScriptArray(this);

    foreach (const SProvider &p,
             m_DealerSettings->getProvidersByRange(m_Directory->getRangesForNumber(aNumber),
                                                   m_Directory->getOverlappedIDs())) {
        result->append(new PPSDK::Scripting::Provider(updateSkipCheckFlag(p), this));
    }

    return result;
}

//------------------------------------------------------------------------------
void PaymentService::setExternalParameter(const QString &aName, const QVariant &aValue) {
    m_PaymentService->updatePaymentField(getActivePaymentID(),
                                         IPayment::SParameter(aName, aValue, true, false, true));
}

//------------------------------------------------------------------------------
QString PaymentService::findAliasFrom_Request(const QString &aParamName,
                                              const QString &aRequestName) {
    auto provider = m_PaymentService->getProvider(
        getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong());

    auto fields = provider.processor.requests.value(aRequestName).requestFields;
    foreach (auto field, fields) {
        if (field.name == aParamName) {
            QRegularExpression macroPattern("\\{(.+)\\}");

            QString result = field.value;

            QRegularExpressionMatch match = macroPattern.match(result);
            if (match.hasMatch()) {
                result.replace(match.capturedStart(0), match.capturedLength(0), match.captured(1));
            }

            return result;
        }
    }

    return {};
}

//------------------------------------------------------------------------------
void PaymentService::setParameter(const QString &aName,
                                  const QVariant &aValue,
                                  bool aCrypted /*= false*/) {
    m_PaymentService->updatePaymentField(getActivePaymentID(),
                                         IPayment::SParameter(aName, aValue, true, aCrypted));
}

//------------------------------------------------------------------------------
void PaymentService::setParameters(const QVariantMap &aParameters) {
    auto provider = m_PaymentService->getProvider(
        getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong());

    // функция проверки - является ли параметр значением поля, содержащим пароль
    auto keepEncrypted = [&provider](const QString &aParamName) -> bool {
        foreach (auto field, provider.fields) {
            if (aParamName.contains(field.id)) {
                return field.keepEncrypted();
            }
        }

        return false;
    };

    QList<PPSDK::IPayment::SParameter> parameters;

    for (auto it = aParameters.begin(); it != aParameters.end(); ++it) {
        parameters << PPSDK::IPayment::SParameter(
            it.key(), it.value(), true, keepEncrypted(it.key()));
    }

    m_PaymentService->updatePaymentFields(getActivePaymentID(), parameters);
}

//------------------------------------------------------------------------------
void PaymentService::updateLimits(qint64 aProviderId, double aExternalMin, double aExternalMax) {
    m_DealerSettings->setExternalLimits(aProviderId, aExternalMin, aExternalMax);

    m_ProviderWithExternalLimits = aProviderId;
}

//------------------------------------------------------------------------------
bool PaymentService::canProcessOffline() {
    return m_PaymentService->canProcessPaymentOffline(getActivePaymentID());
}

//------------------------------------------------------------------------------
void PaymentService::check() {
    processStep(EPaymentStep::DataCheck);
}

//------------------------------------------------------------------------------
void PaymentService::stepForward() {
    processStep(EPaymentStep::GetStep);
}

//------------------------------------------------------------------------------
void PaymentService::processStep(int aStep) {
    m_PaymentService->processPaymentStep(getActivePaymentID(),
                                         static_cast<EPaymentStep::Enum>(aStep));
}

//------------------------------------------------------------------------------
PaymentService::EProcessResult PaymentService::process(bool aOnline) {
    double amount = getParameter(PPSDK::CPayment::Parameters::Amount).toDouble();
    double minAmount = getParameter(PPSDK::CPayment::Parameters::MinAmount).toDouble();

    if ((amount < minAmount) && !qFuzzyCompare(amount, minAmount)) {
        cancel();

        return LowMoney;
    }

    if (!aOnline && !canProcessOffline()) {
        cancel();

        return OfflineIsNotSupported;
    }

    if (!aOnline && m_CommonSettings.blockCheatedPayment &&
        (getParameter(PPSDK::CPayment::Parameters::Cheated).toInt() != 0)) {
        stop(PPSDK::Humo::EServerError::Cheated, "Cheated");

        resetChange();

        return BadPayment;
    }

    checkStatus();

    return m_PaymentService->processPayment(getActivePaymentID(), aOnline) ? OK : BadPayment;
}

//------------------------------------------------------------------------------
bool PaymentService::stop(int aError, const QString &aErrorMessage) {
    return m_PaymentService->stopPayment(getActivePaymentID(), aError, aErrorMessage);
}

//------------------------------------------------------------------------------
bool PaymentService::cancel() {
    return m_PaymentService->cancelPayment(getActivePaymentID());
}

//------------------------------------------------------------------------------
double PaymentService::getChangeAmount() {
    return m_PaymentService->getChangeAmount();
}

//------------------------------------------------------------------------------
void PaymentService::useChange() {
    m_PaymentService->moveChangeToPayment(getActivePaymentID());
}

//------------------------------------------------------------------------------
void PaymentService::useChangeBack() {
    m_PaymentService->movePaymentToChange(getActivePaymentID());
}

//------------------------------------------------------------------------------
void PaymentService::resetChange() {
    m_PaymentService->resetChange();
}

//------------------------------------------------------------------------------
QObject *PaymentService::getPaymentNotes() {
    auto *notes = new ScriptArray(this);

    foreach (const SNote &note, m_PaymentService->getPaymentNotes(getActivePaymentID())) {
        notes->append(new Note(note, notes));
    }

    return notes;
}

//------------------------------------------------------------------------------
QObject *PaymentService::notesFrom_Balance(const PPSDK::SBalance &aBalance) {
    auto *notes = new ScriptArray(this);

    foreach (const SBalance::SAmounts &amounts, aBalance.detailedSums) {
        foreach (const SBalance::SAmounts::SAmount &amount, amounts.amounts) {
            foreach (QString serial, amount.serials.split(",")) {
                SNote note(amounts.type, amount.value.toDouble(), amounts.currency, serial);
                notes->append(new Note(note, notes));
            }
        }
    }

    return notes;
}

//------------------------------------------------------------------------------
QObject *PaymentService::getBalanceNotes() {
    return notesFrom_Balance(m_PaymentService->getBalance());
}

//------------------------------------------------------------------------------
QObject *PaymentService::getLastEncashmentNotes() {
    return notesFrom_Balance(m_PaymentService->getLastEncashment().balance);
}

//------------------------------------------------------------------------------
QString PaymentService::currentStep() {
    qint64 providerId = getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong();
    SProvider provider = m_PaymentService->getProvider(providerId);

    if (provider.processor.type == CMultistage::Type) {
        QString step = getParameter(CMultistage::Step).toString();

        return step.isEmpty() ? "0" : step;
    }

    return "0";
}

//------------------------------------------------------------------------------
void PaymentService::reset() {
    // Установить шаг на первый
    if (currentStep() != "0") {
        setParameter(CMultistage::Step, "");
        setParameter(CMultistage::History, "");
    }

    m_PaymentService->deactivatePayment();

    // Вернем платежные лимиты по умолчанию
    m_DealerSettings->setExternalLimits(m_ProviderWithExternalLimits, 0.0, 0.0);
}

//------------------------------------------------------------------------------
bool PaymentService::isFinalStep() {
    return (getParameter(CMultistage::Step).toString() == "FINAL_STEP");
}

//------------------------------------------------------------------------------
void PaymentService::setExternalCommissions(const QVariantList &aCommissions) {
    m_DealerSettings->setExternalCommissions(PPSDK::Commissions::from_Variant(aCommissions));
}

//------------------------------------------------------------------------------
void PaymentService::resetExternalCommissions() {
    m_DealerSettings->resetExternalCommissions();
}

//------------------------------------------------------------------------------
void PaymentService::stepBack() {
    if (currentStep() != "0") {
        QStringList history =
            getParameter(CMultistage::History).toString().split(";", Qt::SkipEmptyParts);
        if (!history.isEmpty()) {
            setParameter(CMultistage::Step, history.takeLast());
            setParameter(CMultistage::History, history.join(";"));
        }
    }
}

//------------------------------------------------------------------------------
bool PaymentService::loadFieldsForStep(TProviderFields &aFields) {
    QString step = currentStep();
    if (step != "0") {
        aFields =
            SProvider::json2Fields(getParameter(CMultistage::StepFields.arg(step)).toString());

        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
QStringList PaymentService::findPayments(const QDate &aDate, const QString &aPhoneNumber) {
    QStringList result;

    foreach (auto id, m_PaymentService->findPayments(aDate, aPhoneNumber)) {
        result << QString::number(id);
    }

    return result;
}

//------------------------------------------------------------------------------
bool PaymentService::convert(const QString &aTargetType) {
    return m_PaymentService->convertPayment(m_PaymentService->getActivePayment(), aTargetType);
}

//------------------------------------------------------------------------------
void PaymentService::setForcePayOffline(bool aForcePayOffline) {
    m_ForcePayOffline = aForcePayOffline;
}

//------------------------------------------------------------------------------
bool isEmptyRequiredResponseFields(const SProvider &aProvider) {
    int count = 0;

    foreach (auto f, aProvider.processor.requests["CHECK"].responseFields) {
        count = f.required ? count + 1 : count;
    }

    foreach (auto f, aProvider.processor.requests["PAY"].responseFields) {
        count = f.required ? count + 1 : count;
    }

    return (count == 0);
}

//------------------------------------------------------------------------------
PPSDK::SProvider PaymentService::updateSkipCheckFlag(SProvider aProvider) {
    if (!aProvider.processor.payOnline && m_ForcePayOffline) {
        // Изменяем параметр проверки платежа для возможности проведения его в оффлайне
        if (!m_Core->getNetworkService()->isConnected(true) && !aProvider.processor.skipCheck) {
            aProvider.processor.skipCheck = isEmptyRequiredResponseFields(aProvider);
        }
    }

    return aProvider;
}

//------------------------------------------------------------------------------
void PaymentService::onStepCompleted(qint64 aPayment, int aStep, bool aError) {
    if (aStep == PPSDK::EPaymentStep::DataCheck && aError) {
        auto provider = updateSkipCheckFlag(m_PaymentService->getProvider(
            getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong()));

        if (m_ForcePayOffline && isEmptyRequiredResponseFields(provider) &&
            !provider.processor.payOnline) {
            // проверка на сетевую ошибку, константа далеко закопана в проекте
            if (getParameter(PPSDK::CPayment::Parameters::ServerError).toInt() == -1) {
                emit stepCompleted(aPayment, aStep, false);

                return;
            }
        }
    }

    emit stepCompleted(aPayment, aStep, aError);
}

//------------------------------------------------------------------------------
void PaymentService::addProviderProvider(const QWeakPointer<IProviderProvider> &aProvider) {
    m_ProviderProviders << aProvider;
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::getStatistic() {
    QVariantMap statistic;
    QMapIterator<qint64, unsigned> i(m_PaymentService->getStatistic());

    while (i.hasNext()) {
        i.next();
        statistic.insert(QString::number(i.key()), i.value());
    }

    return statistic;
}

//------------------------------------------------------------------------------
void PaymentService::checkStatus() {
    PPSDK::TerminalSettings *terminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(
        m_Core->getSettingsService()->getAdapter(CAdapterNames::TerminalAdapter));

    QList<SDK::PaymentProcessor::SBlockByNote> notes =
        terminalSettings->getCommonSettings().blockNotes;

    QString currencyName = terminalSettings->getCurrencySettings().name;

    PPSDK::IDatabaseService *db = m_Core->getDatabaseService();

    QString queryStr =
        QString("SELECT `create_date` FROM `device_status` WHERE `fk_device_id` = 1 AND "
                "`description` = 'Unlocked' ORDER BY 1 DESC LIMIT 1");
    QSharedPointer<IDatabaseQuery> query(db->createAndExecQuery(queryStr));

    quint32 deltaTime = 0;

    if (query && query->first()) {
        deltaTime = qAbs(QDateTime::currentDateTime().secsTo(query->value(0).toDateTime()));
    }

    foreach (SDK::PaymentProcessor::SBlockByNote note, notes) {
        QString queryStr =
            QString("SELECT COUNT(*) FROM `payment_note` WHERE ((NOT `ejection`) OR "
                    "(`ejection` IS NULL)) AND "
                    "`type` <> 2 AND `nominal` = %1 AND `date` >= DATETIME('now', "
                    "'localtime', '-%2 seconds')")
                .arg(note.nominal)
                .arg((deltaTime != 0U) ? qMin(deltaTime, note.interval) : note.interval);

        QSharedPointer<IDatabaseQuery> query(db->createAndExecQuery(queryStr));

        if (query && query->first() && query->value(0).toUInt() > note.repeat) {
            setExternalParameter(PPSDK::CPayment::Parameters::Cheated,
                                 PPSDK::EPaymentCheatedType::NotesCount);

            m_Core->getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::TerminalLock));
            m_Core->getEventService()->sendEvent(
                PPSDK::Event(SDK::PaymentProcessor::EEventType::Critical,
                             "Payment",
                             QString("Terminal blocked by nominal limit reached: %1 %2 * %3")
                                 .arg(note.nominal)
                                 .arg(currencyName)
                                 .arg(query->value(0).toInt())));

            return;
        }
    }
}

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
