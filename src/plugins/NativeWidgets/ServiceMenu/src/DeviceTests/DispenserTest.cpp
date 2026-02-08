/* @file Класс для тестирования диспенсеров. */

#include "DispenserTest.h"

#include <QtCore/QStringList>

#include <SDK/PaymentProcessor/Core/ICore.h>

#include <utility>

//------------------------------------------------------------------------------
namespace CDispenserTest {
const QString TestDispense = QT_TRANSLATE_NOOP("DispenserTest", "#dispense");
} // namespace CDispenserTest

//------------------------------------------------------------------------------
DispenserTest::DispenserTest(SDK::Driver::IDevice *aDevice,
                             QString aConfigurationName,
                             SDK::PaymentProcessor::ICore *aCore)
    : m_Dispenser(dynamic_cast<SDK::Driver::IDispenser *>(aDevice)),
      m_ConfigurationName(std::move(aConfigurationName)), m_Core(aCore) {}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> DispenserTest::getTestNames() const {
    return QList<QPair<QString, QString>>()
           << qMakePair(CDispenserTest::TestDispense, tr("#dispense"));
}

//------------------------------------------------------------------------------
bool DispenserTest::run(const QString &aName) {
    m_Results.clear();

    if (aName == CDispenserTest::TestDispense) {
        if (m_Dispenser->isDeviceReady()) {
            m_Dispenser->subscribe(
                SDK::Driver::IDispenser::DispensedSignal, this, SLOT(onDispensed(int, int)));
            m_Dispenser->subscribe(
                SDK::Driver::IDispenser::RejectedSignal, this, SLOT(onRejected(int, int)));

            // Выдаём из диспенсера по одной купюре из каждого ящика.
            for (int i = 0; i < m_Dispenser->units(); ++i) {
                m_Dispenser->dispense(i, 1);
            }

            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void DispenserTest::stop() {
    // m_HID->unsubscribe(SDK::Driver::IHID::DataSignal, this);
}

//------------------------------------------------------------------------------
bool DispenserTest::isReady() {
    return m_Dispenser && m_Dispenser->isDeviceReady();
}

//------------------------------------------------------------------------------
bool DispenserTest::hasResult() {
    return true;
}

//------------------------------------------------------------------------------
void DispenserTest::onDispensed(int aCashUnit, int aCount) {
    m_Results
        << QString("%1 from slot %2 count: %3").arg(tr("#dispensed")).arg(aCashUnit).arg(aCount);

    emit result("", m_Results.join("\n"));
}

//------------------------------------------------------------------------------
void DispenserTest::onRejected(int aCashUnit, int aCount) {
    m_Results
        << QString("%1 from slot %2 count: %3").arg(tr("#rejected")).arg(aCashUnit).arg(aCount);

    emit result("", m_Results.join("\n"));
}

//------------------------------------------------------------------------------
