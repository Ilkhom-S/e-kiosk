/* @file Linux-реализация асинхронного последовательного порта. */

#pragma once

// Linux
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVector>
#include <QtCore/QSet>
#include <QtCore/QUuid>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IOPort/COMParameters.h>

// Project
#include <Hardware/IOPorts/IOPortBase.h>

//--------------------------------------------------------------------------------
class AsyncSerialPortLin : public IOPortBase
{
    SET_SERIES("COM")

  public:
    AsyncSerialPortLin();

    /// Возвращает список доступных в системе портов.
    static QStringList enumerateSystemNames();

    /// Опрашивает данные портов.
    virtual void initialize();

#pragma region SDK::Driver::IDevice
    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
    virtual bool release();
#pragma endregion

#pragma region SDK::Driver::IIOPort
    /// Открыть порт.
    virtual bool open();

    /// Закрыть порт.
    virtual bool close();

    /// Очистить буферы порта.
    virtual bool clear();

    /// Установить параметры порта.
    virtual bool setParameters(const SDK::Driver::TPortParameters &aParameters);

    /// Получить параметры порта.
    virtual void getParameters(SDK::Driver::TPortParameters &aParameters);

    /// Прочитать данные.
    virtual bool read(QByteArray &aData, int aTimeout = DefaultReadTimeout, int aMinSize = 1);

    /// Передать данные.
    virtual bool write(const QByteArray &aData);

    /// Подключено новое устройство?
    virtual bool deviceConnected();

    /// Открыт?
    virtual bool opened();
#pragma endregion

    /// Изменить таймаут выполнения зависоноопасной операции
    void changePerformingTimeout(const QString &aContext, int aTimeout, int aPerformingTime);

  protected:
    /// Идентификация.
    virtual bool checkExistence();

    /// Открыть порт.
    virtual bool performOpen();

    /// Прочитать данные.
    virtual bool processReading(QByteArray &aData, int aTimeout);

    /// Проверить готовность порта.
    virtual bool checkReady();

    /// Порт существует?
    virtual bool isExist();

    /// Файловый дескриптор порта.
    int mPortFd;

    /// Буфер для чтения.
    TReadingBuffer mReadingBuffer;

    /// Cуществует в системе.
    bool mExist;

    /// Максимальное количество байтов для чтения.
    int mMaxReadingSize;

    /// Системные имена портов.
    QStringList mSystemNames;
};

//--------------------------------------------------------------------------------