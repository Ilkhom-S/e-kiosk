/* @file Принтер Citizen PPU-231. */

#include "CitizenPPU231.h"

#include <QtCore/qmath.h>

#include "CitizenPPU231Constants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
CitizenPPU231::CitizenPPU231() {
    // данные порта
    m_ModelData.data().clear();

    m_PortParameters[EParameters::BaudRate].clear();
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);

    // теги
    m_TagEngine = Tags::PEngine(new CCitizenPPU231::TagEngine());

    // данные устройства
    m_DeviceName = "Citizen PPU-231";
    m_LineSize = CCitizenPPU231::LineSize;
    m_AutoDetectable = false;
    m_PrintingStringTimeout = 100;
    m_RussianCodePage = '\x07';
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::isConnected() {
    return SerialPrinterBase::isConnected();
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::updateParameters() {
    if (!POSPrinter::updateParameters()) {
        return false;
    }

    int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();
    int feedingFactor = getConfigParameter(CHardware::Printer::Settings::FeedingFactor).toInt();
    lineSpacing = feedingFactor * qCeil(360 / (lineSpacing * 4.0));
    setConfigParameter(CHardware::Printer::FeedingAmount, lineSpacing);

    m_Verified = true;

    return true;
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt) {
    waitAvailable();

    return SerialPrinterBase::printReceipt(aLexemeReceipt);
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray answer;
    PollingExpector expector;
    auto poll = [&]() -> bool {
        return m_IOPort->write(CPOSPrinter::Command::GetPaperStatus) && getAnswer(answer, 50);
    };

    if (!expector.wait<bool>(
            poll,
            [&answer]() -> bool { return !answer.isEmpty(); },
            CCitizenPPU231::PaperStatusWaiting)) {
        return false;
    }

    for (int i = 0; i < 8; ++i) {
        if (answer[0] & (1 << i)) {
            aStatusCodes.insert(CCitizenPPU231::Statuses[i]);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::printBarcode(const QString &aBarcode) {
    // 1. В Qt 6 для кодирования текста (from_Unicode) используется QStringEncoder.
    // Так как m_Decoder в 2026 году — это std::shared_ptr<QStringDecoder>,
    // создаем энкодер с той же кодировкой.
    QByteArray encodedBarcode;

    if (m_Decoder) {
        // Создаем энкодер на основе имени кодировки декодера (напр. "IBM 866")
        QStringEncoder encoder(m_Decoder->name());
        // Используем оператор вызова (функтор) для кодирования
        encodedBarcode = encoder(aBarcode);
    } else {
        // Фолбэк на Latin1, если кодек не задан
        encodedBarcode = aBarcode.toLatin1();
    }

    // 2. Формируем команду печати штрих-кода.
    QByteArray barcodePrinting =
        CPOSPrinter::Command::Barcode::Print + CCitizenPPU231::Barcode::CodeSystem128 +
        CCitizenPPU231::Barcode::Code128Spec + encodedBarcode + CCitizenPPU231::Barcode::Postfix;

    // 3. Записываем в порт.
    // Предполагается, что prepareBarcodePrinting() возвращает QByteArray с префиксами.
    return m_IOPort->write(prepareBarcodePrinting() + barcodePrinting);
}

//--------------------------------------------------------------------------------
