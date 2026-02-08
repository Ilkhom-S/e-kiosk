/* @file Базовый класс сторожевого устройства. */

#include <Hardware/Watchdogs/WatchdogBase.h>
#include <Hardware/Watchdogs/WatchdogStatusesDescriptions.h>

WatchdogBase::WatchdogBase() {
    m_PingTimer.moveToThread(&m_Thread);

    connect(&m_PingTimer, SIGNAL(timeout()), SLOT(onPing()));

    m_PollingInterval = 5000;

    m_IOMessageLogging = ELoggingType::ReadWrite;
    m_SensorDisabledValue = false;
    setConfigParameter(CHardware::Watchdog::CanRegisterKey, false);
    setConfigParameter(CHardware::Watchdog::CanWakeUpPC, false);
    m_StatusCodesSpecification =
        DeviceStatusCode::PSpecifications(new WatchdogStatusCode::CSpecifications());
}

//----------------------------------------------------------------------------
bool WatchdogBase::updateParameters() {
    setPingEnable(true);

    return true;
}

//--------------------------------------------------------------------------------
bool WatchdogBase::release() {
    setPingEnable(false);

    return TWatchdogBase::release();
}

//-----------------------------------------------------------------------------
void WatchdogBase::setPingEnable(bool aEnabled) {
    if (checkConnectionAbility() && m_PingTimer.interval()) {
        toLog(LogLevel::Normal, aEnabled ? "Ping is enabled." : "Pinging is disabled.");

        QMetaObject::invokeMethod(&m_PingTimer, aEnabled ? "start" : "stop", Qt::QueuedConnection);
    }
}

//---------------------------------------------------------------------------
void WatchdogBase::cleanStatusCodes(TStatusCodes &aStatusCodes) {
    bool needUpdateConfiguration = false;

    for (auto it = CWatchdogs::SensorData.data().begin(); it != CWatchdogs::SensorData.data().end();
         ++it) {
        QString sensor = it.key();
        QString sensorValue = getConfigParameter(sensor, CHardwareSDK::Values::Auto).toString();

        if (containsConfigParameter(sensor) && (sensorValue == CHardwareSDK::Values::Auto)) {
            needUpdateConfiguration = true;

            bool sensorActive = aStatusCodes.contains(it->statusCode) != m_SensorDisabledValue;
            sensorValue = sensorActive ? CHardwareSDK::Values::Use : CHardwareSDK::Values::NotUse;

            setConfigParameter(sensor, sensorValue);
        }

        if (sensorValue != CHardwareSDK::Values::Use) {
            aStatusCodes.remove(it->statusCode);
        }
    }

    if (needUpdateConfiguration) {
        emit configurationChanged();
    }

    TWatchdogBase::cleanStatusCodes(aStatusCodes);
}

//--------------------------------------------------------------------------------
void WatchdogBase::emitStatusCodes(TStatusCollection &aStatusCollection, int aExtendedStatus) {
    TWatchdogBase::emitStatusCodes(aStatusCollection, aExtendedStatus);

    TStatusCodes statusCodes = getStatusCodes(aStatusCollection);

    foreach (int statusCode, statusCodes) {
        auto it = std::find_if(CWatchdogs::SensorData.data().begin(),
                               CWatchdogs::SensorData.data().end(),
                               [&](const CWatchdogs::SSensorData &aData) -> bool {
                                   return aData.statusCode == statusCode;
                               });

        if (it != CWatchdogs::SensorData.data().end()) {
            QString actionValue = getConfigParameter(it->action).toString();

            if ((actionValue == CHardwareSDK::Values::Use) &&
                CWatchdogs::SensorActionData.data().contains(actionValue)) {
                int extendedStatus = CWatchdogs::SensorActionData[actionValue];

                emitStatusCode(statusCode, extendedStatus);
            }
        }
    }
}

//----------------------------------------------------------------------------
