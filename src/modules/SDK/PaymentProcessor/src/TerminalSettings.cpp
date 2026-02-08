/* @file  Настройки терминала (данной инсталляции ПО). */

#include "TerminalSettings.h"

#include <SDK/Drivers/Components.h>

#include <array>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CDefaults {
const char DefaultDatabaseName[] = "data.db";
const int DefaultDatabasePort = 0;
} // namespace CDefaults

//---------------------------------------------------------------------------
namespace CServiceMenuPasswords {
const char Service[] = "service_password";
const char Screen[] = "screen_password";
const char Collection[] = "collection_password";
const char Technician[] = "technician_password";
} // namespace CServiceMenuPasswords

//---------------------------------------------------------------------------
QString TerminalSettings::getAdapterName() {
    return CAdapterNames::TerminalAdapter;
}

//---------------------------------------------------------------------------
TerminalSettings::TerminalSettings(TPtree &aProperties)
    : m_Properties(aProperties.get_child(CAdapterNames::TerminalAdapter, aProperties)) {}

//---------------------------------------------------------------------------
TerminalSettings::~TerminalSettings() {}

//---------------------------------------------------------------------------
void TerminalSettings::initialize() {
    foreach (auto error,
             m_Properties.get("config.terminal.critical_errors", QString())
                 .split(",", Qt::SkipEmptyParts)) {
        bool ok = false;
        int errorId = error.toInt(&ok);

        if (ok) {
            m_CriticalErrors << errorId;
        }
    }
}

//---------------------------------------------------------------------------
SConnection TerminalSettings::getConnection() const {
    SConnection connection;

    try {
        connection.name = m_Properties.get<QString>("terminal.connection.name");

        QString type = m_Properties.get<QString>("terminal.connection.type");
        connection.type = type.compare("modem", Qt::CaseInsensitive) ? EConnectionTypes::Unmanaged
                                                                     : EConnectionTypes::Dialup;
        connection.checkInterval =
            m_Properties.get("config.terminal.connection_check_interval.<xmlattr>.value",
                             CConnection::DefaultCheckInterval);
        connection.checkInterval = connection.checkInterval < 1 ? 1 : connection.checkInterval;

        QNetworkProxy proxy;

        auto proxyType = m_Properties.get<QString>("terminal.proxy.type", QString("none"));

        if (proxyType == "http") {
            proxy.setType(QNetworkProxy::HttpProxy);
        } else if (proxyType == "http_caching") {
            proxy.setType(QNetworkProxy::HttpCachingProxy);
        } else if (proxyType == "socks5") {
            proxy.setType(QNetworkProxy::Socks5Proxy);
        } else {
            proxy.setType(QNetworkProxy::NoProxy);
        }

        if (proxy.type() != QNetworkProxy::NoProxy) {
            proxy.setHostName(m_Properties.get<QString>("terminal.proxy.host"));
            proxy.setPort(m_Properties.get<QString>("terminal.proxy.port").toUShort());
            proxy.setPassword(m_Properties.get("terminal.proxy.password", QString()));
            proxy.setUser(m_Properties.get("terminal.proxy.login", QString()));
        }

        connection.proxy = proxy;
    } catch (std::exception &e) {
        toLog(LogLevel::Error,
              QString("Connection settings error: %1. Using defaults.").arg(e.what()));
    }

    return connection;
}

//---------------------------------------------------------------------------
void TerminalSettings::setConnection(const SConnection &aConnection) {
    m_Properties.put("terminal.connection.name", aConnection.name);
    m_Properties.put("terminal.connection.type",
                     (aConnection.type == EConnectionTypes::Dialup) ? "modem" : "unmanaged");

    if (aConnection.proxy.type() != QNetworkProxy::NoProxy) {
        m_Properties.put("terminal.proxy.host", aConnection.proxy.hostName());
        m_Properties.put("terminal.proxy.password", aConnection.proxy.password());
        m_Properties.put("terminal.proxy.login", aConnection.proxy.user());
        m_Properties.put("terminal.proxy.port", QString("%1").arg(aConnection.proxy.port()));

        QString proxyType;

        switch (aConnection.proxy.type()) {
        case QNetworkProxy::HttpCachingProxy:
            proxyType = "http_caching";
            break;
        case QNetworkProxy::HttpProxy:
            proxyType = "http";
            break;
        case QNetworkProxy::Socks5Proxy:
            proxyType = "socks5";
            break;
        default:
            proxyType = "http";
        }

        m_Properties.put("terminal.proxy.type", proxyType);
    } else {
        try {
            m_Properties.get_child("terminal.proxy").clear();
        } catch (std::exception &) {
            // Ветвь не была найдена.
        }
    }
}

//---------------------------------------------------------------------------
QList<IConnection::CheckUrl> TerminalSettings::getCheckHosts() const {
    QList<IConnection::CheckUrl> hosts;

    for (int i = 1;; i++) {
        try {
            QUrl url = m_Properties.get(QString("system.check_hosts.host%1").arg(i).toStdString(),
                                        QString());
            QString response = m_Properties.get(
                QString("system.check_hosts.response%1").arg(i).toStdString(), QString());

            if (url.isEmpty())
                break;

            hosts << IConnection::CheckUrl(url, response);
        } catch (std::exception &e) {
            toLog(LogLevel::Error, QString("Failed to parse check host. Error: %1.").arg(e.what()));

            break;
        }
    }

    return hosts;
}

//---------------------------------------------------------------------------
SDatabaseSettings TerminalSettings::getDatabaseSettings() const {
    SDatabaseSettings databaseSettings;

    databaseSettings.name =
        m_Properties.get("system.database.name", QString(CDefaults::DefaultDatabaseName));
    databaseSettings.host = m_Properties.get("system.database.host", QString());
    databaseSettings.port =
        m_Properties.get("system.database.port", CDefaults::DefaultDatabasePort);
    databaseSettings.user = m_Properties.get("system.database.user", QString());
    databaseSettings.password = m_Properties.get("system.database.password", QString());

    return databaseSettings;
}

//---------------------------------------------------------------------------
QStringList TerminalSettings::getDeviceList() const {
    QStringList deviceList;

    static TPtree emptyTreeDeviceList;
    BOOST_FOREACH (const TPtree::value_type &value,
                   m_Properties.get_child("terminal.hardware", emptyTreeDeviceList)) {
        deviceList.append(value.second.get_value<QString>());
    }

    return deviceList;
}

//---------------------------------------------------------------------------
void TerminalSettings::setDeviceList(const QStringList &aHardware) {
    TPtree branch;

    int id = 0;
    foreach (QString configName, aHardware) {
        branch.put(std::string("device") + boost::lexical_cast<std::string>(id), configName);
        id++;
    }

    m_Properties.put_child("terminal.hardware", branch);
}

//---------------------------------------------------------------------------
SMonitoringSettings TerminalSettings::getMonitoringSettings() const {
    SMonitoringSettings settings;

    settings.url = QUrl(m_Properties.get<QString>("system.monitoring.url", QString()));
    settings.restUrl = QUrl(m_Properties.get<QString>("system.monitoring.rest_url", QString()));
    settings.heartbeatTimeout =
        m_Properties.get<int>("system.monitoring.heartbeat", settings.heartbeatTimeout);
    settings.restCheckTimeout =
        m_Properties.get<int>("system.monitoring.rest_check_timeout", settings.restCheckTimeout);
    settings.restLimit = m_Properties.get<int>("config.terminal.block_by_rest", 0);

    settings.cleanupItems =
        m_Properties.get<QString>("system.user_cleanup.remove", "").split(",", Qt::SkipEmptyParts);
    for (QString &item : settings.cleanupItems) {
        item = item.trimmed();
    }
    settings.cleanupItems.removeAll("");

    settings.cleanupExclude =
        m_Properties.get<QString>("system.user_cleanup.exclude", "").split(",", Qt::SkipEmptyParts);
    for (QString &item : settings.cleanupExclude) {
        item = item.trimmed();
    }
    settings.cleanupExclude.removeAll("");

    return settings;
}

//---------------------------------------------------------------------------
QMap<int, SKeySettings> TerminalSettings::getKeys() const {
    QMap<int, SKeySettings> keys;

    static TPtree emptyTreeKeys;
    BOOST_FOREACH (const TPtree::value_type &value, m_Properties.get_child("keys", emptyTreeKeys)) {
        try {
            if (value.first == "<xmlattr>") {
                continue;
            }

            SKeySettings key;

            key.id = value.second.get<int>("<xmlattr>.id");
            key.engine = value.second.get<int>("engine", key.engine);
            key.ap = value.second.get<QString>("ap");
            key.sd = value.second.get<QString>("sd");
            key.op = value.second.get<QString>("op");
            key.serialNumber = value.second.get<ulong>("secret_serial_number", key.serialNumber);
            key.bankSerialNumber = value.second.get<ulong>("serial_number");
            key.publicKeyPath = value.second.get<QString>("public_key");
            key.secretKeyPath = value.second.get<QString>("secret_key");
            key.secretPassword = value.second.get<QString>("secret_password");
            key.description = value.second.get<QString>("description", QString());
            key.isValid = true;

            if (keys.contains(key.id)) {
                toLog(LogLevel::Error, QString("There is already a key with id = %1.").arg(key.id));
            } else {
                keys.insert(key.id, key);
            }
        } catch (std::exception &e) {
            toLog(LogLevel::Error, QString("Failed to load key data: %1.").arg(e.what()));
        }
    }

    return keys;
}

//---------------------------------------------------------------------------
void TerminalSettings::setKey(const SKeySettings &aKey, bool aReplaceIfExists) {
    auto applyConfig = [&](TPtree &aPair) {
        aPair.put("ap", aKey.ap);
        aPair.put("sd", aKey.sd);
        aPair.put("op", aKey.op);
        aPair.put("engine", aKey.engine);
        aPair.put("secret_serial_number", aKey.serialNumber);
        aPair.put("serial_number", aKey.bankSerialNumber);
        aPair.put("public_key", aKey.publicKeyPath);
        aPair.put("secret_key", aKey.secretKeyPath);
        aPair.put("secret_password", aKey.secretPassword);
        aPair.put("description", aKey.description);
    };

    static TPtree emptyTreeSetKey;
    BOOST_FOREACH (TPtree::value_type &value, m_Properties.get_child("keys", emptyTreeSetKey)) {
        try {
            if (value.first == "<xmlattr>") {
                continue;
            }

            if (value.second.get<int>("<xmlattr>.id") == aKey.id) {
                if (aReplaceIfExists) {
                    // Перезаписываем существующий ключ.
                    applyConfig(value.second);

                    return;
                }
            }
        } catch (std::exception &e) {
            toLog(LogLevel::Error, QString("setKey error: %1 .").arg(e.what()));
        }
    }

    if (aKey.isValid) {
        // Добавляем новую пару ключей.
        TPtree pair;

        pair.put("<xmlattr>.id", aKey.id);
        applyConfig(pair);

        m_Properties.add_child("keys.pair", pair);
    }
}

//---------------------------------------------------------------------------
void TerminalSettings::cleanKeys() {
    static TPtree emptyTreeCleanKeys;
    while (m_Properties.get_child("keys", emptyTreeCleanKeys).erase("pair") > 0)
        ;
}

//---------------------------------------------------------------------------
SCurrencySettings TerminalSettings::getCurrencySettings() const {
    SCurrencySettings currencySettings;
    currencySettings.id = m_Properties.get("system.currency.id", -1);

    if (currencySettings.id != -1) {
        currencySettings.code = m_Properties.get("system.currency.code", QString());
        currencySettings.name = m_Properties.get("system.currency.name", QString());

        if (currencySettings.code.isEmpty() || currencySettings.name.isEmpty()) {
            toLog(LogLevel::Warning,
                  QString("For currency id = %1 code or name is not found.")
                      .arg(currencySettings.id));
        }

        foreach (auto coin, m_Properties.get("system.currency.coins", QString()).split(",")) {
            currencySettings.coins << Currency::Nominal(coin.toDouble());
        }

        foreach (auto note, m_Properties.get("system.currency.notes", QString()).split(",")) {
            currencySettings.notes << Currency::Nominal(note.toInt());
        }
    }

    return currencySettings;
}

//----------------------------------------------------------------------------
const QSet<int> &TerminalSettings::getCriticalErrors() const {
    return m_CriticalErrors;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getKeygenURL() const {
    QString url = m_Properties.get("system.common.keygen_url", QString());

    if (url.isEmpty()) {
        toLog(LogLevel::Error, "Keygen url is not set!");
    }

    return url;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getReceiptMailURL() const {
    QString url = m_Properties.get("system.common.receipt_mail_url", QString());

    if (url.isEmpty()) {
        toLog(LogLevel::Error, "Url for receipt mail is not set!");
    }

    return url;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getFeedbackURL() const {
    QString url = m_Properties.get("system.common.feedback_url", QString());

    if (url.isEmpty()) {
        toLog(LogLevel::Error, "Url for feedback is not set!");
    }

    return url;
}

//---------------------------------------------------------------------------
QVariantMap TerminalSettings::getChargeProviderAccess() const {
    QVariantMap result;

    static TPtree emptyTreeChargeAccess;
    BOOST_FOREACH (const TPtree::value_type &value,
                   m_Properties.get_child("system.charge_access", emptyTreeChargeAccess)) {
        result.insert(QString::fromStdString(value.first),
                      value.second.get_value<QString>().split(","));
    }

    return result;
}

//---------------------------------------------------------------------------
SAppEnvironment TerminalSettings::getAppEnvironment() const {
    SAppEnvironment environment;

    environment.userDataPath = m_Properties.get("environment.user_data_path", QString());
    environment.contentPath = m_Properties.get("environment.content_path", QString());
    environment.interfacePath = m_Properties.get("environment.interface_path", QString());
    environment.adPath = m_Properties.get("environment.ad_path", QString());
    environment.version = m_Properties.get("environment.version", QString());

    return environment;
}

//---------------------------------------------------------------------------
void TerminalSettings::setAppEnvironment(const SAppEnvironment &aEnv) {
    m_Properties.put("environment.user_data_path", aEnv.userDataPath);
    m_Properties.put("environment.content_path", aEnv.contentPath);
    m_Properties.put("environment.interface_path", aEnv.interfacePath);
    m_Properties.put("environment.ad_path", aEnv.adPath);
    m_Properties.put("environment.version", aEnv.version);
}

//---------------------------------------------------------------------------
SCommonSettings TerminalSettings::getCommonSettings() const {
    SCommonSettings settings;

    static TPtree emptyTreeCommonSettings;
    if (m_Properties.get_child("config.hardware", emptyTreeCommonSettings).empty()) {
        settings.isValid = false;

        return settings;
    }

    settings.setBlockOn(
        SCommonSettings::ValidatorError,
        m_Properties.get("config.hardware.validator_settings.block_terminal_on_error", true));
    settings.autoEncashment = m_Properties.get("config.hardware.validator_settings.auto_encashment",
                                               settings.autoEncashment);
    settings.printFailedReceipts = m_Properties.get(
        "config.hardware.printer_settings.print_failed_receipts", settings.printFailedReceipts);
    settings.randomReceiptsID = m_Properties.get(
        "config.hardware.printer_settings.random_receipts_id", settings.randomReceiptsID);
    settings.enableBlankFiscalData =
        m_Properties.get("config.hardware.printer_settings.enable_blank_fiscal_data",
                         settings.enableBlankFiscalData);

    QString defaultZReportTime =
        (!settings.autoZReportTime.isNull() && settings.autoZReportTime.isValid())
            ? settings.autoZReportTime.toString("hh:mm")
            : "";
    settings.autoZReportTime = QTime::fromString(
        m_Properties.get("config.hardware.printer_settings.auto_z_report_time", defaultZReportTime),
        "hh:mm");

    settings.timeZoneOffset = m_Properties.get_optional<int>("config.terminal.timezone");

    // Получаем минимальный разрешенный номинал.
    auto minNote = m_Properties.get_optional<double>("config.hardware.validator_settings.min_note");
    if (minNote.is_initialized()) {
        settings.minPar = minNote.get();

        // Парсим список активированных купюр.
        foreach (const QString &str,
                 m_Properties.get("config.hardware.validator_settings.notes", QString())
                     .split(",", Qt::SkipEmptyParts)) {
            auto nominal = Currency::Nominal(str.toDouble());

            if (nominal >= settings.minPar) {
                settings.enabledParNotesList << nominal;
                settings.enabledParCoinsList << nominal;
            }
        }
    } else // Новый формат с раздельным указанием купюр и монет
    {
        settings.minPar = 10000;

        // Парсим список активированных монет.
        foreach (auto str,
                 m_Properties.get("config.hardware.validator_settings.coins", QString())
                     .split(",", Qt::SkipEmptyParts)) {
            auto nominal = Currency::Nominal(str.toDouble());
            settings.minPar = qMin(nominal, settings.minPar);
            settings.enabledParCoinsList << nominal;
        }

        // Парсим список активированных купюр.
        foreach (auto str,
                 m_Properties.get("config.hardware.validator_settings.notes", QString())
                     .split(",", Qt::SkipEmptyParts)) {
            auto nominal = Currency::Nominal(str.toInt());
            settings.minPar = qMin(nominal, settings.minPar);
            settings.enabledParNotesList.insert(nominal);
        }
    }

    settings.skipCheckWhileNetworkError = m_Properties.get(
        "config.terminal.skip_check_while_network_error", settings.skipCheckWhileNetworkError);

    settings.setBlockOn(
        SCommonSettings::PrinterError,
        m_Properties.get("config.hardware.printer_settings.block_terminal_on_error", true));
    settings.setBlockOn(
        SCommonSettings::CardReaderError,
        m_Properties.get("config.hardware.cardreader_settings.block_terminal_on_error", false));
    settings.setBlockOn(SCommonSettings::AccountBalance,
                        getMonitoringSettings().isBlockByAccountBalance());

    switch (m_Properties.get("config.terminal.block_by_penetration", 0)) {
    case 2:
        settings.penetrationEventLevel = EEventType::Critical;
        break;
    case 1:
        settings.penetrationEventLevel = EEventType::Warning;
        break;
    default:
        settings.penetrationEventLevel = EEventType::OK;
        break;
    }

    settings.setBlockOn(SCommonSettings::Penetration,
                        settings.penetrationEventLevel == EEventType::Critical);

    auto updateBlockNotes = [&settings](quint32 aNominal, quint32 aInterval, quint32 aRepeat) {
        if (aInterval && aRepeat) {
            settings.blockNotes << SBlockByNote(aNominal, aInterval, aRepeat);
        }
    };

    settings.blockCheatedPayment =
        m_Properties.get("config.terminal.block_cheated_payment", settings.blockCheatedPayment);

    static TPtree emptyTreeBlockByNote;
    BOOST_FOREACH (const TPtree::value_type &notes,
                   m_Properties.get_child("config.terminal.block_by_note", emptyTreeBlockByNote)) {
        BOOST_FOREACH (const TPtree::value_type &note, notes.second) {
            if (note.first == "<xmlattr>") {
                updateBlockNotes(note.second.get<int>("nominal"),
                                 note.second.get<int>("interval"),
                                 note.second.get<int>("repeat"));
            }
        }
    }

    settings.disableAmountOverflow =
        m_Properties.get("config.hardware.validator_settings.disable_amount_overflow",
                         settings.disableAmountOverflow);

    return settings;
}

//---------------------------------------------------------------------------
SServiceMenuPasswords TerminalSettings::getServiceMenuPasswords() const {
    SServiceMenuPasswords passwords;

    passwords.phone = m_Properties.get("config.service_menu.phone", QString());
    passwords.operatorId = m_Properties.get("config.service_menu.operator", -1);

    std::array<QString, 4> passwordTypes = {CServiceMenuPasswords::Service,
                                            CServiceMenuPasswords::Screen,
                                            CServiceMenuPasswords::Collection,
                                            CServiceMenuPasswords::Technician};

    for (size_t i = 0; i < passwordTypes.size(); i++) {
        try {
            passwords.passwords[passwordTypes[i]] = m_Properties.get<QString>(
                (QString("config.service_menu.") + passwordTypes[i]).toStdString());
        } catch (std::exception &e) {
            toLog(LogLevel::Error, QString("Error %1.").arg(e.what()));
        }
    }

    return passwords;
}

//---------------------------------------------------------------------------
SServiceMenuSettings TerminalSettings::getServiceMenuSettings() const {
    SServiceMenuSettings settings;

    settings.allowAnyKeyPair = m_Properties.get("config.service_menu.alloy_any_keypair", false);

    return settings;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getPrinterForReceipt(const QString &aReceiptType) {
    static TPtree emptyTreeReceipts;
    auto receipts = m_Properties.get_child("terminal.receipts", emptyTreeReceipts);
    auto deviceAlias = receipts.get(aReceiptType.toStdString(), QString());

    // Ищем девайс с таким алиасом в списке всех устройств.
    static TPtree emptyTreePrinterDevices;
    auto devices = m_Properties.get_child("terminal.hardware", emptyTreePrinterDevices);
    return devices.get(deviceAlias.toStdString(), QString());
}

//---------------------------------------------------------------------------
bool TerminalSettings::isValid() const {
    QStringList configs = QStringList() << "environment"
                                        << "keys"
                                        << "terminal"
                                        << "system"
                                        << "config";
    QStringList errorConfigs;

    foreach (const QString &aConfig, configs) {
        static TPtree emptyTreeIsValid;
        if (m_Properties.get_child(aConfig.toStdString(), emptyTreeIsValid).empty()) {
            errorConfigs << aConfig;
        }
    }

    if (!errorConfigs.isEmpty()) {
        toLog(LogLevel::Error,
              "Failed to validate configuration settings due to bad section(s): " +
                  errorConfigs.join(", "));
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
int TerminalSettings::getLogsMaxSize() const {
    return m_Properties.get("config.terminal.logs.<xmlattr>.max_size", 100);
}

//---------------------------------------------------------------------------
QStringList TerminalSettings::getUpdaterUrls() const {
    QStringList result;

    result << m_Properties.get("system.updater.url", QString())
           << m_Properties.get("system.updater.data_url", QString());

    return result;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getAdProfile() const {
    return m_Properties.get("config.terminal.ad.<xmlattr>.profile", QString());
}

//---------------------------------------------------------------------------
QTime TerminalSettings::autoUpdate() const {
    if (m_Properties.get<bool>("config.terminal.check_update", false)) {
        return QTime::fromString(
            m_Properties.get<QString>("config.terminal.check_update.<xmlattr>.start", ""), "hh:mm");
    }

    return QTime();
}

//---------------------------------------------------------------------------
QString TerminalSettings::energySave() const {
    QStringList result =
        QStringList() << m_Properties.get<QString>("config.hardware.energy_save.<xmlattr>.from", "")
                      << m_Properties.get<QString>("config.hardware.energy_save.<xmlattr>.till", "")
                      << m_Properties.get<QString>("config.hardware.energy_save", "");

    return result.join(";");
}

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
