/* @file Плагин сценария меню платежной книжки */

#include "FirmwareUploadScenario.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QMetaObject>

#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/IDevice.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>

#include "DeviceService.h"
#include "EventService.h"
#include "GUIService.h"
#include "System/IApplication.h"
#include "TerminalService.h"
#include "UpdateEngine/ReportBuilder.h"

namespace PPSDK = SDK::PaymentProcessor;

//--------------------------------------------------------------------------
namespace CFirmwareUploadScenario {
const QString Name = "FirmwareUpload";

const int UploadRetryTimeout = 10 * 1000;
const int DeviceInitializedTimeout = 10 * 60 * 1000;
} // namespace CFirmwareUploadScenario

//---------------------------------------------------------------------------
FirmwareUploadScenario::FirmwareUploadScenario(IApplication *aApplication)
    : Scenario(CFirmwareUploadScenario::Name, ILog::getInstance(CFirmwareUploadScenario::Name)),
      m_Application(aApplication), m_RetryCount(2), m_Device(nullptr), m_DeviceInitializedTimer(0) {
    m_ReportBuilder = new ReportBuilder(m_Application->getWorkingDirectory());
}

//---------------------------------------------------------------------------
FirmwareUploadScenario::~FirmwareUploadScenario() {
    delete m_ReportBuilder;
}

//---------------------------------------------------------------------------
bool FirmwareUploadScenario::initialize(const QList<GUI::SScriptObject> & /*aScriptObjects*/) {
    return true;
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::start(const QVariantMap &aContext) {
    Q_UNUSED(aContext);

    m_TimeoutTimer.stop();

    lockGUI();

    m_Command = RemoteService::instance(m_Application)
                    ->findUpdateCommand(PPSDK::IRemoteService::FirmwareUpload);

    if (!m_Command.isValid()) {
        toLog(LogLevel::Error, "Not found active FirmwareUpload command.");

        GUIService::instance(m_Application)->disable(false);

        QVariantMap parameters;
        parameters.insert("result", "abort");
        emit finished(parameters);

        return;
    }

    toLog(LogLevel::Normal, QString("Command %1 loaded.").arg(m_Command.ID));

    m_ReportBuilder->open(
        QString::number(m_Command.ID), m_Command.configUrl.toString(), m_Command.parameters.at(2));
    m_ReportBuilder->setStatus(PPSDK::IRemoteService::Executing);

    QFile firmwareFile(m_Application->getWorkingDirectory() + "/update/" +
                       m_Command.parameters.at(1) + "/firmware.bin");
    if (firmwareFile.open(QIODevice::ReadOnly)) {
        m_Firmware = firmwareFile.readAll();
        firmwareFile.close();

        toLog(LogLevel::Normal,
              QString("Firmware read from file. size=%1.").arg(m_Firmware.size()));
    } else {
        m_RetryCount = 0;
        abortScenario(QString("Error open file %1.").arg(firmwareFile.fileName()));
        return;
    }

    if (QCryptographicHash::hash(m_Firmware, QCryptographicHash::Md5).toHex().toLower() !=
        m_Command.parameters.at(2).toLower()) {
        m_RetryCount = 0;
        abortScenario("Firmware MD5 verify failed.");
        return;
    }

    toLog(LogLevel::Normal,
          QString("Firmware MD5 verify OK. (%1).").arg(m_Command.parameters.at(2)));

    QMetaObject::invokeMethod(this, "acquireDevice", Qt::QueuedConnection);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::stop() {
    if (m_ReportBuilder) {
        m_ReportBuilder->close();
    }
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::pause() {}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::resume(const QVariantMap & /*aContext*/) {}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::signalTriggered(const QString & /*aSignal*/,
                                             const QVariantMap & /*aArguments*/) {}

//---------------------------------------------------------------------------
QString FirmwareUploadScenario::getState() const {
    return {"main"};
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::onTimeout() {}

//--------------------------------------------------------------------------
bool FirmwareUploadScenario::canStop() {
    return false;
}

//--------------------------------------------------------------------------
void FirmwareUploadScenario::lockGUI() {
    auto *guiService = GUIService::instance(m_Application);

    // Показываем экран блокировки и определяем, что делать дальше.
    guiService->show("SplashScreen", QVariantMap());

    QVariantMap parameters;
    parameters.insert("firmware_upload", true);

    guiService->notify("SplashScreen", parameters);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::abortScenario(const QString &aErrorMessage) {
    toLog(LogLevel::Error, aErrorMessage);

    if (--m_RetryCount > 0) {
        toLog(LogLevel::Normal,
              QString("Retry upload throw %1 sec")
                  .arg(CFirmwareUploadScenario::UploadRetryTimeout / 1000));

        if (m_Device) {
            QTimer::singleShot(
                CFirmwareUploadScenario::UploadRetryTimeout, this, SLOT(onDeviceInitialized()));
        } else {
            QTimer::singleShot(
                CFirmwareUploadScenario::UploadRetryTimeout, this, SLOT(acquireDevice()));
        }

        return;
    }

    killTimer(m_DeviceInitializedTimer);

    if (m_Device) {
        disconnect(dynamic_cast<QObject *>(m_Device),
                   SDK::Driver::IDevice::InitializedSignal,
                   this,
                   SLOT(onDeviceInitialized()));
        disconnect(dynamic_cast<QObject *>(m_Device),
                   SDK::Driver::IDevice::UpdatedSignal,
                   this,
                   SLOT(onUpdated(bool)));

        DeviceService::instance(m_Application)->releaseDevice(m_Device);
        m_Device = nullptr;
    }

    m_ReportBuilder->setStatusDescription(aErrorMessage);
    m_ReportBuilder->setStatus(PPSDK::IRemoteService::Error);

    cleanFirmwareArtifacts();

    EventService::instance(m_Application)->sendEvent(PPSDK::Event(PPSDK::EEventType::Restart));

    QVariantMap parameters;
    parameters.insert("result", "abort");
    emit finished(parameters);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::acquireDevice() {
    SDK::Driver::IDevice *device = nullptr;
    QString deviceConfig;

    foreach (auto config, DeviceService::instance(m_Application)->getConfigurations()) {
        if (config.contains(m_Command.parameters.at(1))) {
            deviceConfig = config;
            device = DeviceService::instance(m_Application)->acquireDevice(config);
            break;
        }
    }

    if (device == nullptr) {
        abortScenario(QString("Not found device with GUID:%1.").arg(m_Command.parameters.at(1)));
        return;
    }

    toLog(LogLevel::Normal, QString("Device '%1' acquire OK.").arg(device->getName()));

    const QMetaObject::Connection updatedConnection = connect(dynamic_cast<QObject *>(device),
                                                              SDK::Driver::IDevice::UpdatedSignal,
                                                              this,
                                                              SLOT(onUpdated(bool)));
    if (updatedConnection == QMetaObject::Connection()) {
        abortScenario("Fail connect to device UpdatedSignal.");
        return;
    }

    toLog(LogLevel::Normal, "Device UpdatedSignal connected.");

    // Проверяем в каком состоянии находится устройство
    auto status = DeviceService::instance(m_Application)->getDeviceStatus(deviceConfig);
    if (status && status->isMatched(SDK::Driver::EWarningLevel::Warning)) {
        toLog(LogLevel::Normal,
              QString("Device '%1' have status: %2.")
                  .arg(device->getName())
                  .arg(status->description()));

        m_Device = device;

        QMetaObject::invokeMethod(this, "onDeviceInitialized", Qt::QueuedConnection);
    } else {
        const QMetaObject::Connection initializedConnection =
            connect(dynamic_cast<QObject *>(device),
                    SDK::Driver::IDevice::InitializedSignal,
                    this,
                    SLOT(onDeviceInitialized()));
        if (initializedConnection == QMetaObject::Connection()) {
            abortScenario("Fail connect to device Initialized signal.");
            return;
        }

        toLog(LogLevel::Normal, "Device Initialized signal connected.");

        m_Device = device;

        auto configuration = device->getDeviceConfiguration();
        int waitInitializedTimeout =
            configuration.contains(CHardwareSDK::WaitUpdatingTimeout)
                ? configuration.value(CHardwareSDK::WaitUpdatingTimeout).toInt()
                : CFirmwareUploadScenario::DeviceInitializedTimeout;

        // Здесь ждём инициализации устройства -> onDeviceInitialized()
        // Если не инициализировались в течении DeviceInitializedTimeout, то обрываем сценарий
        m_DeviceInitializedTimer = startTimer(waitInitializedTimeout);

        toLog(LogLevel::Normal, "Wait for device initialize...");
    }
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::timerEvent(QTimerEvent *aEvent) {
    if (aEvent->timerId() == m_DeviceInitializedTimer) {
        killTimer(m_DeviceInitializedTimer);

        abortScenario("Device initialization timeout.");
    }
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::onDeviceInitialized() {
    toLog(LogLevel::Normal, "Device initialized.");

    killTimer(m_DeviceInitializedTimer);
    disconnect(dynamic_cast<QObject *>(m_Device),
               SDK::Driver::IDevice::InitializedSignal,
               this,
               SLOT(onDeviceInitialized()));

    if (!m_Device) {
        abortScenario("onDeviceInitialized but m_Device isNULL.");
        return;
    }

    if (!m_Device->canUpdateFirmware()) {
        abortScenario("Device can't update.");
        return;
    }

    toLog(LogLevel::Normal, "START upload firmware.");

    m_Device->updateFirmware(m_Firmware);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::onUpdated(bool aSuccess) {
    if (aSuccess) {
        toLog(LogLevel::Normal, "Firmware upload OK.");

        m_ReportBuilder->setStatusDescription("OK");
        m_ReportBuilder->setStatus(PPSDK::IRemoteService::OK);
    } else {
        toLog(LogLevel::Error, "Firmware upload failed. See the log for details device errors.");

        m_ReportBuilder->setStatusDescription("Fail");
        m_ReportBuilder->setStatus(PPSDK::IRemoteService::Error);
    }

    cleanFirmwareArtifacts();

    EventService::instance(m_Application)->sendEvent(PPSDK::Event(PPSDK::EEventType::Restart));

    QVariantMap parameters;
    parameters.insert("result", aSuccess ? "ok" : "error");
    emit finished(parameters);
}

//---------------------------------------------------------------------------
void FirmwareUploadScenario::cleanFirmwareArtifacts() {
    killTimer(m_DeviceInitializedTimer);

    if (m_Command.isValid()) {
        QDir updateDir(m_Application->getWorkingDirectory() + "/update/" +
                       m_Command.parameters.at(1));

        updateDir.remove("firmware.bin");
        updateDir.cdUp();
        updateDir.rmdir(m_Command.parameters.at(1));
    }
}

//---------------------------------------------------------------------------
