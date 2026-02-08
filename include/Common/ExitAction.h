/* @file Класс для выполнения функционала при выходе из области видимости. */
/* @brief Если отпадает необходимость в его вызове - вызвать reset(). */

#pragma once

#include <functional>

typedef std::function<void()> TVoidMethod;

//---------------------------------------------------------------------------
class ExitAction {
public:
    ExitAction(const TVoidMethod &aAction) : m_Action(aAction) {}
    ~ExitAction() {
        if (m_Action)
            m_Action();
    }

    bool reset(const TVoidMethod &aAction = TVoidMethod()) {
        m_Action = aAction;
        return true;
    }

private:
    TVoidMethod m_Action;
};

//---------------------------------------------------------------------------
