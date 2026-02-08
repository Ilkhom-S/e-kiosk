/* @file Реализация интерфейсов для работы с БД. */

#pragma once

#include <QtCore/QRecursiveMutex>

#include <Common/ILog.h>

#include "DatabaseUtils/IDatabaseUtils.h"
#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "DatabaseUtils/IPaymentDatabaseUtils.h"

//---------------------------------------------------------------------------
class IDatabaseProxy;
class IApplication;

//---------------------------------------------------------------------------
class DatabaseUtils : public IDatabaseUtils,
                      public IHardwareDatabaseUtils,
                      public IPaymentDatabaseUtils {
public:
    DatabaseUtils(IDatabaseProxy &aProxy, IApplication *aApplication);
    virtual ~DatabaseUtils() override;

    /// Инициализация.
    virtual bool initialize() override;

#pragma region IDatabaseUtils interface

    /// Подготавливает к выполнению запрос.
    virtual IDatabaseQuery *prepareQuery(const QString &aQuery) override;

    /// Thread защищённое выполнение произвольного запроса.
    virtual bool execQuery(IDatabaseQuery *aQuery) override;

    /// Освобождение памяти из-под запроса.
    virtual void releaseQuery(IDatabaseQuery *aQuery) override;

#pragma endregion

#pragma region IHardwareDatabaseUtils interface

    /// Возвращает список параметров устройства по имени конфигурации устройства.
    virtual bool getDeviceParams(const QString &aDeviceConfigName,
                                 QVariantMap &aParameters) override;

    /// Возвращает true, если параметр aName для устройства с именем aDeviceName и типом aType
    /// существует.
    virtual bool isDeviceParam_Exists(const QString &aDeviceConfigName) override;

    /// Возвращает значение конкретного параметра из список параметров устройства по имени и типу.
    virtual QVariant getDeviceParam(const QString &aDeviceConfigName,
                                    const QString &aParamName) override;

    /// Добавить определенный параметр устройства по имени и типу.
    virtual bool setDeviceParam(const QString &aDeviceConfigName,
                                const QString &aParamName,
                                const QVariant &aParamValue) override;

    /// Предикат, определяющий наличие устройства по его имени и типу.
    virtual bool hasDevice(const QString &aDeviceConfigName) override;

    /// Добавить новый девайс.
    virtual bool addDevice(const QString &aDeviceConfigName) override;

    /// Удаление устройств aDevice с типом aType.
    virtual bool removeDeviceParams(const QString &aDeviceConfigName) override;

    /// Удаляем отправленные статусы устройств.
    virtual bool cleanDevicesStatuses() override;

    /// Удалить из базы неиспользуемые конфигурации устройств
    virtual void removeUnknownDevice(const QStringList &aCurrentDevicesList) override;

    /// Вставить новый статус девайсов.
    virtual bool addDeviceStatus(const QString &aDeviceConfigName,
                                 SDK::Driver::EWarningLevel::Enum aErrorLevel,
                                 const QString &aStatusString) override;

#pragma endregion

#pragma region IPaymentDatabaseUtils interface

    /// Создание пустой платёжной записи в базе.
    virtual qint64 createDummyPayment() override;

    /// Возвращает идентификатор платежа по начальной сессии.
    virtual qint64 getPaymentByInitialSession(const QString &aInitialSession) override;

    /// Возвращает список параметров для платежа с идентификатором aId.
    virtual TPaymentParameters getPaymentParameters(qint64 aId) override;

    /// Возвращает список параметров для платежа с идентификаторами aIds.
    virtual QMap<qint64, TPaymentParameters>
    getPaymentParameters(const QList<qint64> &aIds) override;

    /// Сохраняет платёж в базе. Опционально можно указать подпись.
    virtual bool savePayment(SDK::PaymentProcessor::IPayment *aPayment,
                             const QString &aSignature) override;

    /// Удаляет платёж из базы.
    virtual void removePayment(qint64 aPayment) override;

    /// Временно приостановить обработку платежа.
    virtual bool suspendPayment(qint64 aPayment, int aMinutes) override;

    /// Добавляет сумму aAmount к платежу с идентификатором aPayment.
    virtual bool addPaymentNote(qint64 aPayment,
                                const SDK::PaymentProcessor::SNote &aNote) override;
    virtual bool addPaymentNote(qint64 aPayment,
                                const QList<SDK::PaymentProcessor::SNote> &aNotes) override;

    /// Добавим в БД информацию о выданных купюрах
    virtual bool addChangeNote(const QString &aSession,
                               const QList<SDK::PaymentProcessor::SNote> &aNotes) override;

    /// Получить информацию по всем купюроприемникам в контексте платежа.
    virtual QList<SDK::PaymentProcessor::SNote> getPaymentNotes(qint64 aPayment) override;

    /// Возвращает список платежей, ожидающих проведения.
    virtual QList<qint64> getPaymentQueue() override;

    /// Возвращает краткую информацию по платежам и купюрам с последней инкассации.
    virtual SDK::PaymentProcessor::SBalance getBalance() override;

    /// Получить список платежей определенного статуса. В случае пустого списка статусов - получим
    /// все платежи из базы
    virtual QList<qint64>
    getPayments(const QSet<SDK::PaymentProcessor::EPaymentStatus::Enum> &aStates) override;

    /// Поиск платежа по номеру/счету
    virtual QList<qint64> findPayments(const QDate &aDate, const QString &aPhoneNumber) override;

    /// Выполняет инкассацию.
    virtual SDK::PaymentProcessor::SEncashment
    perform_Encashment(const QVariantMap &aParameters) override;

    /// Возвращает последнюю выполненную инкассацию
    virtual QList<SDK::PaymentProcessor::SEncashment> getLastEncashments(int aCount) override;

    /// Выполняет архивацию устаревших платежей.
    virtual bool backupOldPayments() override;

    /// Получить кол-во платежей по каждому использованному провайдеру
    virtual QMap<qint64, quint32> getStatistic() const override;

#pragma endregion

private:
    IApplication *m_Application;
    IDatabaseProxy &m_Database;
    ILog *m_Log;
    ILog *m_PaymentLog;
    QRecursiveMutex m_AccessMutex;

private:
    /// Заполняет отчет инкассации о платежах
    void fillEncashmentReport(SDK::PaymentProcessor::SEncashment &aEncashment);

private:
    /// Возвращает количество таблиц в базе
    int databaseTableCount() const;

    /// возвращает db_patch терминала
    int databasePatch() const;

    /// Выполнить обновление базы
    bool updateDatabase(const QString &aSqlScriptName);
};

//---------------------------------------------------------------------------
