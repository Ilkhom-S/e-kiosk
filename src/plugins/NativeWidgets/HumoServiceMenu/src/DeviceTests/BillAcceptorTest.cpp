/* @file Класс для тестирования купюроприемников. */

#include "BillAcceptorTest.h"

using namespace SDK::Driver;

//------------------------------------------------------------------------------
namespace CBillAcceptorTest {
const QString TestEscrow = QT_TRANSLATE_NOOP("BillAcceptorTest", "#test_escrow");
} // namespace CBillAcceptorTest

//------------------------------------------------------------------------------
BillAcceptorTest::BillAcceptorTest(IDevice *aDevice)
    : m_BillAcceptor(dynamic_cast<ICashAcceptor *>(aDevice)) {

    connect(&m_ErasingTimer, SIGNAL(timeout()), this, SLOT(onEraseMessage()));
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> BillAcceptorTest::getTestNames() const {
    return QList<QPair<QString, QString>>()
           << qMakePair(CBillAcceptorTest::TestEscrow, tr("#insert_bill"));
}

//------------------------------------------------------------------------------
bool BillAcceptorTest::run(const QString &aName) {
    if ((aName != CBillAcceptorTest::TestEscrow) || !m_BillAcceptor->isDeviceReady()) {
        return false;
    }

    m_BillAcceptor->subscribe(
        SDK::Driver::ICashAcceptor::EscrowSignal, this, SLOT(onEscrow(SDK::Driver::SPar)));
    m_BillAcceptor->subscribe(
        SDK::Driver::ICashAcceptor::StatusSignal,
        this,
        SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

    m_WorkingParList = m_BillAcceptor->getParList();
    TParList testParList(m_WorkingParList);

    for (auto &i : testParList) {
        i.enabled = true;
    }

    if (!isParListEqual(testParList, m_WorkingParList)) {
        m_BillAcceptor->setParList(testParList);
    }

    return m_BillAcceptor->setEnable(true);
}

//------------------------------------------------------------------------------
void BillAcceptorTest::stop() {
    m_ErasingTimer.stop();

    m_BillAcceptor->setEnable(false);
    TParList testParList = m_BillAcceptor->getParList();

    if (!isParListEqual(testParList, m_WorkingParList)) {
        m_BillAcceptor->setParList(m_WorkingParList);
    }

    m_BillAcceptor->unsubscribe(SDK::Driver::ICashAcceptor::EscrowSignal, this);
}

//------------------------------------------------------------------------------
bool BillAcceptorTest::isReady() {
    return m_BillAcceptor && m_BillAcceptor->isDeviceReady();
}

//------------------------------------------------------------------------------
bool BillAcceptorTest::hasResult() {
    return true;
}

//------------------------------------------------------------------------------
void BillAcceptorTest::onEscrow(const SPar &aPar) {
    QString message = QString("%1 %2").arg(tr("#bill_is_escrowed")).arg(aPar.nominal);

    for (auto &i : m_WorkingParList) {
        if ((i == aPar) && !i.enabled) {
            message += QString(" (%1)").arg(tr("#disabled"));
        }
    }

    m_ErasingTimer.stop();

    emit result("", message);

    m_ErasingTimer.start(CBillAcceptorTest::EscrowMessageTimeout);

    // Выбрасываем купюру
    m_BillAcceptor->reject();
}

//------------------------------------------------------------------------------
void BillAcceptorTest::onEraseMessage() {
    emit result("", " ");
}

//------------------------------------------------------------------------------
void BillAcceptorTest::onStatusChanged(EWarningLevel::Enum aWarningLevel,
                                       const QString &aTranslation,
                                       int aStatus) {
    if ((aStatus == ECashAcceptorStatus::Cheated) || (aWarningLevel == EWarningLevel::Error)) {
        m_ErasingTimer.stop();

        emit result("", aTranslation);
    }
}

//------------------------------------------------------------------------------
