/* @file Реализация платформо-независимого асинхронного последовательного порта. */

#include <QtCore/QDebug>

#include <Hardware/IOPorts/AsyncSerialPort.h>

const char GeneralRS232[] = "ACPI";

#ifdef Q_OS_WIN
// Windows implementation
#include <Hardware/IOPorts/COM/windows/AsyncSerialPortWin.h>

class WindowsImpl : public AsyncSerialPort::ISerialPortImpl {
public:
    WindowsImpl() : m_impl() {}

    QStringList enumerateSystemNames() override {
        return AsyncSerialPortWin::enumerateSystemNames();
    }

    void initialize() override { m_impl.initialize(); }

    void setDeviceConfiguration(const QVariantMap &aConfiguration) override {
        m_impl.setDeviceConfiguration(aConfiguration);
    }

    bool release() override { return m_impl.release(); }

    bool open() override { return m_impl.open(); }

    bool close() override { return m_impl.close(); }

    bool clear() override { return m_impl.clear(); }

    bool setParameters(const SDK::Driver::TPortParameters &aParameters) override {
        return m_impl.setParameters(aParameters);
    }

    void getParameters(SDK::Driver::TPortParameters &aParameters) override {
        m_impl.getParameters(aParameters);
    }

    bool read(QByteArray &aData, int aTimeout, int aMinSize) override {
        return m_impl.read(aData, aTimeout, aMinSize);
    }

    bool write(const QByteArray &aData) override { return m_impl.write(aData); }

    bool deviceConnected() override { return m_impl.deviceConnected(); }

    bool opened() override { return m_impl.opened(); }

    bool isExist() override { return m_impl.isExist(); }

    void
    changePerformingTimeout(const QString &aContext, int aTimeout, int aPerformingTime) override {
        m_impl.changePerformingTimeout(aContext, aTimeout, aPerformingTime);
    }

private:
    AsyncSerialPortWin m_impl;
};
#endif // Q_OS_WIN

//--------------------------------------------------------------------------------
#ifndef Q_OS_WIN
// Linux/Unix implementation - stub for macOS and other Unix systems
class LinuxImpl : public AsyncSerialPort::ISerialPortImpl {
public:
    LinuxImpl() {}

    QStringList enumerateSystemNames() override {
        return QStringList(); // No serial ports available
    }

    void initialize() override {
        // Stub implementation
    }

    void setDeviceConfiguration(const QVariantMap &aConfiguration) override {
        Q_UNUSED(aConfiguration)
    }

    bool release() override { return true; }

    bool open() override {
        return false; // Not supported
    }

    bool close() override { return true; }

    bool clear() override { return true; }

    bool setParameters(const SDK::Driver::TPortParameters &aParameters) override {
        Q_UNUSED(aParameters)
        return false; // Not supported
    }

    void getParameters(SDK::Driver::TPortParameters &aParameters) override { Q_UNUSED(aParameters) }

    bool read(QByteArray &aData, int aTimeout, int aMinSize) override {
        Q_UNUSED(aData)
        Q_UNUSED(aTimeout)
        Q_UNUSED(aMinSize)
        return false; // Not supported
    }

    bool write(const QByteArray &aData) override {
        Q_UNUSED(aData)
        return false; // Not supported
    }

    bool deviceConnected() override {
        return false; // Not supported
    }

    bool opened() override {
        return false; // Not supported
    }

    bool isExist() override {
        return false; // Not supported
    }

    void
    changePerformingTimeout(const QString &aContext, int aTimeout, int aPerformingTime) override {
        Q_UNUSED(aContext)
        Q_UNUSED(aTimeout)
        Q_UNUSED(aPerformingTime)
    }
};
#endif // !Q_OS_WIN
AsyncSerialPort::AsyncSerialPort() {
#ifdef Q_OS_WIN
    m_impl = new WindowsImpl();
#else
    m_impl = new LinuxImpl();
#endif
}

AsyncSerialPort::~AsyncSerialPort() {
    delete m_impl;
    m_impl = nullptr;
}

QStringList AsyncSerialPort::enumerateSystemNames() {
#ifdef Q_OS_WIN
    return AsyncSerialPortWin::enumerateSystemNames();
#else
    return QStringList(); // No serial ports available on this platform
#endif
}

void AsyncSerialPort::initialize() {
    if (m_impl)
        m_impl->initialize();
}

void AsyncSerialPort::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    if (m_impl)
        m_impl->setDeviceConfiguration(aConfiguration);
}

bool AsyncSerialPort::release() {
    return m_impl ? m_impl->release() : false;
}

bool AsyncSerialPort::open() {
    return m_impl ? m_impl->open() : false;
}

bool AsyncSerialPort::close() {
    return m_impl ? m_impl->close() : false;
}

bool AsyncSerialPort::clear() {
    return m_impl ? m_impl->clear() : false;
}

bool AsyncSerialPort::setParameters(const SDK::Driver::TPortParameters &aParameters) {
    return m_impl ? m_impl->setParameters(aParameters) : false;
}

void AsyncSerialPort::getParameters(SDK::Driver::TPortParameters &aParameters) {
    if (m_impl)
        m_impl->getParameters(aParameters);
}

bool AsyncSerialPort::read(QByteArray &aData, int aTimeout, int aMinSize) {
    return m_impl ? m_impl->read(aData, aTimeout, aMinSize) : false;
}

bool AsyncSerialPort::write(const QByteArray &aData) {
    return m_impl ? m_impl->write(aData) : false;
}

bool AsyncSerialPort::deviceConnected() {
    return m_impl ? m_impl->deviceConnected() : false;
}

bool AsyncSerialPort::opened() {
    return m_impl ? m_impl->opened() : false;
}

bool AsyncSerialPort::isExist() {
    return m_impl ? m_impl->isExist() : false;
}

void AsyncSerialPort::changePerformingTimeout(const QString &aContext,
                                              int aTimeout,
                                              int aPerformingTime) {
    if (m_impl)
        m_impl->changePerformingTimeout(aContext, aTimeout, aPerformingTime);
}

//--------------------------------------------------------------------------------