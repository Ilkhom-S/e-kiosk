/* @file Виртуальный диспенсер. */

#include "VirtualCashDispenser.h"

#include <algorithm>

#include "Hardware/Dispensers/DispenserData.h"
#include "Hardware/Dispensers/DispenserStatusCodes.h"

VirtualDispenser::VirtualDispenser() : m_JammedItem(0), m_NearEndCount(0) {
    m_DeviceName = "Virtual cash dispenser";
}

//---------------------------------------------------------------------------------
void VirtualDispenser::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    TVirtualDispenser::setDeviceConfiguration(aConfiguration);

    m_JammedItem = aConfiguration.value(CHardware::Dispenser::JammedItem, m_JammedItem).toInt();
    m_NearEndCount =
        aConfiguration.value(CHardware::Dispenser::NearEndCount, m_NearEndCount).toInt();
    m_units = aConfiguration.value(CHardware::Dispenser::Units, m_units).toInt();
}

//---------------------------------------------------------------------------
void VirtualDispenser::applyUnitList() {
    moveToThread(&m_Thread);

    START_IN_WORKING_THREAD(applyUnitList)

    if (!m_unitConfigData.isEmpty()) {
        adjustUnitList(true);
    }

    moveToThread(qApp->thread());
}

//--------------------------------------------------------------------------------
void VirtualDispenser::performDispense(int aUnit, int aItems) {
    moveToThread(&m_Thread);

    if (!isWorkingThread()) {
        QMetaObject::invokeMethod(
            this, "performDispense", Qt::QueuedConnection, Q_ARG(int, aUnit), Q_ARG(int, aItems));

        return;
    }

    moveToThread(qApp->thread());

    int dispensedItems = 0;

    if (!m_StatusCodes.contains(DispenserStatusCode::Error::Jammed)) {
        dispensedItems = qMin(aItems, m_unitData[aUnit]);

        if ((m_JammedItem != 0) && (dispensedItems >= m_JammedItem)) {
            m_StatusCodes.insert(DispenserStatusCode::Error::Jammed);
            dispensedItems = m_JammedItem - 1;
        }

        m_unitData[aUnit] -= dispensedItems;
    }

    const unsigned long delayMs =
        static_cast<unsigned long>(CVirtualDispenser::Item_DispenseDelay) *
        static_cast<unsigned long>(dispensedItems);
    SleepHelper::msleep(delayMs);

    if (m_unitData[aUnit] != 0) {
        m_unitData[aUnit]--;
        toLog(LogLevel::Warning,
              QString("%1: Send rejected 1 item from %2 unit").arg(m_DeviceName).arg(aUnit));

        emit rejected(aUnit, 1);
    }

    if (m_unitData[aUnit] == 0) {
        toLog(LogLevel::Warning, QString("%1: Send emptied unit %2").arg(m_DeviceName).arg(aUnit));

        emit unitEmpty(aUnit);
    }

    toLog(LogLevel::Normal,
          QString("%1: Send dispensed %2 item(s) from %3 unit")
              .arg(m_DeviceName)
              .arg(dispensedItems)
              .arg(aUnit));

    emit dispensed(aUnit, dispensedItems);

    onPoll();
}

//--------------------------------------------------------------------------------
void VirtualDispenser::checkUnitStatus(TStatusCodes &aStatusCodes, int aUnit) {
    if ((m_unitData.size() > aUnit) && (m_unitData[aUnit] <= m_NearEndCount)) {
        aStatusCodes.insert(CDispenser::StatusCodes::Data[aUnit].nearEmpty);
    }

    TVirtualDispenser::checkUnitStatus(aStatusCodes, aUnit);
}

//--------------------------------------------------------------------------------
void VirtualDispenser::filterKeyEvent(int aKey, const Qt::KeyboardModifiers &aModifiers) {
    if (aModifiers.testFlag(Qt::AltModifier)) {
        switch (aKey) {
        case Qt::Key_F1: {
            changeStatusCode(DispenserStatusCode::Error::Unit0Opened);
            break;
        }
        case Qt::Key_F2: {
            changeStatusCode(DispenserStatusCode::Error::Unit1Opened);
            break;
        }
        case Qt::Key_F3: {
            changeStatusCode(DispenserStatusCode::Error::Unit2Opened);
            break;
        }
        case Qt::Key_F4: {
            changeStatusCode(DispenserStatusCode::Error::Unit3Opened);
            break;
        }

        case Qt::Key_F7: {
            changeStatusCode(DispenserStatusCode::Error::RejectingOpened);
            break;
        }
        case Qt::Key_F8: {
            changeStatusCode(DispenserStatusCode::Error::Jammed);
            break;
        }
        case Qt::Key_F9: {
            changeStatusCode(DeviceStatusCode::Error::NotAvailable);
            break;
        }
        default:
            break;
        }
    } else if (aModifiers.testFlag(Qt::ShiftModifier)) {
        switch (aKey) {
        case Qt::Key_F1: {
            changeStatusCode(DispenserStatusCode::Warning::Unit0Empty);
            break;
        }
        case Qt::Key_F2: {
            changeStatusCode(DispenserStatusCode::Warning::Unit1Empty);
            break;
        }
        case Qt::Key_F3: {
            changeStatusCode(DispenserStatusCode::Warning::Unit2Empty);
            break;
        }
        case Qt::Key_F4: {
            changeStatusCode(DispenserStatusCode::Warning::Unit3Empty);
            break;
        }
        default:
            break;
        }
    }
}

//--------------------------------------------------------------------------------
