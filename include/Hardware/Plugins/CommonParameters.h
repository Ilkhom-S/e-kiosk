/* @file Общие параметры плагинов. */

#pragma once

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/DetectingPriority.h>
#include <SDK/Drivers/InteractionTypes.h>
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/PluginParameters.h>

#include <algorithm>

#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Plugins/DevicePluginBase.h"
#include "Hardware/Protocols/Common/ProtocolNames.h"

//------------------------------------------------------------------------
/// Переводы общих параметров плагинов.
namespace CommonPluginParameterTranslations {
extern const char ModelName[];
extern const char OPOSName[];
extern const char RequiredResource[];
extern const char InteractionType[];
extern const char ProtocolName[];
extern const char ProtocolType[];
}; // namespace CommonPluginParameterTranslations

namespace CPPT = CommonPluginParameterTranslations;
namespace DSDKIT = SDK::Driver::CInteractionTypes;

/// Создать путь драйвера.
template <class T> inline QString makeDriverPath() {
    QString result = makePath(SDK::Driver::Application,
                              SDK::Driver::CComponents::Driver,
                              T::getDeviceType(),
                              T::getInteractionType());

    QString series = T::getSeries();
    QString subSeries = T::getSubSeries();

    if (!series.isEmpty())
        result += "." + series;
    if (!subSeries.isEmpty())
        result += "." + subSeries;

    return result;
}

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
template <class T> inline QStringList sortParameters(QList<T> (*aGetParameters)()) {
    QList<T> data = (*aGetParameters)();
    QStringList result;

    foreach (T item, data) {
        result << QString("%1").arg(item);
    }

    result.removeDuplicates();
    std::sort(result.begin(), result.end());

    if (result.isEmpty()) {
        result << "";
    }

    return result;
}

//------------------------------------------------------------------------------
inline QStringList sortParameters(QStringList (*aGetParameters)()) {
    return sortParameters<QString>(reinterpret_cast<QList<QString> (*)()>(aGetParameters));
}

//------------------------------------------------------------------------------
inline TParameterList modifyValue(const TParameterList &aParameterList,
                                  const QString &aName,
                                  const QVariant &aValue,
                                  const QString &aOldValue = "") {
    TParameterList parameterList(aParameterList);

    auto it = std::find_if(
        parameterList.begin(), parameterList.end(), [&aName](const SPluginParameter &aParameter) {
            return aParameter.name == aName;
        });

    if (it != parameterList.end()) {
        it->defaultValue = aValue;
        QVariantMap &possibleValues = it->possibleValues;

        if (possibleValues.contains(aName)) {
            possibleValues[aName] = aValue;
        } else if (possibleValues.contains(aOldValue) && (possibleValues[aOldValue] == aOldValue)) {
            possibleValues.remove(aOldValue);
            possibleValues.insert(aValue.toString(), aValue);
        }
    }

    return parameterList;
}

//------------------------------------------------------------------------------
inline TParameterList modifyPriority(const TParameterList &aParameterList,
                                     SDK::Driver::EDetectingPriority::Enum aPriority) {
    return modifyValue(aParameterList, CHardwareSDK::DetectingPriority, aPriority);
}

//------------------------------------------------------------------------------
/// Модифицированные значения параметров.
inline SPluginParameter setModifiedValues(const QString &aParameterValue,
                                          const QVariantMap &aPossibleValues) {
    return SPluginParameter(CPlugin::ModifiedValues,
                            SPluginParameter::Set,
                            false,
                            aParameterValue,
                            QString(),
                            QString(),
                            aPossibleValues,
                            true);
}

//------------------------------------------------------------------------------
/// Модифицированные значения параметров.
inline SPluginParameter setModifiedValues(const QString &aParameterValue,
                                          const QString &aValueFrom,
                                          const QString &aValueTo) {
    QVariantMap possibleValues;
    possibleValues.insert(aValueFrom, aValueTo);

    return SPluginParameter(CPlugin::ModifiedValues,
                            SPluginParameter::Set,
                            false,
                            aParameterValue,
                            QString(),
                            QString(),
                            possibleValues,
                            true);
}

//------------------------------------------------------------------------------
/// Приоритет при авто поиске.
inline SPluginParameter setNormalPriority() {
    QVariantMap possibleValues;
    possibleValues.insert(CHardwareSDK::DetectingPriority, SDK::Driver::EDetectingPriority::Normal);

    return SPluginParameter(CHardwareSDK::DetectingPriority,
                            SPluginParameter::Text,
                            false,
                            QString(),
                            QString(),
                            SDK::Driver::EDetectingPriority::Normal,
                            possibleValues,
                            true);
}

//------------------------------------------------------------------------------
/// Множественный тип авто поиска устройства.
inline SPluginParameter setMultipleExistence() {
    return SPluginParameter(CHardwareSDK::Existence,
                            false,
                            QString(),
                            QString(),
                            CHardwareSDK::ExistenceTypes::Multiple,
                            QStringList() << CHardwareSDK::ExistenceTypes::Multiple,
                            true);
}

//------------------------------------------------------------------------------
template <class T>
inline TParameterList createSimpleNamedList(const QStringList &aModels, const QString &aDefault) {
    QString interactionType = T::getInteractionType();
    QVariantMap modifiedValues;
    modifiedValues.insert("no change", CHardwareSDK::Values::Auto);
    modifiedValues.insert("not use", CHardwareSDK::Values::NotUse);

    return TParameterList() << SPluginParameter(CHardwareSDK::ModelName,
                                                false,
                                                CPPT::ModelName,
                                                QString(),
                                                aDefault,
                                                aModels,
                                                true)
                            << SPluginParameter(CHardwareSDK::InteractionType,
                                                true,
                                                CPPT::InteractionType,
                                                QString(),
                                                interactionType,
                                                QStringList() << interactionType)
                            << setModifiedValues("", modifiedValues) << setNormalPriority();
}

//------------------------------------------------------------------------------
template <class T1, class T2> struct SNamedList {
    TParameterList create(const QStringList &aModels, const QString &aDefault) {
        return createSimpleNamedList<T1>(aModels, aDefault);
    }
};

//------------------------------------------------------------------------------
template <class T1> struct SNamedList<T1, DSDKIT::ItCOM> {
    TParameterList create(const QStringList &aModels, const QString &aDefault) {
        QStringList optionalPortSettings = sortParameters(&T1::getOptionalPortSettings);

        return createSimpleNamedList<T1>(aModels, aDefault)
               << SPluginParameter(CHardwareSDK::RequiredResource,
                                   SPluginParameter::Text,
                                   false,
                                   CPPT::RequiredResource,
                                   QString(),
                                   "Common.Driver.IOPort.System.COM",
                                   QVariantMap(),
                                   true)
               << SPluginParameter(CHardwareSDK::OptionalPortSettings,
                                   false,
                                   QString(),
                                   QString(),
                                   optionalPortSettings[0],
                                   optionalPortSettings,
                                   true);
    }
};

//------------------------------------------------------------------------------
template <class T1> struct SNamedList<T1, DSDKIT::ItUSB> {
    TParameterList create(const QStringList &aModels, const QString &aDefault) {
        return createSimpleNamedList<T1>(aModels, aDefault) << setMultipleExistence();
    }
};

//------------------------------------------------------------------------------
template <class T1> struct SNamedList<T1, DSDKIT::ItLibUSB> {
    TParameterList create(const QStringList &aModels, const QString &aDefault) {
        return modifyValue(createSimpleNamedList<T1>(aModels, aDefault),
                           CHardwareSDK::InteractionType,
                           SDK::Driver::CInteractionTypes::USB,
                           SDK::Driver::CInteractionTypes::LibUSB)
               << setMultipleExistence();
    }
};

//------------------------------------------------------------------------------
template <class T1> struct SNamedList<T1, DSDKIT::ItTCP> {
    TParameterList create(const QStringList &aModels, const QString &aDefault) {
        return createSimpleNamedList<T1>(aModels, aDefault)
               << SPluginParameter(CHardwareSDK::RequiredResource,
                                   SPluginParameter::Text,
                                   false,
                                   CPPT::RequiredResource,
                                   QString(),
                                   "Common.Driver.IOPort.System.TCP",
                                   QVariantMap(),
                                   true)
               << setMultipleExistence();
    }
};

//------------------------------------------------------------------------------
template <class T1> struct SNamedList<T1, DSDKIT::ItOPOS> {
    TParameterList create(const QStringList &aModels, const QString &aDefault) {
        QStringList possibleNames = sortParameters(&T1::getProfileNames);

        return createSimpleNamedList<T1>(aModels, aDefault) << setMultipleExistence()
                                                            << SPluginParameter(CHardware::OPOSName,
                                                                                true,
                                                                                CPPT::OPOSName,
                                                                                QString(),
                                                                                possibleNames[0],
                                                                                possibleNames);
    }
};

//------------------------------------------------------------------------------
/// Создать список параметров с именем модели.
template <class T>
inline TParameterList createNamedList(const QStringList &aModels, const QString &aDefault) {
    return SNamedList<T, typename T::TIType>().create(aModels, aDefault);
}

//------------------------------------------------------------------------------
/// Создать список параметров с именем модели.
template <class T>
inline TParameterList createNamedList(const QString &aModel, const QString &aDefault) {
    return createNamedList<T>(QStringList() << aModel, aDefault);
}

//------------------------------------------------------------------------------
/// Создать простой список параметров с именем модели.
template <class T> inline TParameterList createNamedList(const QString &aModel) {
    return createNamedList<T>(aModel, aModel);
}

//------------------------------------------------------------------------------
/// Протокол.
inline SPluginParameter setProtocol(const QString &aProtocol) {
    return SPluginParameter(CHardwareSDK::ProtocolName,
                            false,
                            CPPT::ProtocolName,
                            QString(),
                            aProtocol,
                            QStringList() << aProtocol);
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Тип протокола.
inline SPluginParameter setProtocolType(const QString &aDefaultType,
                                        const QStringList &aPossibleTypes) {
    return SPluginParameter(CHardware::ProtocolType,
                            false,
                            CPPT::ProtocolType,
                            QString(),
                            aDefaultType,
                            aPossibleTypes);
}

//------------------------------------------------------------------------------
#define CREATE_PLUGIN(aPluginName, aClassName)                                                     \
    IPlugin *CreatePlugin_##aClassName(IEnvironment *aEnvironment, const QString &aInstancePath) { \
        return new DevicePluginBase<aClassName>(aPluginName, aEnvironment, aInstancePath);         \
    }

#define REGISTER_DRIVER(aPluginName, aClassName, aParameters)                                      \
    CREATE_PLUGIN(aPluginName, aClassName)                                                         \
    REGISTER_PLUGIN_WITH_PARAMETERS(                                                               \
        makeDriverPath<aClassName>(), &CreatePlugin_##aClassName, aParameters, aClassName)

#define REGISTER_DRIVER_WITH_PARAMETERS(aClassName, aConstructor, aParameters)                     \
    REGISTER_PLUGIN_WITH_PARAMETERS(                                                               \
        makeDriverPath<aClassName>(), aConstructor, aParameters, aClassName)

#define COMMON_DRIVER_WITH_PARAMETERS(aClassName, aConstructor, aParameters)                       \
    REGISTER_PLUGIN_WITH_PARAMETERS(                                                               \
        makeDriverPath<aClassName>(), aConstructor, aParameters, aClassName)
#define COMMON_DRIVER(aClassName, aParameters)                                                     \
    COMMON_DRIVER_WITH_PARAMETERS(aClassName, &CreatePlugin<aClassName>, aParameters)
#define SIMPLE_COMMON_DRIVER(aClassName, aParameters)                                              \
    COMMON_DRIVER(aClassName, &aParameters<aClassName>)

#define BEGIN_REGISTER_PLUGIN
#define END_REGISTER_PLUGIN

} // namespace Plugin
} // namespace SDK

//------------------------------------------------------------------------------
