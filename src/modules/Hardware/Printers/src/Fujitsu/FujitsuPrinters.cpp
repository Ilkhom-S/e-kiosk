/* @file Принтер Fujitsu на контроллере Trentino FTP-609. */

#include "FujitsuPrinters.h"

#include "FujitsuPrinterData.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
FujitsuPrinter::FujitsuPrinter() {
    // данные устройства
    m_DeviceName = "Fujitsu FTP-609";

    // данные порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200); // default
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

    m_PortParameters[EParameters::Parity].append(EParity::No);

    // теги
    m_TagEngine = Tags::PEngine(new CFujitsu::TagEngine());

    // кодек
    m_Decoder = CodecByName[CHardware::Codepages::Win1251];

    // данные устройства
    setConfigParameter(CHardware::Printer::FeedingAmount, 4);
    setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x69");
}

//--------------------------------------------------------------------------------
bool FujitsuPrinter::isConnected() {
    QByteArray answer;

    if (!processCommand(CFujitsu::Commands::Identification, &answer)) {
        return false;
    }

    // TODO: переписка
    return (answer.size() >= 2);
}

//----------------------------------------------------------------------------
bool FujitsuPrinter::processCommand(const QByteArray &aCommand, QByteArray *aAnswer) {
    if (!m_IOPort->write(aCommand)) {
        return false;
    }

    QByteArray data;
    QByteArray &answer = aAnswer ? *aAnswer : data;

    // TODO: переписка
    if (isNeedAnswer(aCommand)) {
        SleepHelper::msleep(5);
        m_IOPort->read(answer);

        return !answer.isEmpty();
    }

    return true;
}

//----------------------------------------------------------------------------
bool FujitsuPrinter::isNeedAnswer(const QByteArray &aCommand) const {
    return (aCommand == CFujitsu::Commands::Identification) ||
           (aCommand == CFujitsu::Commands::Status) || (aCommand == CFujitsu::Commands::Voltage);
}

//--------------------------------------------------------------------------------
bool FujitsuPrinter::updateParameters() {
    return processCommand(CFujitsu::Commands::Initialize);
}

//--------------------------------------------------------------------------------
bool FujitsuPrinter::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray answer;

    if (!processCommand(CFujitsu::Commands::Status, &answer)) {
        return false;
    }

    for (int i = 0; i < 8; ++i) {
        if ((answer[0] & (1 << i)) == static_cast<int>(i != 7)) {
            aStatusCodes.insert(CFujitsu::Statuses[i]);
        }
    }

    // TODO: buffer full

    // TODO: переписка - иногда не приходит ответ
    if (!processCommand(CFujitsu::Commands::Voltage, &answer)) {
        return false;
    }

    double voltage = uchar(answer[0]) * 0.1;

    if (fabs(voltage - CFujitsu::Voltage::Nominal) > CFujitsu::Voltage::Delta) {
        aStatusCodes.insert(DeviceStatusCode::Error::PowerSupply);
    }

    return true;
}

//--------------------------------------------------------------------------------
