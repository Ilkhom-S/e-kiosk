/* @file Класс RAII для транзакций абстрактного провайдера СУБД. */

#pragma once

#include "IDatabaseProxy.h"

//---------------------------------------------------------------------------
class DatabaseTransaction {
    IDatabaseProxy *mProxy;
    bool mTransactionOpened;

  public:
    /// Конструктор. Начинает транзакцию.
    DatabaseTransaction(IDatabaseProxy *aProxy) : mProxy(aProxy), mTransactionOpened(false) {
        begin();
    }

    /// Деструктор. Откатывает транзакцию.
    ~DatabaseTransaction() {
        rollback();

        mTransactionOpened = false;
        mProxy = nullptr;
    }

    /// Проверяет, открыта ли транзакция.
    operator bool() const {
        return mTransactionOpened;
    }

    /// Начинает транзакцию.
    bool begin() {
        if (mProxy && !mTransactionOpened) {
            mTransactionOpened = mProxy->transaction();
        }

        return mTransactionOpened;
    }

    /// Фиксирует транзакцию.
    bool commit() {
        bool result = false;

        if (mProxy && mTransactionOpened) {
            result = mProxy->commit();
            mTransactionOpened = false;
        }

        return result;
    }

    /// Откатывает транзакцию.
    bool rollback() {
        bool result = false;

        if (mProxy && mTransactionOpened) {
            result = mProxy->rollback();
            mTransactionOpened = false;
        }

        return result;
    }
};

//---------------------------------------------------------------------------
