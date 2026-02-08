/* @file Базовый класс для сценариев. */

#include <ScenarioEngine/Scenario.h>
#include <utility>

namespace GUI {

//---------------------------------------------------------------------------
Scenario::Scenario(QString aName, ILog *aLog) : m_Name(std::move(aName)), m_DefaultTimeout(0) {
    setLog(aLog);
    connect(&m_TimeoutTimer, SIGNAL(timeout()), SLOT(onTimeout()));

    m_TimeoutTimer.setSingleShot(true);
}

//---------------------------------------------------------------------------
Scenario::~Scenario() = default;

//---------------------------------------------------------------------------
QString Scenario::getName() const {
    return m_Name;
}

//---------------------------------------------------------------------------
void Scenario::resetTimeout() {
    if (m_TimeoutTimer.interval() > 0) {
        m_TimeoutTimer.start();
    }
}

//---------------------------------------------------------------------------
void Scenario::setStateTimeout(int aSec) {
    if (aSec > 0) {
        m_TimeoutTimer.setInterval(aSec * 1000);
        m_TimeoutTimer.start();
    }
}

//---------------------------------------------------------------------------
void Scenario::setLog(ILog *aLog) {
    if (!getLog()) {
        ILogable::setLog(aLog);
    }
}

} // namespace GUI

//---------------------------------------------------------------------------
