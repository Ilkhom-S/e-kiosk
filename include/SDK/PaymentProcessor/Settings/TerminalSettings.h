/* @file  Настройки терминала (данной инсталляции ПО). */

#pragma once

// boost
#include <QtCore/QStringList>
#include <QtCore/QTime>

#include <Common/Currency.h>
#include <Common/ILogable.h>
#include <Common/PropertyTree.h>

#include <SDK/PaymentProcessor/Connection/Connection.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>

#include <Connection/IConnection.h>
#include <boost/optional.hpp>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
struct SDatabaseSettings {
    QString host;
    QString name;
    QString user;
    QString password;
    int port{0};

    SDatabaseSettings() : {}
};

//---------------------------------------------------------------------------
struct SKeySettings {
    bool isValid{false};
    int id{-100}; /// -1 - root key, -100 - invalid value
    int engine{0};
    QString sd;                /// Код дилера.
    QString ap;                /// Код точки.
    QString op;                /// Код оператора.
    ulong serialNumber{0};     /// Серийник ключа.
    ulong bankSerialNumber{0}; /// Серийник банковского ключа.
    QString publicKeyPath;     /// Путь к открытому ключу терминала.
    QString secretKeyPath;     /// Путь к закрытому ключу терминала.
    QString secretPassword;    /// Кодовая фраза.
    QString description;       /// Поле для заметок

    SKeySettings() : {}
};

//----------------------------------------------------------------------------
struct SCurrencySettings {
    int id{-1};                     /// Идентификатор валюты по ISO.
    QString code;                   /// Международное имя валюты ISO.
    QString name;                   /// Имя валюты в текущей локализации.
    QList<Currency::Nominal> coins; /// Список всех существующих номиналов монет
    QList<Currency::Nominal> notes; /// Список всех существующих номиналов купюр

    SCurrencySettings() : {}
};

//----------------------------------------------------------------------------
struct SAppEnvironment {
    QString userDataPath;
    QString contentPath;
    QString interfacePath;
    QString adPath;
    QString version;
};

//----------------------------------------------------------------------------
struct SBlockByNote {
    quint32 nominal;
    quint32 interval;
    quint32 repeat;

    SBlockByNote() : nominal(0), interval(0), repeat(0) {}
    explicit SBlockByNote(quint32 aNominal, quint32 aInterval, quint32 aRepeat)
        : nominal(aNominal), interval(aInterval), repeat(aRepeat) {}
};

//----------------------------------------------------------------------------
struct SCommonSettings {
    using BlockReason = enum {
        ValidatorError = 1,
        PrinterError,
        CardReaderError,
        AccountBalance,
        Penetration
    };

    QSet<BlockReason> blockOn;
    QList<SBlockByNote> blockNotes;  /// Блокировка по номиналам
    bool blockCheatedPayment{false}; /// Блокировка при подозрении на манипуляции с устройством
    bool autoEncashment{false};
    Currency::Nominal minPar;
    QSet<Currency::Nominal> enabledParNotesList; /// Список разрешенных купюр.
    QSet<Currency::Nominal> enabledParCoinsList; /// Список разрешенных монет.
    boost::optional<int> timeZoneOffset;
    bool skipCheckWhileNetworkError{false}; /// Принимать платежи offline, до блокировки по связи
    bool isValid{true};
    bool disableAmountOverflow{
        false}; /// Не допускать появление сдачи путем отбраковки купюр сверх лимита.
    EEventType::Enum penetrationEventLevel{EEventType::OK};

    // Printer
    bool printFailedReceipts{true}; /// Распечатывать не напечатанные фискальные чеки при инкассации
    bool random_ReceiptsID{false};  /// Номера чеков в рандомном порядке
    QTime autoZReportTime;          /// Автоматическое закрытие смены ККТ в определенное время
    bool enableBlankFiscalData{false}; /// Разрешать печатать фискальные чеки без фискальных данных

    SCommonSettings() : minPar(10), {
        blockOn << ValidatorError << PrinterError << CardReaderError;
    }

    void setBlockOn(BlockReason aReason, bool aBlock) {
        if (aBlock) {
            blockOn << aReason;
        } else {
            blockOn.remove(aReason);
        }
    }

    [[nodiscard]] bool blockOn(BlockReason aReason) const { return blockOn.contains(aReason); }
};

//----------------------------------------------------------------------------
struct SMonitoringSettings {
    QUrl url;
    QUrl restUrl;
    int heartbeatTimeout{10};
    int restCheckTimeout{30};
    int restLimit{0};

    QStringList cleanupItems;
    QStringList cleanupExclude;

    SMonitoringSettings() : {}

    [[nodiscard]] bool isBlockByAccountBalance() const {
        return restLimit > 0 && restUrl.isValid();
    }
};

//----------------------------------------------------------------------------
struct SServiceMenuPasswords {
    SServiceMenuPasswords() : {}

    int operatorId{0};
    QString phone;
    QMap<QString, QString> passwords;
};

//----------------------------------------------------------------------------
struct SServiceMenuSettings {
    bool allowAnyKeyPair{false}; // Разрешаем генерировать ключи с произвольным номером пары

    SServiceMenuSettings() : {}
};

//----------------------------------------------------------------------------
namespace CServiceMenuPasswords {
extern const char Service[];
extern const char Screen[];
extern const char Collection[];
extern const char Technician[];
} // namespace CServiceMenuPasswords

//----------------------------------------------------------------------------
class TerminalSettings : public ISettingsAdapter, public ILogable {
public:
    TerminalSettings(TPtree &aProperties);
    ~TerminalSettings() override;

    /// Инициализация настроек.
    void initialize();

    /// Проверка настроек.
    [[nodiscard]] bool isValid() const override;

    /// Получить имя адаптера.
    static QString getAdapterName();

    /// Получить настройки соединения.
    [[nodiscard]] SConnection getConnection() const;

    /// Сохранить настройки соединения.
    void setConnection(const SConnection &aConnection);

    /// Получить список хостов для проверки соединения.
    [[nodiscard]] QList<IConnection::CheckUrl> getCheckHosts() const;

    /// Получить настройки БД.
    [[nodiscard]] SDatabaseSettings getDatabaseSettings() const;

    /// Получить список устройств.
    [[nodiscard]] QStringList getDeviceList() const;

    /// Получить имя конфигурации принтера, настроенного для печати заданного типа чеков.
    QString getPrinterForReceipt(const QString &aReceiptType);

    /// Сохранить список устройств.
    void setDeviceList(const QStringList &aHardware);

    /// Мониторинг.
    [[nodiscard]] SMonitoringSettings getMonitoringSettings() const;

    /// Ключи.
    [[nodiscard]] QMap<int, SKeySettings> getKeys() const;
    void setKey(const SKeySettings &aKey, bool aReplaceIfExists = true);
    void cleanKeys();

    /// Валюта.
    [[nodiscard]] SCurrencySettings getCurrencySettings() const;

    /// URL для генерации ключей.
    [[nodiscard]] QString getKeygenURL() const;

    /// URL для генерации письма с электронной копией чека
    [[nodiscard]] QString getReceiptMailURL() const;

    /// URL для отправки feedback в Хумо
    [[nodiscard]] QString getFeedbackURL() const;

    /// Возвращает мапу из типов процессинга и разрешенных ChargeProvider для каждого
    /// соответственно.
    [[nodiscard]] QVariantMap getChargeProviderAccess() const;

    /// Общие настройки.
    [[nodiscard]] virtual SCommonSettings getCommonSettings() const;

    /// Пароли для доступа к сервисному меню.
    [[nodiscard]] SServiceMenuPasswords getServiceMenuPasswords() const;

    /// Обобщенные настройки для сервисного меню.
    [[nodiscard]] SServiceMenuSettings getServiceMenuSettings() const;

    /// Пути к данным.
    [[nodiscard]] SAppEnvironment getAppEnvironment() const;
    void setAppEnvironment(const SAppEnvironment &aEnv);

    /// Максимальный размер log файлов
    [[nodiscard]] int getLogsMaxSize() const;

    /// Updater urls
    [[nodiscard]] QStringList getUpdaterUrls() const;

    /// Реклама.
    [[nodiscard]] QString getAdProfile() const;

    /// возвращает валидное время если необходимо проверять обновление ПО терминала
    [[nodiscard]] QTime autoUpdate() const;

    /// возвращает диапазон времени с какого по какое мы держим монитор в выключенном состоянии
    [[nodiscard]] QString energySave() const;

    /// Получить список критичных ошибок процессинга
    [[nodiscard]] const QSet<int> &getCriticalErrors() const;

private:
    Q_DISABLE_COPY(TerminalSettings)

    TPtree &m_properties;

    /// Список критичных ошибок процессинга
    QSet<int> m_criticalErrors;
};

} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
