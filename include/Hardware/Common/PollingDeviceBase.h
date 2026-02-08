/* @file Базовый класс устройств с поллингом. */

#pragma once

#include <QtCore/QTimer>

#include "Hardware/Common/DeviceBase.h"

namespace CPollingDeviceBase {
/// Ожидание останова поллинга, [мс].
const SWaitingData StopWaiting = SWaitingData(1, 15 * 1000);
} // namespace CPollingDeviceBase

/// Список задач.
typedef QList<TVoidMethod> TTaskList;

//---------------------------------------------------------------------------
template <class T> class PollingDeviceBase : public DeviceBase<T> {
public:
    PollingDeviceBase();

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до initialize().
    virtual bool release();

    /// Завершение инициализации.
    virtual void finalizeInitialization();

    /// Фоновая логика при появлении определенных состояний устройства.
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection);

    /// Есть ли ошибка инициализации при фильтрации статусов.
    bool isInitializationError(TStatusCodes &aStatusCodes);

    /// Запуск поллинга.
    void startPolling(bool aNotWaitFirst = false);

    /// Останов поллинга.
    void stopPolling(bool aWait = true);

    /// Установить таймаут.
    void setPollingInterval(int aPollingInterval);

    /// Ожидание состояния или выполнения полла.
    bool waitCondition(TBoolMethod aCondition, const SWaitingData &aWaitingData);

    /// Запуск/останов поллинга.
    virtual void setPollingActive(bool aActive);

    /// Пере инициализация в рамках фоновой логики пост-поллинга.
    virtual void reInitialize();

    /// Останавливает функционал поллинга, возвращается в состояние до initialize().
    void releasePolling();

    /// Таймер для поллинга.
    QTimer m_Polling;

    /// Интервал поллинга, [мс].
    int m_PollingInterval;

    /// Поллинг активирован. QTimer::isActive() не всегда работает корректно.
    bool m_PollingActive;

    /// Список задач для выполнения после поллинга, если нет ошибок.
    TTaskList m_PPTaskList;

    /// Принудительно не ждать первого полла на инициализации.
    bool m_ForceNotWaitFirst;
};

//--------------------------------------------------------------------------------

#include <Hardware/Common/PollingDeviceBase.tpp>
