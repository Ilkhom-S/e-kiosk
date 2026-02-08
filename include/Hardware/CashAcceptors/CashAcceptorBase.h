/* @file Базовый класс приемника купюр. */

#pragma once

#include <SDK/Drivers/CashAcceptor/CashAcceptorStatus.h>

#include "CashAcceptorStatusData.h"
#include "CurrencyErrors.h"
#include "Hardware/CashAcceptors/BillTable.h"
#include "Hardware/CashAcceptors/CashAcceptorBaseConstants.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Common/DeviceBase.h"

typedef QList<int> TStatusCodesHistory;
typedef QList<TStatusCodesHistory> TStatusCodesHistoryList;

//--------------------------------------------------------------------------------
namespace CCashAcceptor {
typedef StatusCache<SDK::Driver::ECashAcceptorStatus::Enum> TStatuses;

/// Качественный описатель последнего состояния.
struct SStatusSpecification {
    /// Уровень тревожности статуса.
    SDK::Driver::EWarningLevel::Enum warningLevel;

    /// Буфер статусов, сопряженных с статус-кодами.
    TStatuses statuses;

    SStatusSpecification() : warningLevel(SDK::Driver::EWarningLevel::OK) {}

    bool operator==(const SStatusSpecification &aStatusSpecification) const {
        return (statuses == aStatusSpecification.statuses);
    }

    bool operator!=(const SStatusSpecification &aStatusSpecification) const {
        return !operator==(aStatusSpecification);
    }
};

/// История актуальных отправленных статусов.
typedef HistoryList<SStatusSpecification> TStatusHistory;
} // namespace CCashAcceptor

typedef QList<SDK::Driver::SPar> TPars;

//--------------------------------------------------------------------------------
template <class T> class CashAcceptorBase : public T {
public:
    CashAcceptorBase();

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова
    /// initialize().
    virtual bool release();

    /// Готов ли к работе (инициализировался успешно, ошибок нет).
    virtual bool isDeviceReady();

    /// Установить таблицу номиналов.
    virtual void setParList(const SDK::Driver::TParList &aParList);

    /// Получить таблицу номиналов.
    virtual SDK::Driver::TParList getParList();

protected:
    /// Устанавливает запрещения списка номиналов.
    virtual void employParList();

    /// Получение и обработка списка номиналов.
    ECurrencyError::Enum processParTable();

    /// Загрузка таблицы номиналов из устройства.
    virtual bool loadParTable() { return false; }

    /// Анализирует коды статусов устройства и фильтрует несуществующие статусы для нижней логики.
    virtual void cleanStatusCodes(TStatusCodes &aStatusCodes);

    /// Анализирует коды статусов кастомных устройств и фильтрует несуществующие статусы для нижней
    /// логики.
    virtual void cleanSpecificStatusCodes(TStatusCodes & /*aStatusCodes*/) {}

    /// Ищет с конца совпадение истории статусов с куском истории статусов.
    bool isStatusCollectionConformed(const TStatusCodesHistory &aHistory);

    /// Заменяет совпадающие с куском истории статусов статус-коды.
    void
    replaceConformedStatusCodes(TStatusCodes &aStatusCodes, int aStatusCodeFrom, int aStatusCodeTo);

    /// Отправка статусов.
    virtual void sendStatuses(const TStatusCollection &aNewStatusCollection,
                              const TStatusCollection &aOldStatusCollection);

    /// Сохранение последних статусов.
    void
    saveStatuses(const CCashAcceptor::TStatuses &aStatuses,
                 SDK::Driver::ECashAcceptorStatus::Enum aTargetStatus,
                 const CCashAcceptor::TStatusSet aSourceStatuses = CCashAcceptor::TStatusSet());

    /// Получение последних статусов.
    CCashAcceptor::TStatuses getLastStatuses(int aLevel = 1) const;

    /// Получение признака возможности отключения купюроприёмника.
    bool canDisable() const;

    /// Включен на прием купюр?
    bool isEnabled(const CCashAcceptor::TStatuses &aStatuses = CCashAcceptor::TStatuses()) const;

    /// Не включен на прием купюр?
    bool isNotEnabled() const;

    /// Отключен на прием купюр?
    bool isDisabled(const CCashAcceptor::TStatuses &aStatuses = CCashAcceptor::TStatuses()) const;

    /// Не отключен на прием купюр?
    bool isNotDisabled() const;

    /// Инициализируется?
    bool isInitialize() const;

    /// Доступен?
    bool isAvailable();

    /// Получение признака возможности возврата купюры.
    bool canReturning(bool aOnline);

    /// Получить долгоиграющие статус-коды.
    TStatusCodes getLongStatusCodes() const;

    /// Вывод в лог таблицы номиналов.
    virtual void logEnabledPars();

    /// Отправка статусов.
    void emitStatuses(CCashAcceptor::SStatusSpecification &aSpecification,
                      const CCashAcceptor::TStatusSet &aSet);

    /// Список номиналов.
    SDK::Driver::TParList m_ParList;

    /// История актуальных отправленных статусов.
    typedef HistoryList<CCashAcceptor::SStatusSpecification> TStatusHistory;
    TStatusHistory m_StatusHistory;

    /// Кэш последних статусов для бизнес-логики.
    CCashAcceptor::TStatuses m_Statuses;

    /// Логические ошибки работы с валютой.
    ECurrencyError::Enum m_CurrencyError;

    /// Купюры/монеты в эскроу/стекеде.
    typedef QList<SDK::Driver::SPar> TPars;
    TPars m_EscrowPars;

    /// Тип устройства приема денег.
    QString m_DeviceType;

    /// Мьютекс для блокировки запросов к списку номиналов.
    QMutex m_ParListMutex;

    /// Список включенных номиналов для реализаций без протоколов.
    CBillTable m_EscrowParTable;

    /// Готов ли к работе (инициализировался успешно, ошибок нет).
    bool m_Ready;
};