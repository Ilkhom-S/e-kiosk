/* @file Приемник купюр по порту. */

#pragma once

#include <QtCore/QSharedPointer>

#include <Hardware/CashAcceptors/CashAcceptorBase.h>
#include <Hardware/Common/DeviceCodeSpecification.h>
#include <Hardware/Common/WaitingData.h>

/// Ожидание после резета
namespace EResetWaiting {
enum Enum { No = 0, Available, Full };
} // namespace EResetWaiting

//--------------------------------------------------------------------------------
namespace CPortCashAcceptor {
/// Ожидание завершения выполнения функционала для фильтрации/отправки накопленных статусов.
const SWaitingData RestoreStatusesWaiting = SWaitingData(1, 10 * 1000);
} // namespace CPortCashAcceptor

//--------------------------------------------------------------------------------
template <class T> class PortCashAcceptor : public CashAcceptorBase<T> {
public:
    PortCashAcceptor();

    /// Активировать/деактивировать приём с учетом отложенного выполнения.
    virtual bool setEnable(bool aEnabled);

    /// Можно ли обновлять прошивку.
    virtual bool canUpdateFirmware();

    /// Обновить прошивку.
    virtual void updateFirmware(const QByteArray &aBuffer);

protected:
    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Запросить и сохранить параметры устройства.
    virtual void processDeviceData() {}

    /// Получить и обработать статус.
    virtual bool processStatus(TStatusCodes &aStatusCodes);

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Получить статус.
    virtual bool checkStatus(QByteArray & /*aAnswer*/) { return false; }

    /// Получить статусы.
    typedef QList<QByteArray> TStatusData;
    virtual bool checkStatuses(TStatusData &aData);

    /// Обновить прошивку.
    virtual bool performUpdateFirmware(const QByteArray &aBuffer);

    /// Фоновая логика при появлении определенных состояний устройства.
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection);

    /// Извещает верхнюю логику о завершении отключения устройства приема денег.
    virtual void onSendDisabled();

    /// Восстановление статусов для отправки наверх после отключения polling.
    virtual void restoreStatuses();

    /// Выполняет физические действия по включению/выключению устройства.
    virtual void processEnable(bool aEnabled);

    /// Установка параметров по умолчанию.
    virtual bool setDefaultParameters() { return true; }

    /// Ждет определенного состояния купюроприёмника.
    bool processAndWait(const TBoolMethod &aCommand,
                        TBoolMethod aCondition,
                        int aTimeout,
                        bool aNeedReset = true,
                        bool aRestartPolling = true,
                        TBoolMethod aErrorCondition = TBoolMethod());

    /// Сброс.
    bool reset(bool aWait);

    /// Локальный сброс.
    virtual bool processReset() { return false; }

    /// Завершен ли сброс.
    bool isResetCompleted(bool aWait);

    /// Применить таблицу номиналов.
    virtual bool applyParTable() { return true; }

    /// Проверка возможности применения буфера статусов.
    virtual bool canApplyStatusBuffer();

    /// Завершение инициализации.
    virtual void finalizeInitialization();

    /// Изменение режима приема денег.
    virtual bool enableMoneyAcceptingMode(bool /*aEnabled*/) { return false; }

    /// Повтор изменения режима приема денег.
    bool reenableMoneyAcceptingMode();

    /// Извещает верхнюю логику о завершении включения устройства приема денег.
    virtual void sendEnabled();

    /// Установить начальные параметры.
    virtual void setInitialData();

    /// Установить эскроу-данные.
    virtual bool setLastPar(const QByteArray &aAnswer);

    /// Получить интервал polling в режиме приема денег.
    int getPollingInterval(bool aEnabled);

    /// Признак контроля отключения купюроприёмника.
    bool m_CheckDisable;

    /// Устройству необходим reset на идентификации.
    bool m_ResetOnIdentification;

    /// Номер эскроу-байта в распакованной посылке.
    int m_EscrowPosition;

    /// Описание для КОДОВ протоколов.
    typedef QSharedPointer<IDeviceCodeSpecification> PDeviceCodeSpecification;
    PDeviceCodeSpecification m_DeviceCodeSpecification;

    /// Последние девайс-коды устройства.
    typedef QSet<QByteArray> TDeviceCodeBuffers;
    TDeviceCodeBuffers m_DeviceCodeBuffers;

    /// Приходит ли в событии Stacked информация о купюре.
    bool m_ParInStacked;

    /// Интервал опроса (время между посылками запроса статуса), девайс выключен на прием денег.
    int m_PollingIntervalDisabled;

    /// Интервал опроса, девайс включен на прием денег.
    int m_PollingIntervalEnabled;

    /// Ждать доступности после reset.
    EResetWaiting::Enum m_ResetWaiting;

    /// Возможна ли перепрошивка.
    bool m_Updatable;

    /// Ждать окончания reset даже если не прошла команда.
    // TODO: убрать, поправив логику в месте использования
    bool m_ForceWaitResetCompleting;
};

//--------------------------------------------------------------------------------
