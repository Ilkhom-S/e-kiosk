/* @file Настройки расширений. */

#include <SDK/PaymentProcessor/Settings/ExtensionsSettings.h>
#include <SDK/PaymentProcessor/Settings/Provider.h>

#include <array>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
QString ExtensionsSettings::getAdapterName() {
    return CAdapterNames::Extensions;
}

//---------------------------------------------------------------------------
ExtensionsSettings::ExtensionsSettings(TPtree &aProperties)
    : m_Properties(aProperties.get_child(CAdapterNames::Extensions, aProperties)) {
    auto loadExtensions = [&](const char *aChildPropertyName) -> void {
        TPtree empty;
        SRange range;

        BOOST_FOREACH (const TPtree::value_type &record,
                       m_Properties.get_child(aChildPropertyName, empty)) {
            if (record.first == "<xmlattr>") {
                continue;
            }

            try {
                auto extensionName = record.second.get<QString>("<xmlattr>.name");

                QMap<QString, QString> extensionParams;
                BOOST_FOREACH (const TPtree::value_type &parameter, record.second) {
                    if (parameter.first == "<xmlattr>") {
                        continue;
                    }

                    extensionParams.insert(parameter.second.get<QString>("<xmlattr>.name"),
                                           parameter.second.get_value<QString>());
                }

                m_ExtensionSettings.insert(extensionName, extensionParams);
            } catch (std::exception &e) {
                toLog(LogLevel::Error, QString("Skipping broken extension: %1.").arg(e.what()));
            }
        }
    };

    loadExtensions("extensions");
    loadExtensions("config.extensions");
}

//---------------------------------------------------------------------------
ExtensionsSettings::~ExtensionsSettings() = default;

//---------------------------------------------------------------------------
bool ExtensionsSettings::isValid() const {
    return true;
}

//---------------------------------------------------------------------------
TStringMap ExtensionsSettings::getSettings(const QString &aExtensionName) const {
    TStringMap result;

    if (m_ExtensionSettings.contains(aExtensionName)) {
        result = m_ExtensionSettings.value(aExtensionName);
    }

    return result;
}

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
