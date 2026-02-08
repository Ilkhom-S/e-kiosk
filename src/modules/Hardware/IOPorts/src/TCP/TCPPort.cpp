/* @file TCP-порт. */

#include <QtCore/QMetaType>
#include <QtCore/QRegularExpression>

#include <Hardware/IOPorts/TCPPort.h>

namespace CTCPPort {
const char AddressMaskLog[] = "0xx.0xx.0xx.0xx";
const char AntiNaglePing[] = "\xFF";
} // namespace CTCPPort

TCPPort::TCPPort()
    : m_State(QAbstractSocket::UnconnectedState), m_Error(QAbstractSocket::UnknownSocketError),
      m_SocketGuard(QMutex::Recursive) {
    m_Type = SDK::Driver::EPortTypes::TCP;
    setOpeningTimeout(CTCPPort::OpeningTimeout);

    moveToThread(&m_Thread);
    m_Thread.start();

    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    connect(this,
            SIGNAL(invoke(TBoolMethod, bool *)),
            SLOT(onInvoke(TBoolMethod, bool *)),
            Qt::BlockingQueuedConnection);
}

//--------------------------------------------------------------------------------
bool TCPPort::invokeMethod(TBoolMethod aMethod) {
    bool result;

    isWorkingThread() ? onInvoke(aMethod, &result) : emit invoke(aMethod, &result);

    return result;
}

//--------------------------------------------------------------------------------
void TCPPort::onInvoke(TBoolMethod aMethod, bool *aResult) {
    *aResult = aMethod();
}

//--------------------------------------------------------------------------------
void TCPPort::initialize() {}

//--------------------------------------------------------------------------------
bool TCPPort::opened() {
    return PERFORM_IN_THREAD(performOpened);
}

//--------------------------------------------------------------------------------
bool TCPPort::performOpened() {
    return m_Socket && (m_Socket->state() == QAbstractSocket::ConnectedState);
}

//--------------------------------------------------------------------------------
bool TCPPort::open() {
    return PERFORM_IN_THREAD(performOpen);
}

//--------------------------------------------------------------------------------
bool TCPPort::performOpen() {
    if (!m_Socket) {
        m_Socket = PSocket(new QTcpSocket());

        connect(m_Socket.data(),
                SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                SLOT(onStateChanged(QAbstractSocket::SocketState)));
        connect(m_Socket.data(),
                SIGNAL(error(QAbstractSocket::SocketError)),
                SLOT(onErrorChanged(QAbstractSocket::SocketError)));
        connect(m_Socket.data(), SIGNAL(readyRead()), SLOT(onReadyRead()));
    }

    if (m_Socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    QString IP = getConfigParameter(CHardwareSDK::Port::TCP::IP).toString();

    auto writeLog = [&](const QString &aLog) {
        toLog((aLog == m_LastErrorLog) ? LogLevel::Debug : LogLevel::Error, aLog);
        m_LastErrorLog = aLog;
    };

    if (!QRegularExpression(CTCPPort::AddressMask).match(IP).hasMatch()) {
        writeLog(QString("Failed to open the TCP socket with IP = %1, need like %2")
                     .arg(IP)
                     .arg(CTCPPort::AddressMaskLog));
        return false;
    }

    m_Socket->abort();

    uint portNumber = getConfigParameter(CHardwareSDK::Port::TCP::Number).toUInt();
    m_Socket->connectToHost(IP, portNumber);
    QString portLogName = QString("TCP socket %1:%2").arg(IP).arg(portNumber);

    if (!m_Socket->waitForConnected(m_OpeningTimeout)) {
        writeLog(QString("Failed to open the %1 due to timeout = %2 is expired")
                     .arg(portLogName)
                     .arg(m_OpeningTimeout));
        return false;
    }

    QAbstractSocket::SocketState state = m_Socket->state();

    if (state != QAbstractSocket::ConnectedState) {
        writeLog(QString("Failed to open the %1 due to wrong state = %2")
                     .arg(portLogName)
                     .arg(int(state)));
        return false;
    }

    m_Socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_Socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    toLog(LogLevel::Normal, portLogName + " is opened");

    return true;
}

//--------------------------------------------------------------------------------
bool TCPPort::close() {
    QMutexLocker locker(&m_SocketGuard);

    m_LastErrorLog.clear();
    bool result = PERFORM_IN_THREAD(performClose);

    SleepHelper::msleep(CTCPPort::CloseningPause);

    return result;
}

//--------------------------------------------------------------------------------
bool TCPPort::performClose() {
    if (!m_Socket || (m_Socket->state() == QAbstractSocket::UnconnectedState)) {
        return true;
    }

    m_Socket->close();

    if (m_Socket->state() == QAbstractSocket::UnconnectedState) {
        toLog(LogLevel::Normal, "TCP socket is closed");
        return true;
    }

    if (!m_Socket->waitForDisconnected(CTCPPort::CloseningTimeout)) {
        toLog(LogLevel::Error,
              QString("Failed to close the TCP socket due to timeout = %1 is expired")
                  .arg(CTCPPort::CloseningTimeout));
        return false;
    }

    QAbstractSocket::SocketState state = m_Socket->state();

    if (state != QAbstractSocket::UnconnectedState) {
        toLog(LogLevel::Error,
              QString("Failed to close the TCP socket due to wrong state = %1").arg(int(state)));
        return false;
    }

    toLog(LogLevel::Normal, QString("TCP socket is closed, state = %1").arg(int(state)));

    return true;
}

//--------------------------------------------------------------------------------
void TCPPort::onStateChanged(QAbstractSocket::SocketState aState) {
    toLog(LogLevel::Debug, QString("Socket state has changed = %1").arg(int(aState)));

    m_State = aState;
}

//--------------------------------------------------------------------------------
void TCPPort::onErrorChanged(QAbstractSocket::SocketError aError) {
    if (aError != QAbstractSocket::SocketTimeoutError) {
        toLog(LogLevel::Debug, QString("Socket error has changed = %1").arg(int(aError)));

        m_Error = aError;
    }
}

//--------------------------------------------------------------------------------
void TCPPort::onReadyRead() {
    QMutexLocker locker(&m_DataFromGuard);

    m_DataFrom += m_Socket->readAll();
}

//--------------------------------------------------------------------------------
bool TCPPort::checkReady() {
    return PERFORM_IN_THREAD(performCheckReady);
}

//--------------------------------------------------------------------------------
bool TCPPort::performCheckReady() {
    return m_Socket && ((m_Socket->state() == QAbstractSocket::ConnectedState) || open());
}

//--------------------------------------------------------------------------------
bool TCPPort::read(QByteArray &aData, int aTimeout, int aMinSize) {
    QMutexLocker locker(&m_SocketGuard);

    return PERFORM_IN_THREAD(performRead, std::ref(aData), aTimeout, aMinSize);
}

//--------------------------------------------------------------------------------
bool TCPPort::performRead(QByteArray &aData, int aTimeout, int aMinSize) {
    aData.clear();

    if (!checkReady()) {
        return false;
    }

    QTime waitingTimer;
    waitingTimer.start();

    while ((waitingTimer.elapsed() < aTimeout) && (aData.size() < aMinSize)) {
        m_Socket->waitForReadyRead(CTCPPort::ReadingTimeout);

        QMutexLocker locker(&m_DataFromGuard);
        {
            aData += m_DataFrom;

            m_DataFrom.clear();
        }

        if (aData == CTCPPort::AntiNaglePing) {
            aData.clear();
        }
    }

    if (m_DeviceIOLoging == ELoggingType::ReadWrite) {
        toLog(LogLevel::Normal,
              QString("%1: << {%2}").arg(m_ConnectedDeviceName).arg(aData.toHex().constData()));
    }

    return true;
}

//--------------------------------------------------------------------------------
bool TCPPort::write(const QByteArray &aData) {
    QMutexLocker locker(&m_SocketGuard);

    return PERFORM_IN_THREAD(performWrite, std::ref(aData));
}

//--------------------------------------------------------------------------------
bool TCPPort::performWrite(const QByteArray &aData) {
    if (aData.isEmpty()) {
        toLog(LogLevel::Normal, m_ConnectedDeviceName + ": written data is empty.");
        return false;
    }

    if (!checkReady()) {
        return false;
    }

    if (m_DeviceIOLoging != ELoggingType::None) {
        toLog(LogLevel::Normal,
              QString("%1: >> {%2}").arg(m_ConnectedDeviceName).arg(aData.toHex().constData()));
    }

    if ((m_Socket->state() != QAbstractSocket::ConnectedState) && !open()) {
        return false;
    }

    int bytesWritten = int(m_Socket->write(aData));
    int actualSize = aData.size();

    if (bytesWritten != actualSize) {
        toLog(LogLevel::Normal,
              m_ConnectedDeviceName + QString(": %1 bytes instead of %2 bytes have been written.")
                                         .arg(bytesWritten)
                                         .arg(actualSize));
        return false;
    }

    if (!m_Socket->waitForBytesWritten()) {
        toLog(LogLevel::Debug, "Failed to wait writing bytes");

        if (!m_Socket->waitForBytesWritten()) {
            toLog(LogLevel::Error, "Failed twice to wait writing bytes");
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool TCPPort::deviceConnected() {
    return open();
};

//--------------------------------------------------------------------------------
bool TCPPort::isExist() {
    return m_State != QAbstractSocket::UnconnectedState;
}

//--------------------------------------------------------------------------------
