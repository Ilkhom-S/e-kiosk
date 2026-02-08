/* @file Windows-реализация асинхронного последовательного порта. */

#pragma once

// Windows
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <QtCore/QSet>
#include <QtCore/QUuid>
#include <QtCore/QVector>

#include <SDK/Drivers/IOPort/COMParameters.h>

#include <Hardware/IOPorts/COM/windows/System_DeviceUtils.h>
#include <Hardware/IOPorts/IOPortBase.h>
#include <Hardware/IOPorts/IOPortGUIDs.h>
#include <windows.h>

//--------------------------------------------------------------------------------
typedef QVector<QUuid> TUuids;

/// Буфер для чтения.
typedef QVector<char> TReadingBuffer;

/// Данные портов.
typedef QMap<QString, QString> TIOPortDeviceData;

//--------------------------------------------------------------------------------

#define BOOL_CALL(aFunctionName, ...)                                                              \
    [&]() -> bool {                                                                                \
        if (!checkReady())                                                                         \
            return false;                                                                          \
        return process(std::bind(&::aFunctionName, m_PortHandle, __VA_ARGS__), #aFunctionName);     \
    }()

//--------------------------------------------------------------------------------
class AsyncSerialPortWin : public IOPortBase {
    SET_SERIES("COM")

    typedef std::function<BOOL()> TBOOLMethod;

public:
    AsyncSerialPortWin();

    /// Возвращает список доступных в системе портов.
    static QStringList enumerateSystem_Names();

    /// Опрашивает данные портов.
    virtual void initialize();

#pragma region SDK::Driver::IDevice
    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова
    /// initialize().
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
    virtual bool perform_Open();

    /// Прочитать данные.
    virtual bool processReading(QByteArray &aData, int aTimeout);

    /// Выполнить команду.
    bool process(TBOOLMethod aMethod, const QString &aFunctionName);

    /// Обработка ошибки.
    void handleError(const QString &aFunctionName);

    /// Логгирование ошибки.
    void logError(const QString &aFunctionName);

    /// Проверить готовность порта.
    virtual bool checkReady();

    /// Ждет окончание асинхронной операции.
    bool waitAsyncAction(DWORD &aResult, int aTimeout);

    /// Управление структурой DCB.
    bool setBaudRate(SDK::Driver::IOPort::COM::EBaudRate::Enum aValue);
    bool setRTS(SDK::Driver::IOPort::COM::ERTSControl::Enum aValue);
    bool setDTR(SDK::Driver::IOPort::COM::EDTRControl::Enum aValue);
    bool setStopBits(SDK::Driver::IOPort::COM::EStopBits::Enum aValue);
    bool setByteSize(int aValue);
    bool setParity(SDK::Driver::IOPort::COM::EParity::Enum aValue);

    /// Запись настроек в порт.
    bool applyPortSettings();

    /// Инициализировать структуру для асинхронного обмена с портом.
    void initializeOverlapped(OVERLAPPED &aOverlapped);

    /// Проверить хэндл порта.
    bool checkHandle();

    /// Порт существует?
    virtual bool isExist();

    /// Служебная структура настроек.
    DCB m_DCB;

    /// Хендл порта.
    HANDLE m_PortHandle;

    /// Структуры и мьютексы для отслеживания асинхронных чтения/записи.
    QMutex m_ReadMutex;
    QMutex m_WriteMutex;
    OVERLAPPED m_ReadOverlapped;
    OVERLAPPED m_WriteOverlapped;
    DWORD m_ReadEventMask;

    /// Буфер для чтения.
    TReadingBuffer m_ReadingBuffer;

    /// Cуществует в системе.
    bool m_Exist;

    /// Последняя системная ошибка.
    DWORD m_LastError;

    /// Последняя ошибка проверки порта.
    DWORD m_LastErrorChecking;

    /// Максимальное количество байтов для чтения.
    int m_MaxReadingSize;

    /// Системные имена портов.
    QStringList m_System_Names;

    /// Нефильтрованные системные данные портов.
    TWinDeviceProperties m_WinProperties;

    /// Системное свойство для формирования пути для открытия порта.
    DWORD m_PathProperty;

    /// GUID-ы для авто поиска.
    TUuids m_Uuids;

    /// Получение системных данных о портах (порт -> виртуальность).
    typedef QMap<QString, SDK::Driver::EPortTypes::Enum> TData;
    static TData getSystem_Data(bool aForce = false);

    /// Получить данные о ресурсах.
    static TWinDeviceProperties getDeviceProperties(const TUuids &aUuids,
                                                    DWORD aPropertyName,
                                                    bool aQuick = false,
                                                    TIOPortDeviceData *aData = nullptr);

    /// Ждать окончания асинхронного чтения из порта, если результат - WAIT_TIMEOUT.
    bool m_WaitResult;

    /// Количество прочитанных байтов.
    DWORD m_ReadBytes;
};

//--------------------------------------------------------------------------------