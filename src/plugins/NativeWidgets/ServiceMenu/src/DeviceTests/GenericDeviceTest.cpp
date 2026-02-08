/* @file Обобщенный тест устройств. */

// boost

#include "GenericDeviceTest.h"

#include <SDK/Drivers/IDevice.h>

#include <boost/bind/bind.hpp>

namespace CGenericDeviceTest {
const QString GenericTest = QT_TRANSLATE_NOOP("GenericDeviceTest", "#generic_test");
} // namespace CGenericDeviceTest

//------------------------------------------------------------------------------
GenericDeviceTest::GenericDeviceTest(SDK::Driver::IDevice *aDevice) : m_Device(aDevice) {
    connect(&m_Result, SIGNAL(finished()), this, SLOT(onTestFinished()));
}

//------------------------------------------------------------------------------
GenericDeviceTest::~GenericDeviceTest() {
    m_Result.waitForFinished();
}

//------------------------------------------------------------------------------
bool GenericDeviceTest::run(const QString &aTestName) {
    if (aTestName != CGenericDeviceTest::GenericTest) {
        return false;
    }

    QFuture<void> future =
        QtConcurrent::run([capture0 = m_Device.data()] { capture0->initialize(); });
    m_Result.setFuture(future);

    return true;
}

//------------------------------------------------------------------------------
void GenericDeviceTest::stop() {}

//------------------------------------------------------------------------------
bool GenericDeviceTest::isReady() {
    return false;
}

//------------------------------------------------------------------------------
bool GenericDeviceTest::hasResult() {
    return false;
}

//------------------------------------------------------------------------------
void GenericDeviceTest::onTestFinished() {
    emit result(m_GenericTest, tr("#ok"));
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> GenericDeviceTest::getTestNames() const {
    return QList<QPair<QString, QString>>()
           << qMakePair(CGenericDeviceTest::GenericTest, QString());
}

//------------------------------------------------------------------------------
