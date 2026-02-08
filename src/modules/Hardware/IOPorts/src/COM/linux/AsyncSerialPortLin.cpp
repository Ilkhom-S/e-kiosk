/* @file Реализация Linux-асинхронного последовательного порта. */

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/qmath.h>

#include <Hardware/IOPorts/COM/linux/AsyncSerialPortLin.h>

#include "Hardware/Common/SafePerformer.h"
#include "SysUtils/ISysUtils.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
AsyncSerialPortLin::AsyncSerialPortLin() : m_PortFd(-1), m_Exist(false), m_MaxReadingSize(0) {
    m_Type = EPortTypes::COM;
    m_System_Names = enumerateSystem_Names();
    setOpeningTimeout(CAsyncSerialPort::OpeningTimeout);
}

//--------------------------------------------------------------------------------
void AsyncSerialPortLin::initialize() {
    // Enumerate available serial ports in /dev/
    QDir devDir("/dev");
    QStringList filters;
    filters << "ttyS*" << "ttyUSB*" << "ttyACM*";

    QStringList portData;
    for (const QString &filter : filters) {
        QStringList ports = devDir.entryList(QStringList() << filter, QDir::System);
        for (const QString &port : ports) {
            portData << "/dev/" + port;
        }
    }

    adjustData(portData, QStringList());
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::release() {
    if (m_PortFd >= 0) {
        ::close(m_PortFd);
        m_PortFd = -1;
    }
    return IOPortBase::release();
}

//--------------------------------------------------------------------------------
void AsyncSerialPortLin::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    IOPortBase::setDeviceConfiguration(aConfiguration);

    if (!m_Exist && !m_System_Name.isEmpty()) {
        checkExistence();
    }
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::open() {
    if (opened()) {
        return true;
    }

    if (!checkReady()) {
        return false;
    }

    return perform_Open();
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::close() {
    if (!opened()) {
        return true;
    }

    if (m_PortFd >= 0) {
        ::close(m_PortFd);
        m_PortFd = -1;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::clear() {
    if (!checkReady()) {
        return false;
    }

    // For Linux, we can use tcflush to clear buffers
    if (m_PortFd >= 0) {
        tcflush(m_PortFd, TCIOFLUSH);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::setParameters(const TPortParameters &aParameters) {
    if (!checkReady()) {
        return false;
    }

    // Basic parameter setting for Linux serial ports
    if (m_PortFd >= 0) {
        struct termios options;
        tcgetattr(m_PortFd, &options);

        // Set baud rate (simplified - only common rates)
        speed_t baudRate = B9600; // default
        switch (aParameters.baudRate) {
        case EBaudRate::BR4800:
            baudRate = B4800;
            break;
        case EBaudRate::BR9600:
            baudRate = B9600;
            break;
        case EBaudRate::BR19200:
            baudRate = B19200;
            break;
        case EBaudRate::BR38400:
            baudRate = B38400;
            break;
        case EBaudRate::BR57600:
            baudRate = B57600;
            break;
        case EBaudRate::BR115200:
            baudRate = B115200;
            break;
        default:
            break;
        }

        cfsetispeed(&options, baudRate);
        cfsetospeed(&options, baudRate);

        // Set data bits
        options.c_cflag &= ~CSIZE;
        switch (aParameters.byteSize) {
        case 5:
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            options.c_cflag |= CS8;
            break;
        }

        // Set parity
        switch (aParameters.parity) {
        case EParity::No:
            options.c_cflag &= ~PARENB;
            break;
        case EParity::Odd:
            options.c_cflag |= PARENB;
            options.c_cflag |= PARODD;
            break;
        case EParity::Even:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            break;
        default:
            options.c_cflag &= ~PARENB;
            break;
        }

        // Set stop bits
        switch (aParameters.stopBits) {
        case EStopBits::One:
            options.c_cflag &= ~CSTOPB;
            break;
        case EStopBits::Two:
            options.c_cflag |= CSTOPB;
            break;
        default:
            options.c_cflag &= ~CSTOPB;
            break;
        }

        // Apply settings
        tcsetattr(m_PortFd, TCSANOW, &options);
    }

    return true;
}

//--------------------------------------------------------------------------------
void AsyncSerialPortLin::getParameters(TPortParameters &aParameters) {
    // Return default parameters since we don't store them
    aParameters.baudRate = EBaudRate::BR9600;
    aParameters.parity = EParity::No;
    aParameters.byteSize = 8;
    aParameters.stopBits = EStopBits::One;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::read(QByteArray &aData, int aTimeout, int aMinSize) {
    if (!checkReady() || !opened()) {
        return false;
    }

    return processReading(aData, aTimeout);
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::write(const QByteArray &aData) {
    if (!checkReady() || !opened() || m_PortFd < 0) {
        return false;
    }

    ssize_t written = ::write(m_PortFd, aData.constData(), aData.size());
    return written == aData.size();
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::deviceConnected() {
    return checkExistence();
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::opened() {
    return m_PortFd >= 0;
}

//--------------------------------------------------------------------------------
void AsyncSerialPortLin::changePerformingTimeout(const QString &aContext,
                                                 int aTimeout,
                                                 int aPerformingTime) {
    Q_UNUSED(aContext)
    Q_UNUSED(aTimeout)
    Q_UNUSED(aPerformingTime)
    // Implementation can be added if needed
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::checkExistence() {
    if (m_System_Name.isEmpty()) {
        return false;
    }

    m_Exist = QFile::exists(m_System_Name);
    return m_Exist;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::perform_Open() {
    if (m_System_Name.isEmpty()) {
        return false;
    }

    m_PortFd = ::open(m_System_Name.toLocal8Bit().constData(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_PortFd < 0) {
        return false;
    }

    // Configure for non-blocking
    fcntl(m_PortFd, F_SETFL, 0);

    return true;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::processReading(QByteArray &aData, int aTimeout) {
    if (m_PortFd < 0) {
        return false;
    }

    fd_set readfds;
    struct timeval tv;

    tv.tv_sec = aTimeout / 1000;
    tv.tv_usec = (aTimeout % 1000) * 1000;

    FD_ZERO(&readfds);
    FD_SET(m_PortFd, &readfds);

    int result = select(m_PortFd + 1, &readfds, NULL, NULL, &tv);
    if (result > 0) {
        char buffer[1024];
        ssize_t bytesRead = ::read(m_PortFd, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            aData.append(buffer, bytesRead);
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::checkReady() {
    return m_Exist;
}

//--------------------------------------------------------------------------------
bool AsyncSerialPortLin::isExist() {
    return checkExistence();
}

//--------------------------------------------------------------------------------
QStringList AsyncSerialPortLin::enumerateSystem_Names() {
    QDir devDir("/dev");
    QStringList filters;
    filters << "ttyS*" << "ttyUSB*" << "ttyACM*";

    QStringList ports;
    for (const QString &filter : filters) {
        QStringList found = devDir.entryList(QStringList() << filter, QDir::System);
        for (const QString &port : found) {
            ports << "/dev/" + port;
        }
    }

    return ports;
}

//--------------------------------------------------------------------------------