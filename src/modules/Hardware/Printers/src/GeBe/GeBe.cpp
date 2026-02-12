/* @file Принтер Gebe. */

#include "GeBe.h"

#include "GeBEData.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
GeBe::GeBe() {
    // данные устройства
    m_DeviceName = "GeBE Compact";

    // данные порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200); // preferable for work
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);   // default
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);
    // TODO: добавить baudrate = 1200, 2400.

    m_PortParameters[EParameters::Parity].append(EParity::No); // default
    m_PortParameters[EParameters::Parity].append(EParity::Space);
    m_PortParameters[EParameters::Parity].append(EParity::Even);
    m_PortParameters[EParameters::Parity].append(EParity::Odd);

    // TODO: добавить stopbit = 2.

    m_LineSize = CGeBE::LineSize;

    // теги
    m_TagEngine = Tags::PEngine(new CGeBE::TagEngine());

    // кодек
    m_Decoder = CodecByName[CHardware::Codepages::Win1251];

    // данные устройства
    setConfigParameter(CHardware::Printer::FeedingAmount, 4);
    setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x43\x30");
}

//--------------------------------------------------------------------------------
bool GeBe::updateParameters() {
    return m_IOPort->write(QByteArray(CGeBE::Commands::Initialize) + CGeBE::Commands::SetFont);
}

//--------------------------------------------------------------------------------
bool GeBe::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray answer;

    if (!m_IOPort->write(CGeBE::Commands::GetStatus) || !m_IOPort->read(answer) ||
        answer.isEmpty()) {
        return false;
    }

    for (char i : answer) {
        aStatusCodes.insert(CGeBE::Statuses[i]);
    }

    return true;
}

//--------------------------------------------------------------------------------
