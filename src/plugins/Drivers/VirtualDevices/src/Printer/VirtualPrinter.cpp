/* @file Виртуальный диспенсер. */

#include "VirtualPrinter.h"

#include <algorithm>

#include "Hardware/Dispensers/DispenserData.h"
#include "Hardware/Dispensers/DispenserStatusCodes.h"

VirtualPrinter::VirtualPrinter() {
    mDeviceName = "Virtual printer";
    mMaxBadAnswers = 0;
}

//--------------------------------------------------------------------------------
bool VirtualPrinter::isDeviceReady(bool aOnline) {
    if (aOnline) {
        SleepHelper::msleep(CVirtualPrinter::Delay::OnlineReadyChecking);
    }

    MutexLocker locker(&mExternalMutex);

    return mStatusCollection.isEmpty(SDK::Driver::EWarningLevel::Error);
}

//--------------------------------------------------------------------------------
bool VirtualPrinter::print(const QStringList &aReceipt) {
    if (!isDeviceReady(false)) {
        toLog(LogLevel::Normal, mDeviceName + ": Failed to print receipt");
        return false;
    }

    Tags::TLexemeReceipt lexemeReceipt;
    makeLexemeReceipt(aReceipt, lexemeReceipt);
    QStringList receipt;

    for (int i = 0; i < lexemeReceipt.size(); ++i) {
        QString line;

        for (int j = 0; j < lexemeReceipt[i].size(); ++j) {
            for (int k = 0; k < lexemeReceipt[i][j].size(); ++k) {
                line += lexemeReceipt[i][j][k].data;
            }
        }

        receipt << line;
    }

    if (!receipt.isEmpty()) {
        toLog(LogLevel::Normal, "Printing receipt:\n" + receipt.join("\n"));
        SleepHelper::msleep(CVirtualPrinter::Delay::Printing);
        toLog(LogLevel::Normal, "Receipt has been printed successfully");
    } else {
        toLog(LogLevel::Normal, mDeviceName + ": receipt is empty");
    }

    return true;
}

//--------------------------------------------------------------------------------
bool VirtualPrinter::updateParameters() {
    return true;
}

//--------------------------------------------------------------------------------
void VirtualPrinter::filterKeyEvent(int /*aKey*/, const Qt::KeyboardModifiers & /*aModifiers*/) {
    // Virtual printer doesn't handle key events
}
