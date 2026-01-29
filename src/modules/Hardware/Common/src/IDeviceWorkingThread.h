/* @file Интерфейс рабочего потока устройства. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

//--------------------------------------------------------------------------------
class IDeviceWorkingThread : public QThread
{
    Q_OBJECT

  signals:
    /// Сигнал статуса.
    void status(SDK::Driver::EWarningLevel::Enum, const QString &, int);

    /// Сигнал инициализации.
    void initialized();

  public slots:
    /// Инициализация.
    virtual void initialize() = 0;

    /// Проверка существования.
    virtual bool checkExistence() = 0;
};

//--------------------------------------------------------------------------------
