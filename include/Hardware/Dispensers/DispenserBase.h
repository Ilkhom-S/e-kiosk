/* @file Базовый диспенсер. */
/* @details Терминология:
        item - то, что выдает диспенсер
        unit - кассета и т.п., куда складываются итемы
*/

#pragma once

#include <SDK/Drivers/Dispenser/DispenserStatus.h>

#include <Hardware/Dispensers/DispenserData.h>
#include <Hardware/Dispensers/ProtoDispenser.h>

namespace CDispensers {
/// Интервал поллинга в режиме простоя.
const int IdlingPollingInterval = 2000;
} // namespace CDispensers

//--------------------------------------------------------------------------------
template <class T> class DispenserBase : public T {
public:
    DispenserBase();

    /// Готов ли к работе (инициализировался успешно, ошибок нет).
    virtual bool isDeviceReady(int aUnit = -1);

    /// Установить конфигурацию кассет.
    virtual void setUnitList(const SDK::Driver::TUnitData &aUnitData);

    /// Получить кол-во кассет.
    virtual int units();

protected:
    /// Запросить и сохранить параметры устройства.
    virtual void processDeviceData() {}

    /// Применить конфигурацию кассет.
    virtual void applyUnitList();

    /// Выдать.
    virtual void dispense(int aUnit, int aItems);

    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Сброс.
    virtual bool reset();

    /// Анализирует коды статусов устройства и фильтрует несуществующие статусы для нижней логики.
    virtual void cleanStatusCodes(TStatusCodes &aStatusCodes);

    /// Отправить статус-коды.
    virtual void emitStatusCodes(TStatusCollection &aStatusCollection,
                                 int aExtendedStatus = SDK::Driver::EStatus::Actual);

    /// Проверить статус кассеты.
    virtual void checkUnitStatus(TStatusCodes &aStatusCodes, int aUnit);

    /// Фоновая логика при появлении определенных состояний устройства.
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection);

    /// Упорядочить конфигурацию кассет в соответствии с новым их количеством.
    void adjustUnitList(bool aConfigData);

    /// Послать сигнал об опустошении кассеты.
    void emitUnitEmpty(int aUnit, const QString &aLog = "");

    /// Послать сигнал о выдаче.
    void emitDispensed(int aUnit, int aItems, const QString &aLog = "");

    /// Кол-во кассет.
    int m_Units;

    /// Данные о количестве предметов в кассетах из конфигурации.
    SDK::Driver::TUnitData m_UnitConfigData;

    /// Данные о количестве предметов в кассетах.
    SDK::Driver::TUnitData m_UnitData;

    /// Необходимо сообщить количество кассет.
    bool m_NeedGetUnits;

    /// Ошибка установки содержимого кассет.
    bool m_UnitError;

    /// Последние девайс-коды для логгирования смены статуса.
    QByteArray m_LastDeviceStatusCodes;

    /// Резет возможен.
    bool m_ResetIsPossible;
};

//--------------------------------------------------------------------------------
