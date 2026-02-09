/* @file Настройки дилера: операторы, комиссии, персональная информация и т.п. */

// Stl

#include "DealerSettings.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QStack>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamReader>

#include <Common/ILog.h>

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace SDK {
namespace PaymentProcessor {

using TPtreeOperators = boost::property_tree::basic_ptree<std::string, std::string>;

//---------------------------------------------------------------------------
DealerSettings::DealerSettings(TPtree &aProperties)
    : m_Properties(aProperties.get_child(CAdapterNames::DealerAdapter, aProperties)),
      m_ProvidersLock(QReadWriteLock::Recursive), m_IsValid(false) {}

//---------------------------------------------------------------------------
DealerSettings::~DealerSettings() = default;

//---------------------------------------------------------------------------
QString DealerSettings::getAdapterName() {
    return CAdapterNames::DealerAdapter;
}

//---------------------------------------------------------------------------
void DealerSettings::initialize() {
    bool r1 = loadProviders();
    bool r2 = loadCommissions();
    bool r3 = loadPersonalSettings();

    m_IsValid = r1 && r2 && r3;

    // Проверяем наличие групп.
    static const TPtree EmptyTree;
    if (m_Properties.get_child("groups", EmptyTree).empty()) {
        m_IsValid = false;
    }

    m_Properties.clear();
}

//---------------------------------------------------------------------------
inline QString &encodeLTGT(QString &value) {
    return value.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
}

//---------------------------------------------------------------------------
inline QString &encodeQUOT(QString &value) {
    return value.replace("\"", "&quot;");
}

//---------------------------------------------------------------------------
bool DealerSettings::loadOperatorsXML(const QString &aFileName) {
    QFile inputFile(aFileName);

    if (!inputFile.open(QIODevice::ReadOnly)) {
        toLog(LogLevel::Error, QString("Failed to open file: %1.").arg(aFileName));
        return false;
    }

    std::string op;
    qint64 opID = 0;
    QString processing;
    QSet<qint64> cids;
    double operatorsVersion = 0.;
    QStack<QString> tags;

    op.reserve(4096);

    QXmlStreamReader xmlReader(&inputFile);

    while (!xmlReader.atEnd()) {
        QXmlStreamReader::TokenType token = xmlReader.readNext();

        switch (token) {
        // Встретили открывающий тег.
        case QXmlStreamReader::StartElement: {
            tags << xmlReader.name().toString().toLower();

            bool isOP = (tags.top() == "operator");

            if (isOP) {
                op = R"(<?xml version="1.0" encoding="utf-8"?><operator version=")" +
                     QString::number(operatorsVersion, 'f').toStdString() + "\"";

                opID = 0;
                processing.clear();
                cids.clear();
            } else {
                op += "<" + tags.top().toLatin1();
            }

            // Обрабатываем список атрибутов, если такие имеются.
            QXmlStreamAttributes attributes = xmlReader.attributes();

            if (!attributes.isEmpty()) {
                QMap<QString, QString> attrs;

                foreach (const QXmlStreamAttribute &attribute, attributes) {
                    QString value = attribute.value().toString();
                    attrs[attribute.name().toString().toLower()].swap(encodeQUOT(value));
                }

                if (tags.top() == "operators") {
                    operatorsVersion = attrs.value("version", "0").toDouble();
                } else if (isOP) {
                    opID = attrs.value("id").toLongLong();

                    if (operatorsVersion < 2.0) {
                        cids << attrs.value("cid").toLongLong();
                    }
                } else if (tags.top() == "processor") {
                    // Процессинг вида тип_процессинга#имя является алиасом для стандартных типов
                    processing = attrs.value("type").split("#").takeFirst();
                }

                foreach (auto name, attrs.keys()) {
                    op += " " + name.toLatin1() + "=\"" + attrs.value(name).toUtf8() + "\"";
                }
            }

            op += ">";

            break;
        }

        // Текст внутри тегов.
        case QXmlStreamReader::Characters: {
            if (!xmlReader.isWhitespace()) {
                QString text = xmlReader.text().toString();
                op += encodeLTGT(text).toUtf8();

                if (operatorsVersion >= 2.0) {
                    if (tags.size() > 2 && tags.at(tags.size() - 2) == "operator") {
                        if (tags.top() == "cid") {
                            cids << xmlReader.text().toString().toLongLong();
                        }
                    }
                }

                if (tags.top() == "tt_list") {
                    foreach (auto cid, xmlReader.text().toString().split(",")) {
                        cids << cid.trimmed().toLongLong();
                    }
                }
            }

            break;
        }

        // Встретили закрывающий тег.
        case QXmlStreamReader::EndElement: {
            QString key = xmlReader.name().toString().toLower();

            if (key == "operator") {
                op += "</operator>";

                if (m_ProviderRawBuffer.contains(opID)) {
                    // qDebug() << "Skip existing operator " << opID;
                } else {
                    m_ProviderRawBuffer[opID].swap(op);
                    foreach (qint64 cid, cids) {
                        m_ProviderGateways.insert(cid, opID);
                    }
                    // operatorsProcessing[opID].swap(processing);
                    // operatorsHash[opID].swap(QString::fromLatin1(CCryptographicHash::hash(op,
                    // CCryptographicHash::Sha256).toHex()));
                    m_ProvidersProcessingIndex.insert(processing, opID);
                }

                cids.clear();
                op.reserve(4096);
                processing.clear();
                opID = 0;
            } else {
                op += "</" + key.toStdString() + ">";
            }

            tags.pop();
            break;
        }

        // Ошибка в формате документа.
        case QXmlStreamReader::Invalid: {
            toLog(LogLevel::Error,
                  QString("'%1' parsing error: %2, line %3, column %4.")
                      .arg(aFileName)
                      .arg(xmlReader.errorString())
                      .arg(xmlReader.lineNumber())
                      .arg(xmlReader.columnNumber()));

            return false;
        }

        default:
            // Handle other token types that we're not explicitly processing
            break;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
bool DealerSettings::loadProviders() {
    const TPtree emptyTree;

    toLog(LogLevel::Normal, "Loading providers.");

    QElapsedTimer elapsed;
    elapsed.start();

    BOOST_FOREACH (const TPtree::value_type &operators, m_Properties.get_child("", emptyTree)) {
        if (operators.first != "operators") {
            continue;
        }

        QString operatorsPath = QString::fromStdWString(operators.second.get_value(L""));

        toLog(LogLevel::Normal, QString("Loading %1.").arg(operatorsPath));

        loadOperatorsXML(operatorsPath);
    }

    toLog(LogLevel::Normal,
          QString("Total providers loaded: %1, elapsed %2 ms.")
              .arg(m_ProviderRawBuffer.size())
              .arg(elapsed.elapsed()));

    return !m_ProviderRawBuffer.isEmpty();
}

//---------------------------------------------------------------------------
void loadProviderEnumItems(SProviderField::TEnum_Items &aItemList, const TPtreeOperators &aTree) {
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
                     [](const SProviderField::SEnum_Item &a, const SProviderField::SEnum_Item &b) {
                         return a.sort < b.sort;
                     });
}

//---------------------------------------------------------------------------
bool DealerSettings::loadProvidersFrom_Buffer(const std::string &aBuffer, SProvider &aProvider) {
    TPtreeOperators operators;
    const TPtreeOperators emptyTree;

    try {
        std::stringstream stream(aBuffer);

        boost::property_tree::read_xml(stream, operators);
    } catch (boost::property_tree::xml_parser_error &e) {
        toLog(LogLevel::Error,
              QString("XML parser error: %1.").arg(QString::fromStdString(e.message())));

        return false;
    }

    BOOST_FOREACH (const auto &value, operators.get_child("", emptyTree)) {
        try {
            double operatorsVersion = value.second.get<double>("<xmlattr>.version", 0);
            aProvider.id = value.second.get<qint64>("<xmlattr>.id");
            aProvider.cid = value.second.get<qint64>("<xmlattr>.cid", -1);

            // Для совместимости с operators.xml version=2.0
            if (operatorsVersion >= 2.0) {
                aProvider.cid = value.second.get<qint64>("cid", -1);

                QString terminalShow = value.second.get<QString>("terminal_show", "1").trimmed();
                terminalShow = terminalShow.isEmpty() ? "1" : terminalShow;

                QString enabled = value.second.get<QString>("enabled", "1").trimmed();
                enabled = enabled.isEmpty() ? "1" : enabled;

                if (terminalShow.toInt() == 0 || enabled.toInt() == 0) {
                    return false;
                }
            }

            QString ttList = value.second.get<QString>("tt_list", QString());
            foreach (auto cid, ttList.split(",", Qt::SkipEmptyParts)) {
                aProvider.ttList.insert(cid.trimmed().toLongLong());
            }

            aProvider.type = value.second.get<QString>("<xmlattr>.type", "humo");
            aProvider.name = value.second.get<QString>("name");
            aProvider.comment = value.second.get<QString>("comment", QString());

            // Подгружаем лимиты
            TPtreeOperators limitBranch = value.second.get_child("limit.<xmlattr>");

            aProvider.limits.min = limitBranch.get<QString>("min");
            aProvider.limits.max = limitBranch.get<QString>("max");
            aProvider.limits.system = limitBranch.get<QString>("system", QString());
            aProvider.limits.check = limitBranch.get<QString>("check", QString());

            // Проверяем корректность лимитов.
            if (aProvider.limits.min.isEmpty() ||
                (aProvider.limits.system.isEmpty() && aProvider.limits.max.isEmpty())) {
                throw std::runtime_error(QString("operator %1: unspecified max or min limit")
                                             .arg(aProvider.id)
                                             .toStdString());
            }

            // Парсим процессор
            TPtreeOperators processorBranch = value.second.get_child("processor");

            auto processorBranchAttr = processorBranch.get_child("<xmlattr>");

            aProvider.processor.type = processorBranchAttr.get<QString>("type");
            aProvider.processor.keyPair = processorBranchAttr.get<int>("keys", 0);
            aProvider.processor.clientCard = processorBranchAttr.get<int>("client_card", 0);
            if (aProvider.processor.clientCard == 0) {
                // читаем атрибут по старому пути, если не нашли его в разделе processor
                aProvider.processor.clientCard = value.second.get<int>("client_card", 0);
            }
            aProvider.processor.skipCheck = processorBranchAttr.get<bool>("skip_check", true);
            aProvider.processor.payOnline = processorBranchAttr.get<bool>("pay_online", false);
            aProvider.processor.askForRetry = processorBranchAttr.get<bool>("ask_for_retry", false);
            aProvider.processor.requirePrinter =
                processorBranchAttr.get<bool>("require_printer", false);
            aProvider.processor.feeType =
                processorBranchAttr.get<QString>("fee_type", "amount_all") == "amount"
                    ? SProvider::FeeByAmount
                    : SProvider::FeeByAmountAll;
            aProvider.processor.rounding = processorBranchAttr.get<bool>("rounding", false);
            aProvider.processor.showAddInfo = processorBranchAttr.get<bool>("show_add_info", false);

            auto requests = processorBranch.equal_range("request");

            for (auto requestIt = requests.first; requestIt != requests.second; ++requestIt) {
                SProvider::SProcessingTraits::SRequest request;

                request.url = requestIt->second.get<QString>("url");

                // Отправляемые поля
                auto searchBounds = requestIt->second.equal_range("request_property");

                for (auto fieldIt = searchBounds.first; fieldIt != searchBounds.second; ++fieldIt) {
                    SProvider::SProcessingTraits::SRequest::SField field;

                    auto attr = fieldIt->second.get_child("<xmlattr>");

                    field.name = attr.get<QString>("name");
                    field.value = attr.get<QString>("value");
                    field.crypted = attr.get<bool>("crypted", field.crypted);

                    QString algorithm = attr.get<QString>("algorithm", "ipriv").toLower();
                    if (algorithm == "md5") {
                        field.algorithm = SProvider::SProcessingTraits::SRequest::SField::Md5;
                    } else if (algorithm == "sha1") {
                        field.algorithm = SProvider::SProcessingTraits::SRequest::SField::Sha1;
                    } else if (algorithm == "sha256") {
                        field.algorithm = SProvider::SProcessingTraits::SRequest::SField::Sha256;
                    }

                    request.requestFields << field;
                }

                // Ожидаемые поля
                searchBounds = requestIt->second.equal_range("receive_property");

                for (auto fieldIt = searchBounds.first; fieldIt != searchBounds.second; ++fieldIt) {
                    SProvider::SProcessingTraits::SRequest::SResponseField field;

                    auto attr = fieldIt->second.get_child("<xmlattr>");

                    field.name = attr.get<QString>("name");
                    field.crypted = attr.get<bool>("crypted", field.crypted);
                    field.required = attr.get<bool>("required", field.required);

                    field.setCodepage(attr.get<QString>("codepage", ""));
                    field.setEncoding(attr.get<QString>("encoding", ""));

                    request.responseFields << field;
                }

                aProvider.processor
                    .requests[requestIt->second.get<QString>("<xmlattr>.name").toUpper()] = request;
            }

            BOOST_FOREACH (const auto &fieldIt, value.second.get_child("fields")) {
                SProviderField field;

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

                QString externalDataHandler =
                    fieldIt.second.get<QString>("on_external_data", QString());
                if (!externalDataHandler.isEmpty()) {
                    aProvider.externalDataHandler = externalDataHandler;
                }

                loadProviderEnumItems(field.enum_Items,
                                      fieldIt.second.get_child("enum", emptyTree));

                auto security = fieldIt.second.get_child("security", emptyTree);
                if (!security.empty()) {
                    field.security.insert(SProviderField::SecuritySubsystem::Default,
                                          security.get<QString>("<xmlattr>.hidemask", QString()));

                    if (field.security.value(SProviderField::SecuritySubsystem::Default, "")
                            .isEmpty()) {
                        throw std::runtime_error("empty security@hideMask attribute");
                    }

                    BOOST_FOREACH (auto subSystem, security.get_child("", emptyTree)) {
                        if (subSystem.first == "printer") {
                            field.security.insert(
                                SProviderField::SecuritySubsystem::Printer,
                                subSystem.second.get<QString>("<xmlattr>.hidemask", QString()));
                        } else if (subSystem.first == "logger") {
                            field.security.insert(
                                SProviderField::SecuritySubsystem::Log,
                                subSystem.second.get<QString>("<xmlattr>.hidemask", QString()));
                        } else if (subSystem.first == "display") {
                            field.security.insert(
                                SProviderField::SecuritySubsystem::Display,
                                subSystem.second.get<QString>("<xmlattr>.hidemask", QString()));
                        }
                    }
                }

                aProvider.fields << field;
            }

            std::stable_sort(
                aProvider.fields.begin(),
                aProvider.fields.end(),
                [](const SProviderField &a, const SProviderField &b) { return a.sort < b.sort; });

            // Типы и параметры чеков
            BOOST_FOREACH (const auto &receiptIt, value.second.get_child("receipts", emptyTree)) {
                if (receiptIt.first == "parameter") {
                    aProvider.receiptParameters.insert(
                        receiptIt.second.get<QString>("<xmlattr>.name"),
                        receiptIt.second.get<QString>("<xmlattr>.value", QString()));
                } else if (receiptIt.first == "receipt") {
                    aProvider.receipts.insert(
                        receiptIt.second.get<QString>("<xmlattr>.type"),
                        receiptIt.second.get<QString>("<xmlattr>.template", QString()));
                }
            }
        } catch (std::runtime_error &error) {
            toLog(LogLevel::Error,
                  QString("Failed to load provider (id:%1, cid:%2). Error: %3.")
                      .arg(aProvider.id)
                      .arg(aProvider.cid)
                      .arg(error.what()));

            return false;
        }

        break;
    }

    return true;
}

//---------------------------------------------------------------------------
void DealerSettings::disableProvider(qint64 aId) {
    QWriteLocker locker(&m_ProvidersLock);

    m_Providers.remove(aId);
    m_ProviderRawBuffer.remove(aId);
    m_ProvidersProcessingIndex.remove(m_ProvidersProcessingIndex.key(aId), aId);

    foreach (auto cid, m_ProviderGateways.keys()) {
        m_ProviderGateways.remove(cid, aId);
    }
}

//----------------------------------------------------------------------------
bool DealerSettings::loadCommissions() {
    try {
        const TPtree emptyTree;

        toLog(LogLevel::Normal, "Loading commissions.");

        BOOST_FOREACH (const TPtree::value_type &commissions,
                       m_Properties.get_child("", emptyTree)) {
            if (commissions.first != "commissions") {
                continue;
            }

            if (!m_Commissions.isValid()) {
                m_Commissions = Commissions::from_Settings(commissions.second);
            } else {
                m_Commissions.appendFrom_Settings(commissions.second);
            }
        }
    } catch (std::runtime_error &error) {
        toLog(LogLevel::Error, QString("Failed to load commissions. Error: %1.").arg(error.what()));
        return false;
    }

    try {
        toLog(LogLevel::Normal, "Loading customers.");

        BOOST_FOREACH (const TPtree::value_type &value, m_Properties.get_child("customers")) {
            if (value.first == "<xmlattr>") {
                continue;
            }

            TPtree operatorSettings = value.second.get_child("operator");

            SCustomer customer;

            customer.blocked = operatorSettings.get<bool>("<xmlattr>.blocked", false);

            std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds =
                operatorSettings.equal_range("field");
            for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second;
                 ++it) {
                customer.addValue(it->second.get<QString>("<xmlattr>.value").trimmed());
            }

            if (!customer.blocked) {
                customer.commissions = Commissions::from_Settings(operatorSettings);
            }

            if (!customer.isEmpty()) {
                m_Customers << customer;
            }
        }
    } catch (std::runtime_error &error) {
        toLog(LogLevel::Error, QString("Failed to load customers. Error: %1.").arg(error.what()));
    }

    return true;
}

//----------------------------------------------------------------------------
bool DealerSettings::loadPersonalSettings() {
    toLog(LogLevel::Normal, "Loading personal settings.");

    try {
        TPtree &branch = m_Properties.get_child("config.dealer");

        m_PersonalSettings.pointName = branch.get("point_name", QString());
        m_PersonalSettings.pointAddress = branch.get("point_address", QString());
        m_PersonalSettings.pointExternalID =
            branch.get("external_id_for_cash_collection", QString());
        m_PersonalSettings.name = branch.get("dealer_name", QString());
        m_PersonalSettings.address = branch.get("dealer_address", QString());
        m_PersonalSettings.businessAddress = branch.get("dealer_business_address", QString());
        m_PersonalSettings.inn = branch.get("dealer_inn", QString());
        m_PersonalSettings.kbk = branch.get("dealer_kbk", QString());
        m_PersonalSettings.phone = branch.get("dealer_support_phone", QString());
        m_PersonalSettings.isBank = branch.get("dealer_is_bank", QString("0"));

        m_PersonalSettings.operatorName = branch.get("operator_name", QString());
        m_PersonalSettings.operatorAddress = branch.get("operator_address", QString());
        m_PersonalSettings.operatorInn = branch.get("operator_inn", QString());
        m_PersonalSettings.operatorContractNumber =
            branch.get("operator_contract_number", QString());

        m_PersonalSettings.bankName = branch.get("bank_name", QString());
        m_PersonalSettings.bankAddress = branch.get("bank_address", QString());
        m_PersonalSettings.bankBik = branch.get("bank_bik", QString());
        m_PersonalSettings.bankInn = branch.get("bank_inn", QString());
        m_PersonalSettings.bankPhone = branch.get("bank_phone", QString());
        m_PersonalSettings.bankContractNumber = branch.get("contract_number", QString());

        const TPtree emptyTree;

        BOOST_FOREACH (const TPtree::value_type &parameter,
                       m_Properties.get_child("config.printing.parameters", emptyTree)) {
            m_PersonalSettings.m_PrintingParameters.insert(
                QString::fromStdString(parameter.first).toUpper(),
                parameter.second.get_value(QString()).replace("\\n", "\n"));
        }
    } catch (std::runtime_error &error) {
        toLog(LogLevel::Error, QString("Failed to load personal settings : %1.").arg(error.what()));
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
SProvider DealerSettings::getProvider(qint64 aId) {
    if (m_ProviderRawBuffer.contains(aId)) {
        QWriteLocker locker(&m_ProvidersLock);

        SProvider provider;
        if (loadProvidersFrom_Buffer(m_ProviderRawBuffer[aId], provider)) {
            m_Providers.insert(aId, provider);
            m_ProviderRawBuffer.remove(aId);
        } else {
            toLog(LogLevel::Error, QString("Error parse provider %1 from buffer.").arg(aId));
        }
    }

    QReadLocker locker(&m_ProvidersLock);

    if (m_Providers.contains(aId)) {
        SProvider provider = m_Providers.value(aId);

        // Лимиты из описания оператора могут быть переопределены снаружи
        provider.limits.min = !qFuzzyIsNull(provider.limits.externalMin.toDouble())
                                  ? provider.limits.externalMin
                                  : provider.limits.min;
        provider.limits.max = !qFuzzyIsNull(provider.limits.externalMax.toDouble())
                                  ? provider.limits.externalMax
                                  : provider.limits.max;

        return provider;
    }

    return {};
}

//----------------------------------------------------------------------------
SProvider DealerSettings::getMNPProvider(qint64 aId, qint64 aCidIn, qint64 aCidOut) {
    auto provider = getProvider(aId);

    if (aCidIn == 0 && aCidOut == 0) {
        return provider;
    }

    if ((aCidOut != 0) && (provider.cid == aCidOut || provider.ttList.contains(aCidOut))) {
        return provider;
    }

    auto providers = getProvidersByCID(aCidOut);

    if (providers.isEmpty() && aCidIn > 0) {
        providers = getProvidersByCID(aCidIn);
    }

    return providers.isEmpty() ? provider : providers.at(0);
}

//----------------------------------------------------------------------------
QList<SProvider> DealerSettings::getProvidersByCID(qint64 aCid) {
    QList<SProvider> providers;

    foreach (auto id, m_ProviderGateways.values(aCid)) {
        providers << getProvider(id);
    }

    return providers;
}

//---------------------------------------------------------------------------
QList<qint64> DealerSettings::getProviders(const QString &aProcessingType) {
    return m_ProvidersProcessingIndex.values(aProcessingType);
}

//---------------------------------------------------------------------------
QStringList DealerSettings::getProviderProcessingTypes() {
    return {m_ProvidersProcessingIndex.keys().begin(), m_ProvidersProcessingIndex.keys().end()};
}

//---------------------------------------------------------------------------
void DealerSettings::setExternalLimits(qint64 aProviderId,
                                       double aMinExternalLimit,
                                       double aMaxExternalLimit) {
    SProvider provider = getProvider(aProviderId);

    if (!provider.isNull()) {
        m_Providers[aProviderId].limits.externalMin = QString::number(aMinExternalLimit);
        m_Providers[aProviderId].limits.externalMax = QString::number(aMaxExternalLimit);
    }
}

//---------------------------------------------------------------------------
const SPersonalSettings &DealerSettings::getPersonalSettings() const {
    return m_PersonalSettings;
}

//---------------------------------------------------------------------------
DealerSettings::TCustomers::iterator DealerSettings::findCustomer(const QVariantMap &aParameters) {
    QSet<QString> parametersValueSet;

    foreach (auto value, aParameters.values()) {
        parametersValueSet << value.toString();
    }

    auto isItRightCustomer = [&](const SCustomer &aCustomer) -> bool {
        return aCustomer.contains(parametersValueSet);
    };

    return std::find_if(m_Customers.begin(), m_Customers.end(), isItRightCustomer);
}

//---------------------------------------------------------------------------
void DealerSettings::setExternalCommissions(const Commissions &aCommissions) {
    m_ExternalCommissions = aCommissions;
}

//---------------------------------------------------------------------------
void DealerSettings::resetExternalCommissions() {
    m_ExternalCommissions.clear();
}

//---------------------------------------------------------------------------
bool DealerSettings::isCustomerAllowed(const QVariantMap &aParameters) {
    TCustomers::iterator it = findCustomer(aParameters);

    return it == m_Customers.end() ? true : !it->blocked;
}

//---------------------------------------------------------------------------
TCommissions DealerSettings::getCommissions(qint64 aProvider, const QVariantMap &aParameters) {
    if (m_ExternalCommissions.isValid() && m_ExternalCommissions.contains(aProvider)) {
        return m_ExternalCommissions.getCommissions(aProvider);
    }

    TCustomers::iterator it = findCustomer(aParameters);

    // Игнорируем настройки customer комиссии если комиссия процессинга не нулевая.
    return it == m_Customers.end() ? m_Commissions.getCommissions(aProvider)
                                   : it->commissions.getCommissions(aProvider);
}

//---------------------------------------------------------------------------
Commission
DealerSettings::getCommission(qint64 aProvider, const QVariantMap &aParameters, double aSum) {
    if (m_ExternalCommissions.isValid() && m_ExternalCommissions.contains(aProvider)) {
        return m_ExternalCommissions.getCommission(aProvider, aSum);
    }

    TCustomers::iterator it = findCustomer(aParameters);

    // Игнорируем настройки customer комиссии если комиссия процессинга не нулевая.
    return it == m_Customers.end() ? m_Commissions.getCommission(aProvider, aSum)
                                   : it->commissions.getCommission(aProvider, aSum);
}

//---------------------------------------------------------------------------
ProcessingCommission DealerSettings::getProcessingCommission(qint64 aProvider) {
    if (m_ExternalCommissions.isValid() && m_ExternalCommissions.contains(aProvider)) {
        return m_ExternalCommissions.getProcessingCommission(aProvider);
    }

    return m_Commissions.getProcessingCommission(aProvider);
}

//---------------------------------------------------------------------------
int DealerSettings::getVAT(qint64 aProvider) {
    if (m_ExternalCommissions.isValid() && m_ExternalCommissions.contains(aProvider)) {
        return m_ExternalCommissions.getVAT(aProvider);
    }

    return m_Commissions.getVAT(aProvider);
}

//---------------------------------------------------------------------------
QList<SProvider> DealerSettings::getProvidersByRange(QList<SRange> aRanges, QSet<qint64> aExclude) {
    QList<SProvider> providers;

    foreach (SRange range, aRanges) {
        if (!range.ids.isEmpty()) {
            foreach (qint64 id, range.ids) {
                SProvider p = getProvider(id);

                if (!p.isNull()) {
                    providers << p;
                }
            }
        } else {
            foreach (qint64 cid, range.cids) {
                foreach (const SProvider &p, getProvidersByCID(cid)) {
                    if (!aExclude.contains(p.id)) {
                        providers << p;
                    }
                }
            }
        }
    }

    return providers;
}

//---------------------------------------------------------------------------
bool DealerSettings::isValid() const {
    return m_IsValid;
}

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
