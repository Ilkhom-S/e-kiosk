/* @file Принтер AV-268. */

#include "AV268.h"

#include <QtCore/QElapsedTimer>

#include "AV268Constants.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
AV268::AV268() : m_Overflow(false), m_Initialize(false), m_ModelType(Unknown) {
    // данные устройства
    m_DeviceName = "SysFuture AV-268";

    // данные порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
    m_PortParameters[EParameters::Parity].append(EParity::No);

    // теги
    m_TagEngine = Tags::PEngine(new CAV268::TagEngine());

    // данные устройства
    setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x69");
    m_LineSize = CAV268::LineSize;
}

//--------------------------------------------------------------------------------
bool AV268::isConnected() {
    TStatusCodes statusCodes;
    bool statusOK = getStatus(statusCodes);
    bool modelTypeOK = (m_ModelType != Unknown) && (m_Initialize || (m_ModelType != Simple));

    if (m_ModelType == Plus) {
        m_DeviceName = "SysFuture AV-268 Plus";
    }

    return statusOK && modelTypeOK;
}

//--------------------------------------------------------------------------------
QStringList AV268::getModelList() {
    return QStringList() << "SysFuture AV-268"
                         << "SysFuture AV-268 Plus";
}

//--------------------------------------------------------------------------------
void AV268::initialize() {
    m_Initialize = true;

    SerialPrinterBase::initialize();
}

//--------------------------------------------------------------------------------
bool AV268::processCommand(const QByteArray &aCommand, QByteArray *aAnswer) {
    return m_IOPort->write(aCommand) && ((aAnswer == nullptr) || getAnswer(*aAnswer));
}

//--------------------------------------------------------------------------------
bool AV268::updateParameters() {
    bool result = processCommand(CAV268::Commands::Initialize);
    SleepHelper::msleep(CAV268::Timeouts::Initialize);

    return result;
}

//--------------------------------------------------------------------------------
bool AV268::printLine(const QByteArray &aString) {
    if (!processCommand(aString)) {
        return false;
    }

    QByteArray answer;
    getAnswer(answer);

    return !m_Overflow || waitBufferClearing();
}

//--------------------------------------------------------------------------------
bool AV268::waitBufferClearing() {
    QElapsedTimer clockTimer;
    clockTimer.start();

    do {
        toLog(LogLevel::Normal, "AV268: buffer overflow, wait...");
        QByteArray answer;
        SleepHelper::msleep(CAV268::Timeouts::Wait);
        getAnswer(answer);

        if (!m_Overflow) {
            return true;
        }
    } while (clockTimer.elapsed() < CAV268::Timeouts::Full);

    toLog(LogLevel::Normal, "AV268: Timeout for waiting buffer clearing");

    return false;
}

//--------------------------------------------------------------------------------
bool AV268::getAnswer(QByteArray &aAnswer, bool aNeedDelay) {
    if (aNeedDelay) {
        SleepHelper::msleep(CAV268::Timeouts::Default);
    }

    if (!SerialPrinterBase::getAnswer(aAnswer, 100)) {
        return false;
    }

    for (int i = 0; i < aAnswer.size(); ++i) {
        if (aAnswer.contains(ASCII::XOn) || aAnswer.contains(ASCII::XOff)) {
            m_Overflow =
                aAnswer.contains(ASCII::XOff); // C4800, QBool::operator const void *() const
            aAnswer.remove(i, 1);
            --i;
        }
    }

    return !aAnswer.isEmpty();
}

//--------------------------------------------------------------------------------
bool AV268::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray answer;

    if (!processCommand(CAV268::Commands::GetStatus, &answer)) {
        return false;
    }

    char status = answer[0];

    if ((status & CAV268::Statuses::NotConnected) != 0) {
        toLog(LogLevel::Error, "AV268: wrong byte returned");
        return false;
    }

    if ((status & CAV268::Statuses::HeadOverheat) != 0) {
        aStatusCodes.insert(Error::Temperature);
    }

    if ((status & CAV268::Statuses::NoPaper) != 0) {
        aStatusCodes.insert(Error::PaperEnd);
    }

    if ((status & CAV268::Statuses::HeadDoorOpened) != 0) {
        aStatusCodes.insert(DeviceStatusCode::Error::Mechanism_Position);
    }

    if ((status & CAV268::Statuses::UnknownError) != 0) {
        aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
    }

    if (m_ModelType != Simple) {
        bool result = processCommand(CAV268::Commands::GetSettings, &answer) &&
                      answer.startsWith(CAV268::Answers::GetSettings) && (answer.size() == 3);

        if (m_ModelType == Unknown) {
            m_ModelType = result ? Extended : Simple;
        }

        if (!result) {
            if (m_ModelType != Simple) {
                toLog(LogLevel::Error, "AV268: Wrong answer for settings request");
                return false;
            }

            return true;
        }

        char dipSettings = answer[2];

        if ((((dipSettings & CAV268::DIPSwitches::HalfHeight) != 0) &&
             ((~dipSettings & CAV268::DIPSwitches::DoubleHeight) != 0)) ||
            ((~dipSettings & CAV268::DIPSwitches::Cutter) != 0) ||
            (bool(dipSettings & CAV268::DIPSwitches::Presenter) != (m_ModelType == Plus))) {
            aStatusCodes.insert(DeviceStatusCode::Warning::WrongSwitchesConfig);
        }

        int factor = ((dipSettings & CAV268::DIPSwitches::DoubleWidth) != 0) ? 2 : 1;
        m_LineSize = CAV268::LineSize / factor;

        if (processCommand(CAV268::Commands::GetPresenterStatus, &answer) &&
            (answer.size() <= 10) && (answer.size() >= 2) &&
            answer.startsWith(CAV268::Answers::GetPresenterStatus)) {
            bool isAVPlus = ((dipSettings & CAV268::DIPSwitches::Presenter) != 0) &&
                            (answer.size() == 10) &&
                            ((answer[1] == CAV268::Answers::Presenter::Enable) ||
                             (answer[1] == CAV268::Answers::Presenter::Disable));

            if (isAVPlus) {
                m_DeviceName = "SysFuture AV-268 Plus";
                m_ModelType = Plus;
            } else {
                m_DeviceName = "SysFuture AV-268";
                m_ModelType = Extended;
            }

            if ((answer[1] == CAV268::Answers::Presenter::Enable) ||
                (answer[1] == CAV268::Answers::Presenter::Disable) ||
                (answer[1] == CAV268::Answers::Presenter::NotAvaiable)) {
                ushort errorCode = answer[3] | answer[2] << 8;

                if ((errorCode & CAV268::Statuses::PaperJam) != 0) {
                    aStatusCodes.insert(Error::PaperJam);
                }

                if ((errorCode & CAV268::Statuses::Presenter) != 0) {
                    aStatusCodes.insert(Error::Presenter);
                }

                if ((errorCode & CAV268::Statuses::PowerSupply) != 0) {
                    aStatusCodes.insert(DeviceStatusCode::Error::PowerSupply);
                }
            } else {
                toLog(LogLevel::Error, "AV268: Wrong answer for presenter request");

                if (m_ModelType == Plus) {
                    aStatusCodes.insert(Error::Presenter);
                }
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AV268::receiptProcessing() {
    bool result = SerialPrinterBase::receiptProcessing();
    SleepHelper::msleep(CPortPrinter::PrintingStringTimeout * m_ActualStringCount);
    waitAvailable();

    return result;
}

//--------------------------------------------------------------------------------
