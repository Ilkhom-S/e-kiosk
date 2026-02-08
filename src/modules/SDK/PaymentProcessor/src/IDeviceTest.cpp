/* @file Dummy implementation for IDeviceTest interface moc compilation. */

// Include the interface header for moc processing

#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace PaymentProcessor {

// Dummy implementation of pure virtual methods
QList<QPair<QString, QString>> IDeviceTest::getTestNames() const {
    return {};
}

bool IDeviceTest::run(const QString & /*unused*/) {
    return false;
}

void IDeviceTest::stop() {}

bool IDeviceTest::isReady() {
    return false;
}

bool IDeviceTest::hasResult() {
    return false;
}

} // namespace PaymentProcessor
} // namespace SDK