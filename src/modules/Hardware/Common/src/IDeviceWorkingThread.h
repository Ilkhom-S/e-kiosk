/* @file Интерфейс рабочего потока устройства. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>

#include <SDK/Drivers/WarningLevel.h>

//--------------------------------------------------------------------------------
class IDeviceWorkingThread : public QThread {
    Q_OBJECT

signals:
    /// Сигнал статуса.
    void status(SDK::Driver::EWarningLevel::Enum, const QString &, int);

    /// Сигнал инициализации.
    void initialized();

public:
    virtual ~IDeviceWorkingThread() override = default;

public slots:
    /// Инициализация.
    virtual void initialize() = 0;

    /// Проверка существования.
    virtual bool checkExistence() = 0;
};

//--------------------------------------------------------------------------------
