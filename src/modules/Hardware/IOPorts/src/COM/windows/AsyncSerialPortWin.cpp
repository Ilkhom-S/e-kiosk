/* @file Асинхронная Windows-реализация COM-порта. */

#include <QtCore/QRegularExpression>
#include <QtCore/qmath.h>

#include <Hardware/IOPorts/COM/windows/AsyncSerialPortWin.h>

#include "Hardware/Common/SafePerformer.h"
#include "SysUtils/ISysUtils.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
AsyncSerialPortWin::AsyncSerialPortWin()
    : m_PortHandle(0), m_Exist(false), m_ReadMutex(QMutex::Recursive), m_WriteMutex(QMutex::Recursive),
      m_ReadEventMask(0), m_LastError(0), m_LastErrorChecking(0), m_MaxReadingSize(0),
      m_WaitResult(false), m_ReadBytes(0) {
    setBaudRate(EBaudRate::BR9600);
    setParity(EParity::No);
    setByteSize(8);
    setRTS(ERTSControl::Enable);
    setDTR(EDTRControl::Disable);
    setStopBits(EStopBits::One);

    ::RtlSecureZeroMemory(&m_ReadOverlapped, sizeof(m_ReadOverlapped));
    ::RtlSecureZeroMemory(&m_WriteOverlapped, sizeof(m_WriteOverlapped));

    m_Type = EPortTypes::COM;
    m_System_Names = enumerateSystem_Names();
    m_Uuids = CAsyncSerialPortWin::System::Uuids();
    m_PathProperty = CAsyncSerialPortWin::System::PathProperty;
    setOpeningTimeout(CAsyncSerialPortWin::OpeningTimeout);
}

//--------------------------------------------------------------------------------
void AsyncSerialPortWin::initialize() {
    TIOPortDeviceData deviceData;
    getDeviceProperties(m_Uuids, m_PathProperty, false, &deviceData);

    QStringList minePortData;
    QStringList otherPortData;

    for (auto it = deviceData.begin(); it != deviceData.end(); ++it) {
        bool mine = !m_System_Name.isEmpty() && it->contains(m_System_Name);
        QStringList &target = mine ? minePortData : otherPortData;
        target << it.key() + "\n" + it.value() + "\n";

        if (mine) {
            bool cannotWaitResult =
                std::find_if(CAsyncSerialPortWin::CannotWaitResult.begin(),
                             CAsyncSerialPortWin::CannotWaitResult.end(),
                             [&](const QString &aLexeme) -> bool {
                                 return it->contains(aLexeme, Qt::CaseInsensitive);
                             }) != CAsyncSerialPortWin::CannotWaitResult.end();
            setConfigParameter(CHardware::Port::COM::ControlRemoving, cannotWaitResult);
        }
    }

    adjustData(minePortData, otherPortData);
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::release() {
    if (checkHandle()) {
        BOOL_CALL(CancelIo);
    }

    return IOPortBase::release();
}

//--------------------------------------------------------------------------------
void AsyncSerialPortWin::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    IOPortBase::setDeviceConfiguration(aConfiguration);
    bool unknownSystem_Name = !m_System_Name.isEmpty() && !m_System_Names.contains(m_System_Name);
    EPortTypes::Enum portType = getSystem_Data()[m_System_Name];

    bool cannotWaitResult = getConfigParameter(CHardware::Port::COM::ControlRemoving).toBool();

    // TODO: при увеличении номенклатуры виртуальных/эмуляторных портов продумать логику загрузки
    // девайса с отсутствующим портом
    if ((m_Type == EPortTypes::COM) && (unknownSystem_Name || (portType == EPortTypes::VirtualCOM))) {
        m_Type = EPortTypes::VirtualCOM;
    }

    if (portType == EPortTypes::COMEmulator) {
        m_Type = EPortTypes::COMEmulator;
    }

    if (!m_Exist && !m_System_Name.isEmpty()) {
        checkExistence();
    }

    if (m_Type == EPortTypes::VirtualCOM) {
        m_WaitResult = !cannotWaitResult &&
                      aConfiguration.value(CHardware::Port::COM::WaitResult, m_WaitResult).toBool();
    }

    if (aConfiguration.contains(CHardware::Port::MaxReadingSize)) {
        m_MaxReadingSize = aConfiguration[CHardware::Port::MaxReadingSize].toInt();
    }

    if (getType() != EPortTypes::USB) {
        TPortParameters portParameters;

        if (containsConfigParameter(CHardware::Port::COM::BaudRate)) {
            portParameters.insert(EParameters::BaudRate,
                                  getConfigParameter(CHardware::Port::COM::BaudRate).toInt());
        }

        if (containsConfigParameter(CHardware::Port::COM::Parity)) {
            portParameters.insert(EParameters::Parity,
                                  getConfigParameter(CHardware::Port::COM::Parity).toInt());
        }

        if (containsConfigParameter(CHardware::Port::COM::RTS)) {
            portParameters.insert(EParameters::RTS,
                                  getConfigParameter(CHardware::Port::COM::RTS).toInt());
        }

        if (containsConfigParameter(CHardware::Port::COM::DTR)) {
            portParameters.insert(EParameters::DTR,
                                  getConfigParameter(CHardware::Port::COM::DTR).toInt());
        }

        if (containsConfigParameter(CHardware::Port::COM::ByteSize)) {
            portParameters.insert(EParameters::ByteSize,
                                  getConfigParameter(CHardware::Port::COM::ByteSize).toInt());
        }

        if (containsConfigParameter(CHardware::Port::COM::StopBits)) {
            portParameters.insert(EParameters::StopBits,
                                  getConfigParameter(CHardware::Port::COM::StopBits).toInt());
        }

        setParameters(portParameters);
    }
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::process(TBOOLMethod aMethod, const QString &aFunctionName) {
    BOOL result = aMethod();

    if (!result) {
        handleError(aFunctionName);
    }

    return result;
}

//--------------------------------------------------------------------------------
void AsyncSerialPortWin::logError(const QString &aFunctionName) {
    if (CAsyncSerialPortWin::NoLogErrors.contains(m_LastError)) {
        return;
    }

    if (checkHandle() || (m_LastErrorChecking != m_LastError)) {
        toLog(LogLevel::Error,
              QString("%1: %2 failed with %3.")
                  .arg(m_System_Name)
                  .arg(aFunctionName)
                  .arg(ISysUtils::getErrorMessage(m_LastError)));
    }

    if (!checkHandle()) {
        m_LastErrorChecking = m_LastError;
    } else {
        m_LastErrorChecking = 0;
    }
}

//--------------------------------------------------------------------------------
void AsyncSerialPortWin::handleError(const QString &aFunctionName) {
    m_LastError = ::GetLastError();

    logError(aFunctionName);

    if (CAsyncSerialPortWin::DisappearingErrors.contains(m_LastError)) {
        m_System_Names = getSystem_Data(true).keys();
    }

    if (!m_System_Names.contains(m_System_Name)) {
        close();

        m_Exist = false;
    }
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::checkHandle() {
    return m_PortHandle && (m_PortHandle != INVALID_HANDLE_VALUE);
}

//--------------------------------------------------------------------------------
void AsyncSerialPortWin::changePerformingTimeout(const QString &aContext,
                                                 int aTimeout,
                                                 int aPerformingTime) {
    if ((aContext == CHardware::Port::OpeningContext) &&
        (aTimeout == getConfigParameter(CHardware::Port::OpeningTimeout).toInt())) {
        int newTimeout = int(aPerformingTime * CAsyncSerialPortWin::KOpeningTimeout);
        toLog(LogLevel::Normal,
              QString("Task performing timeout for context \"%1\" has been changed: %2 -> %3")
                  .arg(aContext)
                  .arg(aTimeout)
                  .arg(newTimeout));

        setOpeningTimeout(newTimeout);
    }
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::opened() {
    return checkHandle();
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::open() {
    if (checkHandle()) {
        return true;
    }

    if (getConfigParameter(CHardware::Port::Suspended).toBool()) {
        toLog(LogLevel::Error, m_System_Name + ": Failed to open due to there is a suspended task.");
        return false;
    }

    using namespace std::placeholders;

    STaskData data;
    data.task = std::bind(&AsyncSerialPortWin::perform_Open, this);
    data.forwardingTask = getConfigParameter(CHardware::Port::OpeningContext).value<TVoidMethod>();
    data.changePerformingTimeout =
        std::bind(&AsyncSerialPortWin::changePerformingTimeout, this, _1, _2, _3);
    data.context = CHardware::Port::OpeningContext;
    data.timeout = m_OpeningTimeout;

    ETaskResult::Enum result = SafePerformer(m_Log).process(data);
    setConfigParameter(CHardware::Port::Suspended, result == ETaskResult::Suspended);

    return result == ETaskResult::OK;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::perform_Open() {
    QByteArray fileName = "\\\\.\\" + m_System_Name.toLatin1();
    m_PortHandle = CreateFileA(fileName.data(),
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              0,
                              OPEN_EXISTING,
                              FILE_FLAG_OVERLAPPED,
                              0);

    if (!checkHandle()) {
        handleError("CreateFileA");
        m_PortHandle = 0;

        setConfigParameter(CHardware::Port::Suspended, false);

        return false;
    }

    toLog(LogLevel::Normal, QString("Port %1 is opened.").arg(m_System_Name));

    if (clear() && BOOL_CALL(GetComm_State, &m_DCB)) {
        bool result = BOOL_CALL(ClearComm_Break);

        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = MAXDWORD;

        if (BOOL_CALL(SetComm_Timeouts, &timeouts) && BOOL_CALL(SetComm_Mask, EV_ERR | EV_RXCHAR)) {
            ::RtlSecureZeroMemory(&m_ReadOverlapped, sizeof(m_ReadOverlapped));
            m_ReadOverlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

            ::RtlSecureZeroMemory(&m_WriteOverlapped, sizeof(m_WriteOverlapped));
            m_WriteOverlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

            if (!applyPortSettings()) {
                return false;
            }

            if (m_Type == EPortTypes::VirtualCOM) {
                setConfigParameter(CHardware::Port::Suspended, false);

                toLog(LogLevel::Normal, m_System_Name + " is virtual COM port via USB.");
                return true;
            }

            if (result) {
                setConfigParameter(CHardware::Port::Suspended, false);

                return true;
            }
        }
    }

    close();

    setConfigParameter(CHardware::Port::Suspended, false);

    return false;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::close() {
    bool result = true;
    bool beenOpened = checkHandle();

    auto closeHandle = [&](HANDLE &aHandle) {
        if (aHandle && (aHandle != INVALID_HANDLE_VALUE)) {
            if (!::CloseHandle(aHandle)) {
                m_LastError = ::GetLastError();
                logError("CloseHandle");

                result = false;
            }

            aHandle = 0;
        }
    };

    closeHandle(m_ReadOverlapped.hEvent);
    closeHandle(m_WriteOverlapped.hEvent);
    closeHandle(m_PortHandle);

    if (result && beenOpened) {
        toLog(LogLevel::Normal, QString("Port %1 is closed.").arg(m_System_Name));
    }

    return result;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::clear() {
    Sleep(1);

    bool result = true;
    DWORD errors = 0;

    if (!BOOL_CALL(PurgeComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {
        result = false;
    }

    if (!BOOL_CALL(ClearComm_Error, &errors, nullptr)) {
        result = false;
    }

    return result;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::isExist() {
    return m_Exist;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::checkExistence() {
    m_Exist = m_System_Names.contains(m_System_Name);

    if (!m_Exist && (m_Type != EPortTypes::COM)) {
        m_System_Names = getSystem_Data(true).keys();
        m_Exist = m_System_Names.contains(m_System_Name);
    }

    if (!m_Exist) {
        setOpeningTimeout(CAsyncSerialPortWin::OnlineOpeningTimeout);

        toLog(LogLevel::Error, QString("Port %1 does not exist.").arg(m_System_Name));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::checkReady() {
    if (m_Exist) {
        return true;
    } else if (!checkExistence()) {
        return false;
    }

    if (!open() || !checkHandle()) {
        toLog(LogLevel::Error, QString("Port %1 is not opened.").arg(m_System_Name));
        return false;
    }

    return applyPortSettings();
}

//--------------------------------------------------------------------------------
void AsyncSerialPortWin::initializeOverlapped(OVERLAPPED &aOverlapped) {
    aOverlapped.Internal = 0;
    aOverlapped.InternalHigh = 0;
    aOverlapped.Pointer = nullptr;
    ::ResetEvent(aOverlapped.hEvent);
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::waitAsyncAction(DWORD &aResult, int aTimeout) {
    initializeOverlapped(m_ReadOverlapped);

    m_ReadEventMask = 0;
    ::WaitComm_Event(m_PortHandle, &m_ReadEventMask, &m_ReadOverlapped);
    aResult = ::WaitForSingleObject(m_ReadOverlapped.hEvent, aTimeout);

    if ((aResult != WAIT_OBJECT_0) && (aResult != WAIT_TIMEOUT)) {
        QString hexResult = QString("%1").arg(uint(aResult), 8, 16, QChar(ASCII::Zero)).toUpper();
        toLog(LogLevel::Error,
              QString("%1: WaitForSingleObject (ReadFile) has returned %2 = 0x%3 result.")
                  .arg(m_System_Name)
                  .arg(aResult)
                  .arg(hexResult));
        handleError("WaitForSingleObject");

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::read(QByteArray &aData, int aTimeout, int aMinSize) {
    aData.clear();

    if (!checkHandle() && !open()) {
        return false;
    }

    int readingTimeout = qMin(aTimeout, CAsyncSerialPortWin::ReadingTimeout);

    QTime timer;
    timer.start();

    while ((timer.elapsed() < aTimeout) && (aData.size() < aMinSize)) {
        if (!processReading(aData, readingTimeout)) {
            return false;
        }
    }

    if (m_DeviceIOLoging == ELoggingType::ReadWrite) {
        toLog(LogLevel::Normal,
              QString("%1: << {%2}").arg(m_ConnectedDeviceName).arg(aData.toHex().constData()));
    } else if (!aData.isEmpty() && !m_MaxReadingSize) {
        toLog(LogLevel::Debug, QString("%1 << %2").arg(m_System_Name).arg(aData.toHex().data()));
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::processReading(QByteArray &aData, int aTimeout) {
    QMutexLocker locker(&m_ReadMutex);

    DWORD result = 0;

    if (!checkReady() || !waitAsyncAction(result, aTimeout)) {
        return false;
    }

    if ((result == WAIT_OBJECT_0) || (m_Type == EPortTypes::VirtualCOM)) {
        m_ReadBytes = 0;
        BOOL wait = ((result == WAIT_OBJECT_0) || m_WaitResult) ? TRUE : FALSE;
        ::GetOverlappedResult(m_PortHandle, &m_ReadOverlapped, &m_ReadBytes, wait);

        SleepHelper::msleep(CAsyncSerialPortWin::VCOMReadingPause);
    }

    DWORD errors = 0;
    COMSTAT comstat = {0};

    if ((m_ReadEventMask & EV_RXCHAR) && BOOL_CALL(ClearComm_Error, &errors, &comstat)) {
        m_ReadingBuffer.fill(ASCII::NUL, comstat.cbInQue);

        if (comstat.cbInQue) {
            m_ReadBytes = 0;
            result = BOOL_CALL(
                ReadFile, &m_ReadingBuffer[0], comstat.cbInQue, &m_ReadBytes, &m_ReadOverlapped);
            int size = m_ReadBytes ? m_ReadBytes : m_ReadingBuffer.size();

            if (size) {
                aData.append(m_ReadingBuffer.data(), size);
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::write(const QByteArray &aData) {
    if (aData.isEmpty()) {
        toLog(LogLevel::Normal, m_ConnectedDeviceName + ": written data is empty.");
        return false;
    }

    if (!checkHandle() && !open()) {
        return false;
    }

    QMutexLocker locker(&m_WriteMutex);

    if (m_DeviceIOLoging != ELoggingType::None) {
        toLog(LogLevel::Normal,
              QString("%1: >> {%2}").arg(m_ConnectedDeviceName).arg(aData.toHex().constData()));
    } else if (!m_MaxReadingSize) {
        toLog(LogLevel::Debug, QString("%1 >> %2").arg(m_System_Name).arg(aData.toHex().data()));
    }

    if (!checkReady() || !clear()) {
        return false;
    }

    initializeOverlapped(m_WriteOverlapped);

    DWORD dataCount = aData.count();
    DWORD bytesWritten = 0;
    DWORD result =
        BOOL_CALL(WriteFile, aData.constData(), dataCount, &bytesWritten, &m_WriteOverlapped);

    if (result || (m_LastError == ERROR_IO_PENDING) || (m_LastError == ERROR_MORE_DATA)) {
        DWORD singlePacketSize =
            DWORD(m_DCB.ByteSize + int(m_DCB.fParity && (m_DCB.Parity != NOPARITY)) +
                  qCeil(double(m_DCB.StopBits) / 2 + 1));
        DWORD requiredTime = qCeil((aData.size() * singlePacketSize * 8 * 1000) / m_DCB.BaudRate);
        DWORD expectedTimeout = qMax(DWORD(IIOPort::DefaultWriteTimeout),
                                     DWORD(requiredTime * CAsyncSerialPortWin::KSafety));
        result = ::WaitForSingleObject(m_WriteOverlapped.hEvent,
                                       qMax(DWORD(IIOPort::DefaultWriteTimeout), expectedTimeout));

        switch (result) {
        case WAIT_OBJECT_0: {
            result = BOOL_CALL(GetOverlappedResult, &m_WriteOverlapped, &bytesWritten, TRUE);

            break;
        }
        case WAIT_TIMEOUT: {
            result = BOOL_CALL(GetOverlappedResult, &m_WriteOverlapped, &bytesWritten, FALSE);

            if (bytesWritten == dataCount) {
                result = true;
            } else {
                QString log = m_System_Name + ": WriteFile timed out";

                if (bytesWritten) {
                    log += QString(", bytes written = %1, size of data = %2")
                               .arg(bytesWritten)
                               .arg(dataCount);
                }

                toLog(LogLevel::Error, log);
            }

            break;
        }
        default: {
            QString hexResult =
                QString("%1").arg(uint(result), 8, 16, QChar(ASCII::Zero)).toUpper();
            toLog(LogLevel::Error,
                  QString("%1: WaitForSingleObject (WriteFile) has returned %2 = 0x%3 result.")
                      .arg(m_System_Name)
                      .arg(result)
                      .arg(hexResult));
            handleError("WaitForSingleObject");

            break;
        }
        }
    }

    if (result) {
        return result && (dataCount == bytesWritten);
    }

    clear();

    return false;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::setParameters(const TPortParameters &aParameters) {
    DCB newDCB(m_DCB);

    for (auto it = aParameters.begin(); it != aParameters.end(); ++it) {
        int value = it.value();

        switch (EParameters::Enum(it.key())) {
        case EParameters::BaudRate:
            if (!setBaudRate(EBaudRate::Enum(value)))
                return false;
            break;
        case EParameters::Parity:
            if (!setParity(EParity::Enum(value)))
                return false;
            break;
        case EParameters::RTS:
            if (!setRTS(ERTSControl::Enum(value)))
                return false;
            break;
        case EParameters::DTR:
            if (!setDTR(EDTRControl::Enum(value)))
                return false;
            break;
        case EParameters::ByteSize:
            if (!setByteSize(value))
                return false;
            break;
        case EParameters::StopBits:
            if (!setStopBits(EStopBits::Enum(value)))
                return false;
            break;
        }
    }

    setConfigParameter(CHardwareSDK::System_Name, m_System_Name);

    setConfigParameter(CHardware::Port::COM::BaudRate, int(m_DCB.BaudRate));
    setConfigParameter(CHardware::Port::COM::Parity, int(m_DCB.Parity));
    setConfigParameter(CHardware::Port::COM::RTS, int(m_DCB.fRtsControl));
    setConfigParameter(CHardware::Port::COM::DTR, int(m_DCB.fDtrControl));
    setConfigParameter(CHardware::Port::COM::ByteSize, int(m_DCB.ByteSize));
    setConfigParameter(CHardware::Port::COM::StopBits, int(m_DCB.StopBits));

    if ((newDCB == m_DCB) && (m_Type == EPortTypes::COM)) {
        return true;
    }

    return applyPortSettings();
}

//--------------------------------------------------------------------------------
void AsyncSerialPortWin::getParameters(TPortParameters &aParameters) {
    aParameters[EParameters::BaudRate] = getConfigParameter(CHardware::Port::COM::BaudRate).toInt();
    aParameters[EParameters::Parity] = getConfigParameter(CHardware::Port::COM::Parity).toInt();
    aParameters[EParameters::RTS] = getConfigParameter(CHardware::Port::COM::RTS).toInt();
    aParameters[EParameters::DTR] = getConfigParameter(CHardware::Port::COM::DTR).toInt();
    aParameters[EParameters::ByteSize] = getConfigParameter(CHardware::Port::COM::ByteSize).toInt();
    aParameters[EParameters::StopBits] = getConfigParameter(CHardware::Port::COM::StopBits).toInt();
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::applyPortSettings() {
    if (!checkHandle()) {
        return true;
    }

    QMutexLocker readLocker(&m_ReadMutex);
    QMutexLocker writeLocker(&m_WriteMutex);

    clear();

    // параметры порта, рекомендованные, но необязательные для некоторых устройств
    // m_DCB.XonLim = 0;
    // m_DCB.XoffLim = 0;
    // m_DCB.fAbortOnError = 1;

    return BOOL_CALL(SetComm_State, &m_DCB);
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::setBaudRate(EBaudRate::Enum aValue) {
    switch (aValue) {
    case EBaudRate::BR4800:
        m_DCB.BaudRate = CBR_4800;
        break;
    case EBaudRate::BR9600:
        m_DCB.BaudRate = CBR_9600;
        break;
    case EBaudRate::BR14400:
        m_DCB.BaudRate = CBR_14400;
        break;
    case EBaudRate::BR19200:
        m_DCB.BaudRate = CBR_19200;
        break;
    case EBaudRate::BR38400:
        m_DCB.BaudRate = CBR_38400;
        break;
    case EBaudRate::BR57600:
        m_DCB.BaudRate = CBR_57600;
        break;
    case EBaudRate::BR115200:
        m_DCB.BaudRate = CBR_115200;
        break;
    default:
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::setRTS(ERTSControl::Enum aValue) {
    switch (aValue) {
    case ERTSControl::Disable:
        m_DCB.fRtsControl = RTS_CONTROL_DISABLE;
        break;
    case ERTSControl::Enable:
        m_DCB.fRtsControl = RTS_CONTROL_ENABLE;
        break;
    case ERTSControl::Handshake:
        m_DCB.fRtsControl = RTS_CONTROL_HANDSHAKE;
        break;
    case ERTSControl::Toggle:
        m_DCB.fRtsControl = RTS_CONTROL_TOGGLE;
        break;
    default:
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::setDTR(EDTRControl::Enum aValue) {
    switch (aValue) {
    case EDTRControl::Disable:
        m_DCB.fDtrControl = DTR_CONTROL_DISABLE;
        break;
    case EDTRControl::Enable:
        m_DCB.fDtrControl = DTR_CONTROL_ENABLE;
        break;
    case EDTRControl::Handshake:
        m_DCB.fDtrControl = DTR_CONTROL_HANDSHAKE;
        break;
    default:
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::setByteSize(int aValue) {
    if (aValue >= 7 && aValue <= 9) {
        m_DCB.ByteSize = BYTE(aValue);
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::setStopBits(EStopBits::Enum aValue) {
    switch (aValue) {
    case EStopBits::One:
        m_DCB.StopBits = 0;
        break;
    case EStopBits::One5:
        m_DCB.StopBits = 1;
        break;
    case EStopBits::Two:
        m_DCB.StopBits = 2;
        break;
    default:
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::setParity(EParity::Enum aValue) {
    switch (aValue) {
    case EParity::Even:
        m_DCB.Parity = EVENPARITY;
        break;
    case EParity::Mark:
        m_DCB.Parity = MARKPARITY;
        break;
    case EParity::No:
        m_DCB.Parity = NOPARITY;
        break;
    case EParity::Odd:
        m_DCB.Parity = ODDPARITY;
        break;
    case EParity::Space:
        m_DCB.Parity = SPACEPARITY;
        break;
    default:
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortWin::deviceConnected() {
    TWinDeviceProperties winProperties = getDeviceProperties(m_Uuids, m_PathProperty, true);
    bool result = (winProperties.size() > m_WinProperties.size()) && !m_WinProperties.isEmpty();

    m_WinProperties = winProperties;

    if (result) {
        checkReady();

        setDeviceConfiguration(getDeviceConfiguration());
    }

    return result;
}

//--------------------------------------------------------------------------------
TWinDeviceProperties AsyncSerialPortWin::getDeviceProperties(const TUuids &aUuids,
                                                             DWORD aPropertyName,
                                                             bool aQuick,
                                                             TIOPortDeviceData *aData) {
    TWinDeviceProperties deviceProperties;
    QMap<QString, QStringList> sourceDeviceData;

    foreach (const QUuid &uuid, aUuids) {
        TWinDeviceProperties uidDeviceProperties;

        if (System_DeviceUtils::enumerateSystem_Devices(
                uuid, uidDeviceProperties, aPropertyName, aQuick)) {
            for (auto it = uidDeviceProperties.begin(); it != uidDeviceProperties.end(); ++it) {
                deviceProperties.insert(it.key(), it.value());
                sourceDeviceData[it.key()] << uuid.toString();
            }
        }
    }

    if (aUuids == CAsyncSerialPortWin::System::Uuids()) {
        TWinDeviceProperties deviceRegistryProperties =
            System_DeviceUtils::enumerateRegistryDevices(aQuick);
        System_DeviceUtils::mergeRegistryDeviceProperties(
            deviceProperties, deviceRegistryProperties, sourceDeviceData);
    }

    if (aData) {
        for (auto it = sourceDeviceData.begin(); it != sourceDeviceData.end(); ++it) {
            QString outKey = System_DeviceUtils::getDeviceOutKey(it.value());
            QString outData = System_DeviceUtils::getDeviceOutData(deviceProperties[it.key()].data);

            if (!outData.toLower().contains("mouse")) {
                aData->insert(outKey, outData);
            }
        }
    }

    return deviceProperties;
}

//--------------------------------------------------------------------------------
AsyncSerialPortWin::TData AsyncSerialPortWin::getSystem_Data(bool aForce) {
    static TData data;

    if (aForce || data.isEmpty()) {
        TWinDeviceProperties deviceProperties = getDeviceProperties(
            CAsyncSerialPortWin::System::Uuids(), CAsyncSerialPortWin::System::PathProperty);

        auto isMatched = [&](const TWinProperties &aProperties, const QStringList &aTags) -> bool {
            return std::find_if(
                       aProperties.begin(), aProperties.end(), [&](const QString &aValue) -> bool {
                           return std::find_if(
                                      aTags.begin(), aTags.end(), [&](const QString &aTag) -> bool {
                                          return aValue.contains(aTag, Qt::CaseInsensitive);
                                      }) != aTags.end();
                       }) != aProperties.end();
        };

        data.clear();
        QRegularExpression regExp("COM[0-9]+");

        for (auto it = deviceProperties.begin(); it != deviceProperties.end(); ++it) {
            int index = -1;

            do {
                index = regExp.match(it.key().capturedStart(), ++index);

                if (index != -1) {
                    EPortTypes::Enum portType =
                        isMatched(it->data, CAsyncSerialPortWin::Tags::Virtual())
                            ? EPortTypes::VirtualCOM
                            : (isMatched(it->data, CAsyncSerialPortWin::Tags::Emulator())
                                   ? EPortTypes::COMEmulator
                                   : EPortTypes::Unknown);
                    data.insert(regExp.capturedTexts()[0], portType);
                }
            } while (index != -1);
        }

        /*
        // раскомментировать, если для авто поиска порта по GUID-у (ам) будут какие-либо проблемы
        foreach(const QString & port, System_DeviceUtils::enumerateCOMPorts())
        {
                if (!data.contains(port))
                {
                        data.insert(port, true);
                }
        }
        */
    }

    return data;
}

//--------------------------------------------------------------------------------
QStringList AsyncSerialPortWin::enumerateSystem_Names() {
    return getSystem_Data().keys();
}

//--------------------------------------------------------------------------------
