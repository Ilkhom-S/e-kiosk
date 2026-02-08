/* @file Класс для тестирования купюроприемников. */

#include "CoinAcceptorTest.h"

namespace CCoinAcceptorTest {
const QString TestStacked = QT_TRANSLATE_NOOP("CoinAcceptorTest", "#test_stacked");
} // namespace CCoinAcceptorTest

//------------------------------------------------------------------------------
CoinAcceptorTest::CoinAcceptorTest(SDK::Driver::IDevice *aDevice)
    : m_CoinAcceptor(dynamic_cast<SDK::Driver::ICashAcceptor *>(aDevice)) {}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> CoinAcceptorTest::getTestNames() const {
    return QList<QPair<QString, QString>>()
           << qMakePair(CCoinAcceptorTest::TestStacked, tr("#insert_coin"));
}

//------------------------------------------------------------------------------
bool CoinAcceptorTest::run(const QString &aName) {
    if (aName == CCoinAcceptorTest::TestStacked) {
        if (m_CoinAcceptor->isDeviceReady() && m_CoinAcceptor->setEnable(true)) {
            m_CoinAcceptor->subscribe(SDK::Driver::ICashAcceptor::StackedSignal,
                                      this,
                                      SLOT(onStacked(SDK::Driver::TParList)));
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void CoinAcceptorTest::stop() {
    m_CoinAcceptor->setEnable(false);
    m_CoinAcceptor->unsubscribe(SDK::Driver::ICashAcceptor::StackedSignal, this);
}

//------------------------------------------------------------------------------
bool CoinAcceptorTest::isReady() {
    return m_CoinAcceptor && m_CoinAcceptor->isDeviceReady();
}

//------------------------------------------------------------------------------
bool CoinAcceptorTest::hasResult() {
    return true;
}

//------------------------------------------------------------------------------
void CoinAcceptorTest::onStacked(SDK::Driver::TParList aNotes) {
    emit result("",
                QString("%1 %2 %3")
                    .arg(tr("#coin_is_stacked"))
                    .arg(aNotes.first().nominal)
                    .arg(aNotes.first().currency));
}

//------------------------------------------------------------------------------
