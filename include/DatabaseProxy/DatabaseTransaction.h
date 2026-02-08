/* @file Класс RAII для транзакций абстрактного провайдера СУБД. */

#pragma once

#include "IDatabaseProxy.h"

//---------------------------------------------------------------------------
class DatabaseTransaction {
    IDatabaseProxy *m_Proxy;
    bool m_TransactionOpened;

public:
    /// Конструктор. Начинает транзакцию.
    DatabaseTransaction(IDatabaseProxy *aProxy) : m_Proxy(aProxy), m_TransactionOpened(false) {
        begin();
    }

    /// Деструктор. Откатывает транзакцию.
    ~DatabaseTransaction() {
        rollback();

        m_TransactionOpened = false;
        m_Proxy = nullptr;
    }

    /// Проверяет, открыта ли транзакция.
    operator bool() const { return m_TransactionOpened; }

    /// Начинает транзакцию.
    bool begin() {
        if (m_Proxy && !m_TransactionOpened) {
            m_TransactionOpened = m_Proxy->transaction();
        }

        return m_TransactionOpened;
    }

    /// Фиксирует транзакцию.
    bool commit() {
        bool result = false;

        if (m_Proxy && m_TransactionOpened) {
            result = m_Proxy->commit();
            m_TransactionOpened = false;
        }

        return result;
    }

    /// Откатывает транзакцию.
    bool rollback() {
        bool result = false;

        if (m_Proxy && m_TransactionOpened) {
            result = m_Proxy->rollback();
            m_TransactionOpened = false;
        }

        return result;
    }
};

//---------------------------------------------------------------------------
