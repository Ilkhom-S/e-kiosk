/* @file Класс-фильтр содержимого полей ввода пользователя. */

#pragma once

#include <QtCore/QRegularExpression>

#include <SDK/PaymentProcessor/Settings/Provider.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
struct SProvider;

//------------------------------------------------------------------------------
class SecurityFilter {
public:
    SecurityFilter(const SProvider &aProvider, SProviderField::SecuritySubsystem aSubsustem);

    /// Проверка - подлежит ли поле маскированию
    bool haveFilter(const QString &aParameterName) const;

    /// Выполняет фильтрацию
    QString apply(const QString &aParameterName, const QString &aValue) const;

private:
    /// Получить актуальный regexp для маскирования
    QRegularExpression getMask(const QString &aParameterName) const;

private:
    const SProvider &m_Provider;
    SProviderField::SecuritySubsystem m_Subsystem;

    Q_DISABLE_COPY(SecurityFilter)
};

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
