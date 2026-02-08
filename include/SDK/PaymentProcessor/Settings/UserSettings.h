/* @file Настройки пользователя. */

#pragma once

#include <Common/ILogable.h>
#include <Common/PropertyTree.h>

#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
class UserSettings : public ISettingsAdapter, public ILogable {
public:
    explicit UserSettings(TPtree &aProperties);
    ~UserSettings() override;

    /// Валидация данных.
    virtual bool isValid() const override;

    /// Получить имя адаптера.
    static QString getAdapterName();

    /// Настройка для мониторинга: выгружать все платежи, игнорируя статус
    bool reportAllPayments() const;

    /// Требовать ввода номера стекера в момент инкассации
    bool useStackerID() const;

private:
    TPtree &m_Properties;

private:
    Q_DISABLE_COPY(UserSettings);
};

} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
