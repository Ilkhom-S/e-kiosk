/* @file Настройки расширений. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include <Common/ILogable.h>
#include <Common/PropertyTree.h>

#include <SDK/PaymentProcessor/Connection/Connection.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Settings/Range.h>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
typedef QMap<QString, QString> TStringMap;

//----------------------------------------------------------------------------
class ExtensionsSettings : public ISettingsAdapter, public ILogable {
public:
    ExtensionsSettings(TPtree &aProperties);
    virtual ~ExtensionsSettings();

    /// Валидация данных.
    virtual bool isValid() const;

    /// Получить имя адаптера.
    static QString getAdapterName();

    /// Получить настройки расширения, в случае отсутствия настроек вернет пустой QStringMap
    TStringMap getSettings(const QString &aExtensionName) const;

private:
    TPtree &m_Properties;
    QMap<QString, TStringMap> m_ExtensionSettings;

private:
    Q_DISABLE_COPY(ExtensionsSettings);
};

} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
