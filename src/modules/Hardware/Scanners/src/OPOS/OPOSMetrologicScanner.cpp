/* @file Сканер Honeywell Metrologic на OPOS-драйвере. */

#import "OPOSScanner.tlb"

// Windows
#include <objbase.h>

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QUuid>

#include <Hardware/Scanners/OPOSMetrologicScanner.h>

#include "Hardware/Common/BaseStatusDescriptions.h"

#pragma pop_macro("max")
#pragma pop_macro("min")

//--------------------------------------------------------------------------------
OPOSMetrologicScanner::OPOSMetrologicScanner() {
    m_DeviceName = "Honeywell Metrologic based on OPOS driver";
    m_Claim_Timeout = 2000;
    m_ProfileNames = getProfileNames();
    m_PollingInterval = 500;
    m_ExEnabled = false;
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::isConnected() {
    if (!TPollingOPOSScanner::isConnected()) {
        return false;
    }

    QString deviceName = getConfigParameter(CHardwareSDK::ModelName).toString();
    deviceName = deviceName.replace("/", " ").replace(QRegExp(" +"), " ");

    if (!deviceName.isEmpty()) {
        m_DeviceName = deviceName;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::checkConnectionAbility() {
    if (!TPollingOPOSScanner::checkConnectionAbility()) {
        return false;
    }

    typedef OposScanner_1_8_Lib::IOPOSScanner TNativeDriver;
    TNativeDriver *nativeDriver;

    if (FAILED(m_Driver->queryInterface(QUuid(__uuidof(TNativeDriver)), (void **)&nativeDriver))) {
        toLog(LogLevel::Error, "Failed to query interface of the scanner.");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::updateParameters() {
    return setAvailable(true);
}

//--------------------------------------------------------------------------------
void OPOSMetrologicScanner::initializeResources() {
    TPollingOPOSScanner::initializeResources();

    if (m_COMInitialized && !m_Driver.isNull()) {
        connect(m_Driver.data(),
                SIGNAL(signal(const QString &, int, void *)),
                SLOT(onGotData(const QString &, int, void *)),
                Qt::UniqueConnection);
    }
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::processStatus(TStatusCodes &aStatusCodes) {
    bool result = TPollingOPOSScanner::processStatus(aStatusCodes);
    bool error = !getStatusCollection(aStatusCodes)[SDK::Driver::EWarningLevel::Error].isEmpty();

    if (result && (m_Initialized == ERequestStatus::Success) && !error) {
        enable(m_ExEnabled);
    }

    return result;
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::enable(bool aReady) {
    if (!isWorkingThread()) {
        m_ExEnabled = aReady;
    }

    if (!m_StatusCollectionHistory.isEmpty() &&
        (!checkConnectionAbility() && (m_Initialized == ERequestStatus::Fail))) {
        toLog(LogLevel::Error,
              QString("%1: Cannot set possibility of data receiving to %2")
                  .arg(m_DeviceName)
                  .arg(aReady ? "true" : "false"));
        return !aReady;
    }

    if (!isWorkingThread() || (m_Initialized == ERequestStatus::InProcess)) {
        QMetaObject::invokeMethod(
            this, "enable", Qt::BlockingQueuedConnection, Q_ARG(bool, aReady));
    } else {
        bool claimed = BOOL_CALL_OPOS(Claimed);
        bool enabled = BOOL_CALL_OPOS(DeviceEnabled);

        if (!claimed || !enabled) {
            toLog(aReady ? LogLevel::Error : LogLevel::Normal,
                  QString("%1: device is not %2, setEnable returns %3")
                      .arg(m_DeviceName)
                      .arg(enabled ? "claimed" : "enabled")
                      .arg(aReady ? "false" : "true"));
            return !aReady;
        }

        if (BOOL_CALL_OPOS(DataEventEnabled) != aReady) {
            toLog(LogLevel::Normal,
                  QString("%1: set possibility of data receiving to %2")
                      .arg(m_DeviceName)
                      .arg(aReady ? "true" : "false"));
            VOID_CALL_OPOS(SetDataEventEnabled, aReady);
        }

        if (BOOL_CALL_OPOS(DecodeData) != aReady) {
            toLog(LogLevel::Normal,
                  QString("%1: set data decoding to %2")
                      .arg(m_DeviceName)
                      .arg(aReady ? "true" : "false"));
            VOID_CALL_OPOS(SetDecodeData, aReady);
        }
    }

    return (BOOL_CALL_OPOS(DataEventEnabled) == aReady) && (BOOL_CALL_OPOS(DecodeData) == aReady);
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::setAvailable(bool aEnable) {
    QString log = aEnable ? "enabled" : "disabled";
    toLog(LogLevel::Normal, QString("%1: set scanner %2").arg(m_DeviceName).arg(log));

    if (!BOOL_CALL_OPOS(Claimed)) {
        toLog(aEnable ? LogLevel::Error : LogLevel::Normal,
              QString("%1: device is not claimed, setAvailable returns %2")
                  .arg(m_DeviceName)
                  .arg(aEnable ? "false" : "true"));
        return !aEnable;
    }

    if (aEnable == BOOL_CALL_OPOS(DeviceEnabled)) {
        toLog(LogLevel::Normal, QString("%1: already %2").arg(m_DeviceName).arg(log));
        return true;
    }

    VOID_CALL_OPOS(SetDeviceEnabled, aEnable);

    if (aEnable != BOOL_CALL_OPOS(DeviceEnabled)) {
        toLog(LogLevel::Error, QString("%1: Failed to set device %2").arg(m_DeviceName).arg(log));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
void OPOSMetrologicScanner::onGotData(const QString & /*aName*/,
                                      int /*aArgumentsCount*/,
                                      void * /*aArgumentsValues*/) {
    QMutexLocker lock(&m_DataMutex);

    QString capturedData = (((OPOS::OPOSScanner *)sender())->ScanData()).trimmed();

    QString logData = capturedData;

    for (char ch = 0x00; ch < 0x20; ++ch) {
        capturedData.replace(ch, "");
    }

    QString log = QString("%1: data received: %2").arg(getName()).arg(capturedData);

    if (logData != capturedData) {
        log += QString(", {%1} -> {%2}")
                   .arg(logData.toLatin1().toHex().data())
                   .arg(capturedData.toLatin1().toHex().data());
    }

    if (getConfigParameter(CHardware::Scanner::Prefix).toBool()) {
        capturedData = capturedData.mid(COPOSScanners::Prefix);
        log += QString(" -> {%1}").arg(capturedData.toLatin1().toHex().data());
    }

    toLog(LogLevel::Normal, log);

    ((OPOS::OPOSScanner *)sender())->SetDataEventEnabled(true);

    QVariantMap result;
    result[CHardwareSDK::HID::Text] = capturedData;

    emit data(result);
}

//--------------------------------------------------------------------------------
QStringList OPOSMetrologicScanner::getProfileNames() {
    static QStringList result;

    if (result.isEmpty()) {
        result = QSettings("SOFTWARE\\OLEforRetail\\ServiceOPOS\\Scanner", QSettings::NativeFormat)
                     .childGroups();
    }

    return result;
}

//--------------------------------------------------------------------------------
