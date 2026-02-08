/* @file Прокси класс для работы с платежами в скриптах. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <QtCore/QWeakPointer>

#include <SDK/PaymentProcessor/Core/Encashment.h>
#include <SDK/PaymentProcessor/Payment/Amount.h>
#include <SDK/PaymentProcessor/Scripting/ScriptArray.h>
#include <SDK/PaymentProcessor/Settings/Provider.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

namespace SDK {
namespace PaymentProcessor {
class ICore;
class IPaymentService;
class Directory;
class DealerSettings;

namespace Scripting {
class PaymentService;

//------------------------------------------------------------------------------
/// Элемент перечисления.
class Enum_Item : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString value READ getValue CONSTANT)
    Q_PROPERTY(QString id READ getID CONSTANT)

public:
    /// Конструктор.
    Enum_Item(const SProviderField::SEnum_Item &aItem, QObject *aParent)
        : QObject(aParent), m_Item(aItem) {}

private:
    /// Получить имя.
    QString getName() { return m_Item.title; }
    /// Получить значение.
    QString getValue() { return m_Item.value; }
    /// Получить ID.
    QString getID() { return m_Item.id; }

private:
    /// Элемент перечисления.
    SProviderField::SEnum_Item m_Item;
};

//------------------------------------------------------------------------------
/// Поле провайдера.
class ProviderField : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString type READ getType CONSTANT)
    Q_PROPERTY(QString id READ getId CONSTANT)

    Q_PROPERTY(QString keyboardType READ getKeyboardType CONSTANT)
    Q_PROPERTY(QString language READ getLanguage CONSTANT)
    Q_PROPERTY(QString letterCase READ getLetterCase CONSTANT)

    Q_PROPERTY(int minSize READ getMinSize CONSTANT)
    Q_PROPERTY(int maxSize READ getMaxSize CONSTANT)

    Q_PROPERTY(bool isRequired READ isRequired CONSTANT)

    Q_PROPERTY(QString title READ getTitle CONSTANT)
    Q_PROPERTY(QString comment READ getComment CONSTANT)
    Q_PROPERTY(QString extendedComment READ getExtendedComment CONSTANT)

    Q_PROPERTY(QString mask READ getMask CONSTANT)
    Q_PROPERTY(QString format READ getFormat CONSTANT)
    Q_PROPERTY(bool isPassword READ isPassword CONSTANT)

    Q_PROPERTY(QString behavior READ getBehavior CONSTANT)
    Q_PROPERTY(QString defaultValue READ getDefaultValue CONSTANT)
    Q_PROPERTY(QObject *enum_Items READ getEnum_Items CONSTANT)

    Q_PROPERTY(QString url READ getUrl CONSTANT)
    Q_PROPERTY(QString html READ getHtml CONSTANT)
    Q_PROPERTY(QString backButton READ getBackButton CONSTANT)
    Q_PROPERTY(QString forwardButton READ getForwardButton CONSTANT)
    Q_PROPERTY(QString dependency READ getDependency CONSTANT)

public:
    /// Конструктор.
    ProviderField(const SProviderField &aField, QObject *aParent = 0)
        : QObject(aParent), m_Field(aField) {}

private:
    /// Получить тип.
    QString getType() { return m_Field.type; }
    QString getId() { return m_Field.id; }

    QString getKeyboardType() { return m_Field.keyboardType; }
    QString getLanguage() { return m_Field.language; }
    QString getLetterCase() { return m_Field.letterCase; }

    int getMinSize() { return m_Field.minSize; }
    int getMaxSize() { return m_Field.maxSize; }

    bool isRequired() { return m_Field.isRequired; }

    QString getTitle() { return m_Field.title; }
    QString getComment() { return m_Field.comment; }
    QString getExtendedComment() { return m_Field.extendedComment; }

    QString getMask() { return m_Field.mask; }
    QString getFormat() { return m_Field.format; }
    bool isPassword() { return m_Field.isPassword; }

    QString getBehavior() { return m_Field.behavior; }
    QString getDefaultValue() { return m_Field.defaultValue; }
    ScriptArray *getEnum_Items();

    QString getUrl() { return m_Field.url; }
    QString getHtml() { return m_Field.html; }
    QString getBackButton() { return m_Field.backButton; }
    QString getForwardButton() { return m_Field.forwardButton; }

    QString getDependency() { return m_Field.dependency; }

    SProviderField m_Field;
};

//------------------------------------------------------------------------------
class Provider : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString id READ getID CONSTANT)
    Q_PROPERTY(QString gateway READ getCID CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    Q_PROPERTY(QString processorType READ getProcessorType CONSTANT)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString comment READ getComment CONSTANT)
    Q_PROPERTY(QString minLimit READ getMinLimit CONSTANT)
    Q_PROPERTY(QString maxLimit READ getMaxLimit CONSTANT)
    Q_PROPERTY(QString system_Limit READ getSystem_Limit CONSTANT)
    Q_PROPERTY(QVariant fields READ getFields CONSTANT)
    Q_PROPERTY(bool skipCheck READ getSkipCheck CONSTANT)
    Q_PROPERTY(bool payOnline READ getPayOnline CONSTANT)
    Q_PROPERTY(bool askForRetry READ getAskForRetry CONSTANT)
    Q_PROPERTY(bool requirePrinter READ getRequirePrinter CONSTANT)
    Q_PROPERTY(bool showAddInfo READ getShowAddInfo CONSTANT)
    Q_PROPERTY(QString clientCard READ getClientCard CONSTANT)
    Q_PROPERTY(QString externalDataHandler READ getExternalDataHandler CONSTANT)
    Q_PROPERTY(QVariantMap receipts READ getReceipts CONSTANT)
    Q_PROPERTY(QVariantMap receiptParameters READ getReceiptParameters CONSTANT)

public:
    Provider(SProvider aProvider, QObject *aParent);

public slots:
    bool isNull() const { return m_Provider.id == -1 || m_Provider.fields.isEmpty(); }

    /// Проверить согласование проверки номера и лимитов, получаемых с сервера
    bool isCheckStepSettingsOK() const;

    QString applySecurityFilter(const QString aId,
                                const QString &aValueRaw,
                                const QString &aValueDisplay) const;

    static QString xmlFields2Json(const QString &aXmlFields);

private:
    QString getID() const { return QString::number(m_Provider.id); }
    QString getCID() const { return QString::number(m_Provider.cid); }
    QString getType() const { return m_Provider.type; }
    QString getProcessorType() const { return m_Provider.processor.type; }
    QString getName() const { return m_Provider.name; }
    QString getComment() const { return m_Provider.comment; }
    QString getMinLimit() const { return m_Provider.limits.min; }
    QString getMaxLimit() const { return m_Provider.limits.max; }
    QString getSystem_Limit() const { return m_Provider.limits.system; }
    QVariant getFields();
    bool getSkipCheck() const { return m_Provider.processor.skipCheck; }
    bool getPayOnline() const { return m_Provider.processor.payOnline; }
    bool getAskForRetry() const { return m_Provider.processor.askForRetry; }
    bool getRequirePrinter() const { return m_Provider.processor.requirePrinter; }
    bool getShowAddInfo() const { return m_Provider.processor.showAddInfo; }
    QString getClientCard() const { return QString::number(m_Provider.processor.clientCard); }
    QString getExternalDataHandler() const { return m_Provider.externalDataHandler; }
    QVariantMap getReceipts() const { return m_Provider.receipts; }
    QVariantMap getReceiptParameters() const { return m_Provider.receiptParameters; }

private:
    SProvider m_Provider;
    QMap<QString, QObjectList> m_Fields;
};

#define PROPERTY_GET(type, name, holder)                                                           \
    type name() const {                                                                            \
        return holder.name;                                                                        \
    }

//------------------------------------------------------------------------------
class Note : public QObject {
    Q_OBJECT

    Q_PROPERTY(double nominal READ nominal CONSTANT)
    Q_PROPERTY(QString serial READ serial CONSTANT)
    Q_PROPERTY(int currency READ currency CONSTANT)
    Q_PROPERTY(int type READ type CONSTANT)

public:
    Note(const SNote &aNote, QObject *aParent) : QObject(aParent), m_Note(aNote) {}

private:
    PROPERTY_GET(double, nominal, m_Note)
    PROPERTY_GET(QString, serial, m_Note)
    PROPERTY_GET(int, currency, m_Note)
    PROPERTY_GET(int, type, m_Note)

private:
    SNote m_Note;
};

//------------------------------------------------------------------------------
class IProviderProvider {
public:
    virtual ~IProviderProvider() {}

public:
    virtual QList<qint64> getId() const = 0;
    virtual SProvider getProvider(qint64 aProviderId) = 0;
};

//------------------------------------------------------------------------------
class PaymentService : public QObject {
    Q_OBJECT
    Q_ENUMS(EProcessResult)

public:
    /// Результат попытки проведения платежа.
    enum EProcessResult {
        OK = 0,       /// Ошибок нет.
        LowMoney = 1, /// Недостаточно средств для проведения платежа.
        OfflineIsNotSupported =
            2,         /// Попытка провести в оффлайне платёж, который поддерживает только онлайн.
        BadPayment = 3 /// Платеж не найден или не может быть проведён
    };

    PaymentService(ICore *aCore);

public slots:
    /// Создание платежа по номеру оператора aProvider. Возвращает номер платежа или код ошибки
    /// (меньше 0).
    qint64 create(qint64 aProvider);

    /// Возвращает номер активного платежа.
    qint64 getActivePaymentID();

    /// Возвращает номер последнего платежа.
    qint64 getLastPaymentID();

    /// Возвращает описание оператора активного платежа.
    QObject *getProvider();

    /// Возвращает описание реального оператора активного платежа.
    QObject *getMNPProvider();

    /// Возвращает описание оператора с идентификатором aID.
    QObject *getProvider(qint64 aID);

    /// Возвращает описание первого попавшегося оператора с номером шлюза aСID.
    QObject *getProviderByGateway(qint64 aCID);

    /// Возвращает провайдеров для данного номера в соответствии с номерной ёмкостью.
    QObject *getProviderForNumber(qint64 aNumber);

    /// Возвращает значение поля aName для платежа aPayment.
    QVariant getParameter(const QString &aName);

    /// Возвращает список полей платежа.
    QVariantMap getParameters();

    /// Вычислить размер комиссии для активного платежа, без записи результата в параметры платежа
    QVariantMap calculateCommission(const QVariantMap &aParameters);

    /// Вычислить для активного платежа, без записи результата в параметры платежа, лимиты комиссию
    /// для произвольной суммы
    QVariantMap calculateLimits(const QString &aAmount, bool aFixedAmount = false);

    /// Возвращает список купюр текущего платежа.
    QObject *getPaymentNotes();

    /// Возвращает список не инкассированных купюр.
    QObject *getBalanceNotes();

    /// Возвращает список купюр из последней инкассации.
    QObject *getLastEncashmentNotes();

    /// Обновляет указанное свойство у платежа.
    void setExternalParameter(const QString &aName, const QVariant &aValue);

    /// Возвращает имя алиаса, к которому привязан параметр из запроса
    QString findAliasFrom_Request(const QString &aParamName,
                                  const QString &aRequestName = QString("CHECK"));

    /// Обновляет указанное свойство у платежа.
    void setParameter(const QString &aName, const QVariant &aValue, bool aCrypted = false);

    /// Обновление нескольких свойств платежа.
    void setParameters(const QVariantMap &aParameters);

    /// Установить для провайдера лимиты платежа
    void updateLimits(qint64 aProviderId, double aExternalMin, double aExternalMax);

    /// Возвращает true, если платёж можно провести в оффлайне.
    bool canProcessOffline();

    /// Конвертировать активный платёж в новый процессинг
    bool convert(const QString &aTargetType);

    /// Проверка введённых данных.
    void check();

    /// Выполнение шага платежа в онлайне.
    void processStep(int aStep);

    /// Проведение платежа.
    EProcessResult process(bool aOnline);

    /// Остановка платежа с текстом ошибки
    bool stop(int aError, const QString &aErrorMessage);

    /// Отмена платежа.
    bool cancel();

    /// Сбросить состояние платёжной логики
    void reset();

    /// Получение суммы сдачи, доступной для использования.
    double getChangeAmount();

    /// Использование сдачи от предыдущего платежа.
    void useChange();

    /// Переместить всю сумму из платежа обратно в сдачу.
    void useChangeBack();

    /// Сброс счётчика наполненной сдачи.
    void resetChange();

public slots:
    /// Поиск списка платежей по номеру телефона/счета
    QStringList findPayments(const QDate &aDate, const QString &aPhoneNumber);

    /// Получение списка параметров платежа по его ID
    QVariantMap getParameters(qint64 aPaymentId);

    /// Возвращает статистику по кол-ву платежей
    QVariantMap getStatistic();

    void checkStatus();

public slots:
#pragma region Multistage
    /// Отправка запроса для перехода на следующий шаг
    void stepForward();

    /// Вернутся на шаг назад
    void stepBack();

    /// Получение ID текущего шага
    QString currentStep();

    /// Признак, что текущий шаг - последний
    bool isFinalStep();

    ///
    void setExternalCommissions(const QVariantList &aCommissions);

    ///
    void resetExternalCommissions();

public:
    /// Получение списка полей для интерфейса в многошаговом шлюзе
    bool loadFieldsForStep(TProviderFields &aFields);
#pragma endregion

    /// Включить возможность проведения платежей при отсутствии связи
    void setForcePayOffline(bool aForcePayOffline);

private slots:
    /// Получение сигнала об окончании проверки платежа
    void onStepCompleted(qint64 aPayment, int aStep, bool aError);

signals:
    /// Завершён шаг для платежа aPayment.
    void stepCompleted(qint64 aPayment, int aStep, bool aError);

    /// Обновлены суммы указанного платежа.
    void amountUpdated(qint64 aPayment);

    /// Обновление суммы сдачи
    void changeUpdated(double aAmount);

public:
    void addProviderProvider(const QWeakPointer<IProviderProvider> &aProvider);

private:
    ICore *m_Core;
    IPaymentService *m_PaymentService;
    SDK::PaymentProcessor::Directory *m_Directory;
    SDK::PaymentProcessor::DealerSettings *m_DealerSettings;
    SDK::PaymentProcessor::SCommonSettings m_CommonSettings;
    bool m_ForcePayOffline;

private:
    /// Получить список купюр из баланса
    QObject *notesFrom_Balance(const SDK::PaymentProcessor::SBalance &aBalance);

    /// Модифицируем провайдера в соответствии с настройками
    SProvider updateSkipCheckFlag(SProvider aProvider);

    /// Провайдер, которому меняли лимиты платежа из сценария
    qint64 m_ProviderWithExternalLimits;

private:
    QList<SDK::PaymentProcessor::SBlockByNote> m_BlockNotesList;
    typedef QList<QWeakPointer<IProviderProvider>> TProviderProvider;
    TProviderProvider m_ProviderProviders;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
