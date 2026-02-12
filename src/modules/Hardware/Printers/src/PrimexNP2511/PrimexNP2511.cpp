/* @file Принтер PrimexNP2511. */

#include "PrimexNP2511.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
PrimexNP2511::PrimexNP2511() {
    // данные устройства
    m_DeviceName = "Primex NP-2511";

    // данные порта
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR115200); // default
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
    m_PortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

    m_PortParameters[EParameters::Parity].append(EParity::No); // default
    m_PortParameters[EParameters::Parity].append(EParity::Even);
    m_PortParameters[EParameters::Parity].append(EParity::Odd);

    // теги
    m_TagEngine = Tags::PEngine(new CPrimexNP2511::TagEngine());

    // кодек
    m_Decoder = CodecByName[CHardware::Codepages::Win1251];

    // данные устройства
    m_DeviceName = "Primex NP-2511";
    setConfigParameter(CHardware::Printer::FeedingAmount, 2);
    setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x69");
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::isConnected() {
    CPrimexNP2511::TDeviceParameters deviceParameters = CPrimexNP2511::CDeviceParameters().data();
    bool result = true;

    for (auto it = deviceParameters.begin(); it != deviceParameters.end(); ++it) {
        QString data;

        if (!processDeviceData(it, data)) {
            result = false;
            break;
        }

        setDeviceParameter(it->description, data);
    }

    return result;
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::backFeed(int aCount) {
    return (aCount == 0) ||
           m_IOPort->write(QByteArray(CPrimexNP2511::Commands::BackFeed) + char(aCount));
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::updateParameters() {
    // TODO: вынести выбор кодовой страницы в настройки плагина
    return m_IOPort->write(QByteArray(CPrimexNP2511::Commands::Initialize) +
                           CPrimexNP2511::Commands::SetCyrillicPage);
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt) {
    // TODO: проверить возможности эжектора и вынести все соответствующие параметры в настройки
    // плагина
    return m_IOPort->write(CPrimexNP2511::Commands::ClearDispenser) &&
           backFeed(CPrimexNP2511::BackFeedCount) &&
           m_IOPort->write(CPrimexNP2511::Commands::AutoRetract) &&
           SerialPrinterBase::printReceipt(aLexemeReceipt);
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::printBarcode(const QString &aBarcode) {
    // 1. В Qt 6 для кодирования текста в байты (fromUnicode) используется QStringEncoder.
    // m_Decoder в 2026 году — это std::shared_ptr<QStringDecoder>.
    QByteArray barcodeData;

    if (m_Decoder) {
        // Создаем энкодер с кодировкой текущего декодера
        QStringEncoder encoder(m_Decoder->name());
        barcodeData = encoder(aBarcode);
    } else {
        // Запасной вариант (Latin1), если декодер не задан
        barcodeData = aBarcode.toLatin1();
    }

    // 2. Формируем запрос. Используем конструктор QByteArray для цепочки склейки.
    // ASCII::NUL в конце обязателен для протокола Primex.
    QByteArray request =
        QByteArray(CPrimexNP2511::Commands::BarCode::SetHRIPosition) +
        CPrimexNP2511::Commands::BarCode::SetHeight + CPrimexNP2511::Commands::BarCode::SetFont +
        CPrimexNP2511::PrinterBarCodeFontSize + CPrimexNP2511::Commands::BarCode::SetWidth +
        CPrimexNP2511::Commands::BarCode::Print + barcodeData + ASCII::NUL;

    // 3. Записываем данные в порт.
    return m_IOPort->write(request);
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::getStatus(TStatusCodes &aStatusCodes) {
    QByteArray answer;

    if (!m_IOPort->write(CPrimexNP2511::Commands::GetStatus) || !m_IOPort->read(answer) ||
        (answer.size() != 1)) {
        return false;
    }

    for (int i = 0; i < 8; ++i) {
        if ((answer[0] & (1 << i)) != 0) {
            aStatusCodes.insert(CPrimexNP2511::Statuses[i]);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::processDeviceData(const CPrimexNP2511::TDeviceParametersIt &aIt,
                                     QString &aData) {
    QByteArray answer;

    if (!m_IOPort->write(CPrimexNP2511::Commands::PrinterInfo + aIt.key()) ||
        !m_IOPort->read(answer)) {
        return false;
    }

    QString errorLog = QString("Failed to get %1 due to ").arg(aIt->description);

    if (answer.size() < aIt->size) {
        toLog(LogLevel::Error,
              errorLog + QString("length of the packet is too small, %1 < %2.")
                             .arg(answer.size())
                             .arg(aIt->size));
        return false;
    }

    if (answer[0] != ASCII::Full) {
        toLog(LogLevel::Error,
              errorLog + QString("first byte = %1 is wrong, need %2")
                             .arg(ProtocolUtils::toHexLog(answer[0]))
                             .arg(ProtocolUtils::toHexLog(ASCII::Full)));
        return false;
    }

    if (answer[1] != aIt.key()) {
        toLog(LogLevel::Error,
              QString("second byte = %1 is wrong, need %2 = command")
                  .arg(ProtocolUtils::toHexLog(answer[1]))
                  .arg(ProtocolUtils::toHexLog(aIt.key())));
        return false;
    }

    aData = answer.mid(2);

    return true;
}

//--------------------------------------------------------------------------------
