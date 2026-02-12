/* @file Виджет, отображающий состояние устройства */

#pragma once

#include "DeviceSlot.h"
#include "ui_DeviceStatusWindow.h"

class HumoServiceBackend;

//------------------------------------------------------------------------------
class DeviceStatusWindow : public DeviceSlot {
    Q_OBJECT

public:
    DeviceStatusWindow(HumoServiceBackend *aBackend,
                       const QString &aConfigurationName,
                       QWidget *aParent = 0);
    ~DeviceStatusWindow();

public:
    void updateDeviceStatus(const QString &aNewStatus,
                            const QString &aStatusColor,
                            SDK::Driver::EWarningLevel::Enum aLevel);

private slots:
    /// Перерисовка виджета, связанного со слотом.
    virtual void onRepaint();

protected:
    /// Создание виджета, который будет использоваться для визуализации.
    /// Используется в getWidget().
    virtual QWidget *createWidget();

private:
    Ui::DeviceStatusWindow ui{};
};

//------------------------------------------------------------------------------
