/* @file Функционал работы с интегрированными драйверами. */

// Driver SDK

#include "IntegratedDrivers.h"

#include <QtCore/QSet>

#include <SDK/Drivers/HardwareConstants.h>

#include <algorithm>

using namespace SDK::Plugin;

//------------------------------------------------------------------------------
IntegratedDrivers::IntegratedDrivers() : m_DeviceManager(nullptr) {}

//------------------------------------------------------------------------------
void IntegratedDrivers::initialize(DeviceManager *aDeviceManager) {
    m_Data.clear();
    m_DeviceManager = aDeviceManager;

    QList<TPaths> commonPaths;
    auto getDeviceType = [](const QString &aPath) -> QString {
        QStringList pathParts = aPath.split(".");
        return (pathParts.size() > 1) ? pathParts[2] : "";
    };

    QStringList driverPathList = m_DeviceManager->getDriverList();
    TModelList modelList = m_DeviceManager->getModelList("");

    foreach (const QString &path, driverPathList) {
        QSet<QString> models = QSet<QString>(modelList[path].begin(), modelList[path].end());
        QString deviceType = getDeviceType(path);

        if (!models.isEmpty()) {
            foreach (const QString &checkingPath, driverPathList) {
                QSet<QString> checkingModels =
                    QSet<QString>(modelList[checkingPath].begin(), modelList[checkingPath].end());
                QSet<QString> commonModels = checkingModels & models;
                QString checkingDeviceType = getDeviceType(checkingPath);

                if ((checkingDeviceType == deviceType) && (checkingPath != path) &&
                    !commonModels.isEmpty()) {
                    auto it = std::find_if(
                        commonPaths.begin(), commonPaths.end(), [&](const TPaths &aPaths) -> bool {
                            return aPaths.contains(checkingPath) || aPaths.contains(path);
                        });
                    TPaths newPaths = TPaths() << path << checkingPath;

                    if (it == commonPaths.end()) {
                        commonPaths << newPaths;
                    } else {
                        it->unite(newPaths);
                    }
                }
            }
        }
    }

    int index = 0;

    foreach (const TPaths &paths, commonPaths) {
        QMap<QString, TPaths> pathsByModel;

        foreach (const QString &path, paths) {
            foreach (const QString &model, modelList[path]) {
                pathsByModel[model] << path;
            }
        }

        QStringList pathParts = paths.begin()->split(".").mid(0, 3);
        QString pathPart = pathParts.join(".") + ".";

        foreach (const TPaths &localPaths,
                 QSet<TPaths>(pathsByModel.values().begin(), pathsByModel.values().end())) {
            m_Data.insert(pathPart + QString::number(index++),
                          SData(localPaths, pathsByModel.keys(localPaths)));
        }
    }
}

//------------------------------------------------------------------------------
void IntegratedDrivers::checkDriverPath(QString &aDriverPath, const QVariantMap &aConfig) {
    if (!m_Data.contains(aDriverPath)) {
        return;
    }

    QStringList paths =
        QList<QString>(m_Data[aDriverPath].paths.begin(), m_Data[aDriverPath].paths.end());

    for (int i = 0; i < paths.size(); ++i) {
        TParameterList parameters = m_DeviceManager->getDriverParameters(paths[i]);

        for (auto jt = aConfig.begin(); jt != aConfig.end(); ++jt) {
            auto parameterIt = std::find_if(parameters.begin(),
                                            parameters.end(),
                                            [&](const SPluginParameter &aParameter) -> bool {
                                                return aParameter.name == jt.key();
                                            });

            if (parameterIt != parameters.end()) {
                SPluginParameter &parameter = *parameterIt;
                const QList<QVariant> &possibleValueValues = parameter.possibleValues.values();
                const QList<QString> &possibleValueKeys = parameter.possibleValues.keys();
                const QVariant &value = jt.value();

                if (!parameter.readOnly && !possibleValueValues.contains(value) &&
                    (value != CHardwareSDK::Values::Auto) &&
                    !possibleValueKeys.contains(CHardwareSDK::Mask)) {
                    paths.removeAt(i--);

                    break;
                }
            }
        }
    }

    if (!paths.isEmpty()) {
        aDriverPath = paths[0];
    }
}

//------------------------------------------------------------------------------
void IntegratedDrivers::filterModelList(TModelList &aModelList) const {
    for (auto it = m_Data.begin(); it != m_Data.end(); ++it) {
        for (auto jt = it->paths.begin(); jt != it->paths.end(); ++jt) {
            aModelList.remove(*jt);
        }

        aModelList.insert(it.key(), it->models);
    }
}

//------------------------------------------------------------------------------
void IntegratedDrivers::filterDriverParameters(const QString &aDriverPath,
                                               TParameterList &aParameterList) const {
    if (!m_Data.contains(aDriverPath)) {
        return;
    }

    TPaths paths = m_Data[aDriverPath].paths;
    aParameterList.clear();

    for (auto it = paths.begin(); it != paths.end(); ++it) {
        TParameterList parameters = m_DeviceManager->getDriverParameters(*it);

        for (auto jt = parameters.begin(); jt != parameters.end(); ++jt) {
            auto parameterIt = std::find_if(aParameterList.begin(),
                                            aParameterList.end(),
                                            [&](const SPluginParameter &aParameter) -> bool {
                                                return aParameter.name == jt->name;
                                            });

            if (parameterIt == aParameterList.end()) {
                aParameterList << *jt;
            } else {
                for (auto kt = jt->possibleValues.begin(); kt != jt->possibleValues.end(); ++kt) {
                    parameterIt->possibleValues.insert(kt.key(), kt.value());
                }
            }
        }
    }

    auto parameterIt = std::find_if(aParameterList.begin(),
                                    aParameterList.end(),
                                    [&](const SPluginParameter &aParameter) -> bool {
                                        return aParameter.name == CHardwareSDK::ModelName;
                                    });

    if (parameterIt != aParameterList.end()) {
        parameterIt->possibleValues.clear();

        foreach (const QString &model, m_Data[aDriverPath].models) {
            parameterIt->possibleValues.insert(model, model);
        }
    }
}

//------------------------------------------------------------------------------
void IntegratedDrivers::filterDriverList(QStringList &aDriverList) const {
    for (auto it = m_Data.begin(); it != m_Data.end(); ++it) {
        for (auto jt = it->paths.begin(); jt != it->paths.end(); ++jt) {
            aDriverList.removeAll(*jt);
        }

        aDriverList << it.key();
    }

    std::sort(aDriverList.begin(), aDriverList.end());
}

//------------------------------------------------------------------------------
uint qHash(const IntegratedDrivers::TPaths &aPaths) {
    uint result = 0;
    int index = 0;

    foreach (auto path, aPaths) {
        uint hash = qHash(path);
        int n = index++;
        result ^= (hash << n) | (hash >> ((sizeof(hash) * 8) - n));
    }

    return result;
}

//------------------------------------------------------------------------------
