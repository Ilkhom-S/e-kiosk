/* @file Виртуальный купюроприемник. */

#include "VirtualCashAcceptor.h"

#include <QtCore/QReadLocker>
#include <QtCore/QVector>

using namespace SDK::Driver;

//---------------------------------------------------------------------------------
VirtualCashAcceptor::VirtualCashAcceptor() : m_NotesPerEscrow(1) {
    m_DeviceName = "Virtual cash acceptor";
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::updateParameters() {
    m_Ready = true;
    setEnable(false);

    m_CurrencyError = processParTable();

    return m_CurrencyError == ECurrencyError::OK;
}

//---------------------------------------------------------------------------------
void VirtualCashAcceptor::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    TVirtualCashAcceptor::setDeviceConfiguration(aConfiguration);

    m_NotesPerEscrow =
        aConfiguration.value(CHardware::VirtualCashAcceptor::NotesPerEscrow, m_NotesPerEscrow)
            .toUInt();
}

//--------------------------------------------------------------------------------
void VirtualCashAcceptor::testStack(double aAmount) {
    SDK::Driver::TParList pars;

    pars << SDK::Driver::SPar(
        aAmount, getConfigParameter(CHardwareSDK::CashAcceptor::SystemCurrencyId).toInt());

    emit stacked(pars);

    emit status(EWarningLevel::OK,
                QString("Test stacked %1").arg(aAmount, 0, 'f', 2),
                ECashAcceptorStatus::OK);
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::loadParTable() {
    int currencyId = getConfigParameter(CHardwareSDK::CashAcceptor::SystemCurrencyId).toInt();

    m_EscrowParTable.add(Qt::Key_F1, SPar(10, currencyId));
    m_EscrowParTable.add(Qt::Key_F2, SPar(50, currencyId));
    m_EscrowParTable.add(Qt::Key_F3, SPar(100, currencyId));
    m_EscrowParTable.add(Qt::Key_F4, SPar(500, currencyId));
    m_EscrowParTable.add(Qt::Key_F5, SPar(1000, currencyId));
    m_EscrowParTable.add(Qt::Key_F6, SPar(5000, currencyId));

    // TODO: при необходимости кастомизировать для евро, доллара и тенге

    return true;
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::setEnable(bool aEnabled) {
    if (!m_Ready && aEnabled) {
        return false;
    }

    using namespace BillAcceptorStatusCode::Normal;

    m_StatusCodes.remove(Enabled);
    m_StatusCodes.remove(Disabled);
    m_StatusCodes.insert(aEnabled ? Enabled : Disabled);

    if (aEnabled) {
        emit status(EWarningLevel::OK, "Enabled", ECashAcceptorStatus::Enabled);
    } else {
        emit status(EWarningLevel::OK, "Disabled", ECashAcceptorStatus::Disabled);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::leaveEscrow(int aStatusCode) {
    bool escrow = m_StatusCodes.contains(BillAcceptorStatusCode::BillOperation::Escrow);
    m_StatusCodes.remove(BillAcceptorStatusCode::BillOperation::Escrow);

    if (!m_StackedStatusCodes.isEmpty()) {
        onPoll();
        m_StackedStatusCodes.clear();

        return false;
    }

    if (!m_Ready || !escrow || !m_StatusCodes.contains(BillAcceptorStatusCode::Normal::Enabled)) {
        onPoll();

        return false;
    }

    blinkStatusCode(aStatusCode);

    return true;
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::stack() {
    if (!m_StackedStatusCodes.isEmpty()) {
        m_StatusCodes += m_StackedStatusCodes;
    }

    return leaveEscrow(BillAcceptorStatusCode::BillOperation::Stacked);
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::reject() {
    return leaveEscrow(BillAcceptorStatusCode::Busy::Returned);
}

//--------------------------------------------------------------------------------
void VirtualCashAcceptor::filterKeyEvent(int aKey, const Qt::KeyboardModifiers &aModifiers) {
    if (aModifiers.testFlag(Qt::ControlModifier)) {
        switch (aKey) {
        case Qt::Key_F1:
        case Qt::Key_F2:
        case Qt::Key_F3:
        case Qt::Key_F4:
        case Qt::Key_F5:
        case Qt::Key_F6: {
            if (m_EscrowParTable.data().contains(aKey)) {
                m_EscrowPars = QVector<SPar>(m_NotesPerEscrow, m_EscrowParTable[aKey]).toList();
                m_StatusCodes.insert(BillAcceptorStatusCode::BillOperation::Escrow);
            }

            break;
        }
        case Qt::Key_F7: {
            blinkStatusCode(BillAcceptorStatusCode::Reject::Unknown);
            break;
        } // режект
        case Qt::Key_F8: {
            changeStatusCode(BillAcceptorStatusCode::MechanicFailure::JammedInValidator);
            break;
        } // купюра замята
        case Qt::Key_F9: {
            changeStatusCode(DeviceStatusCode::Error::NotAvailable);
            break;
        } // недоступен питания
        case Qt::Key_F10: {
            changeStatusCode(BillAcceptorStatusCode::MechanicFailure::StackerOpen);
            break;
        } // стекер снят
        case Qt::Key_F11: {
            changeStatusCode(BillAcceptorStatusCode::MechanicFailure::StackerFull);
            break;
        } // стекер полон
        case '*': {
            blinkStatusCode(BillAcceptorStatusCode::Warning::Cheated);
            break;
        } // мошенство
        }
    } else if (aModifiers.testFlag(Qt::AltModifier)) {
        switch (aKey) {
        case Qt::Key_F8: {
            m_StackedStatusCodes.insert(BillAcceptorStatusCode::MechanicFailure::JammedInStacker);
            break;
        } // Купюра замята
        case Qt::Key_F9: {
            m_StackedStatusCodes.insert(DeviceStatusCode::Error::NotAvailable);
            break;
        } // Недоступен питания
        case Qt::Key_F10: {
            m_StackedStatusCodes.insert(BillAcceptorStatusCode::MechanicFailure::StackerOpen);
            break;
        } // Стекер снят
        case Qt::Key_F11: {
            m_StackedStatusCodes.insert(BillAcceptorStatusCode::MechanicFailure::StackerFull);
            break;
        } // Стекер полон
        }
    }
}

//---------------------------------------------------------------------------------
