/* @file Базовый принтер с портовой реализацией протокола. */

#include <Hardware/Printers/PortPrinterBase.h>

template class PortPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>>;

//---------------------------------------------------------------------------
template <class T> PortPrinterBase<T>::PortPrinterBase() {
    this->m_IOMessageLogging = ELoggingType::ReadWrite;

    // кодек - удалено для Qt 6 совместимости
    // this->m_Codec = CodecByName[CHardware::Codepages::CP866].get();
}

//--------------------------------------------------------------------------------
template <class T> void PortPrinterBase<T>::finalizeInitialization() {
    this->addPortData();

    if (this->m_OperatorPresence) {
        if (!this->m_Connected) {
            this->processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
        } else {
            this->onPoll();
        }

        this->m_IOPort->close();
    } else {
        T::finalizeInitialization();
    }
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::isPossible(bool aOnline, QVariant aCommand) {
    bool result = T::isPossible(aOnline, aCommand);

    if (this->m_OperatorPresence && aOnline) {
        this->m_IOPort->close();
    }

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::print(const QStringList &aReceipt) {
    bool result = T::print(aReceipt);

    if (this->m_OperatorPresence) {
        this->m_IOPort->close();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::getAnswer(QByteArray &aAnswer, int aTimeout) const {
    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::Write));
    this->m_IOPort->setDeviceConfiguration(configuration);

    if (!this->m_IOPort->read(aAnswer, aTimeout)) {
        return false;
    }

    this->toLog(aAnswer.isEmpty() ? LogLevel::Warning : LogLevel::Normal,
                QString("%1: << {%2}").arg(this->m_DeviceName).arg(aAnswer.toHex().data()));

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void PortPrinterBase<T>::execTags(Tags::SLexeme &aTagLexeme, QVariant &aLine) {
    // 1. В Qt 6 для превращения QString в QByteArray используем QStringEncoder.
    // Если m_Decoder — это std::shared_ptr<QStringDecoder>, создаем энкодер на его основе.
    QByteArray data;

    if (this->m_Decoder) {
        // Создаем временный энкодер с тем же именем кодировки (напр. "IBM 866")
        QStringEncoder encoder(this->m_Decoder->name());
        data = encoder(aTagLexeme.data);
    } else {
        // Фолбэк на Latin1, если кодек не задан
        data = aTagLexeme.data.toLatin1();
    }

    // 2. В Qt 6 макрос foreach объявлен устаревшим. Используем стандартный range-based for.
    // m_TagEngine->groupsTypesByPrefix возвращает контейнер групп типов.
    auto groups = this->m_TagEngine->groupsTypesByPrefix(aTagLexeme.tags);

    for (const Tags::TTypes &types : groups) {
        // Получаем открывающий и закрывающий теги (байтовые последовательности)
        QByteArray openTag = this->m_TagEngine->getTag(types, Tags::Direction::Open);
        QByteArray closeTag = this->m_TagEngine->getTag(types, Tags::Direction::Close);

        // Обертываем данные тегами
        data = openTag + data + closeTag;
    }

    // 3. Формируем итоговую строку. В Qt 6 QVariant::toByteArray() безопасен.
    aLine = aLine.toByteArray() + data;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt) {
    this->m_ActualStringCount = 0;

    return T::printReceipt(aLexemeReceipt);
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::printOut(const SPrintingOutData &aPrintingOutData) {
    this->setLog(aPrintingOutData.log);
    this->setDeviceConfiguration(aPrintingOutData.configuration);
    this->m_Connected = true;
    this->m_Initialized = ERequestStatus::Success;

    if (!this->checkConnectionAbility()) {
        return false;
    }

    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging,
                         QVariant().fromValue(ELoggingType::ReadWrite));
    this->m_IOPort->setDeviceConfiguration(configuration);
    int feeding = this->getConfigParameter(CHardware::Printer::FeedingAmount).toInt();

    bool result =
        this->updateParametersOut() &&
        this->processReceipt(aPrintingOutData.receipt, aPrintingOutData.receiptProcessing);

    this->setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
    configuration.insert(CHardware::Port::IOLogging,
                         QVariant().fromValue(aPrintingOutData.IOMessageLogging));
    this->m_IOPort->setDeviceConfiguration(configuration);

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::printLine(const QVariant &aLine) {
    QByteArray request = aLine.toByteArray();

    if (this->m_LineFeed) {
        request += CPrinters::LineSpacer;
    }

    return this->printLine(request);
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::printLine(const QByteArray &aLine) {
    return this->m_IOPort->write(aLine);
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::cut() {
    if (!this->m_IOPort->write(
            this->getConfigParameter(CHardware::Printer::Commands::Cutting).toByteArray())) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": Failed to cut paper");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::present() {
    using namespace CHardware::Printer;

    QByteArray command = this->getConfigParameter(Commands::Presentation).toByteArray() +
                         char(this->getConfigParameter(Settings::PresentationLength).toInt());

    if (!this->m_IOPort->write(command)) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": Failed to present paper");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::push() {
    QByteArray command =
        this->getConfigParameter(CHardware::Printer::Commands::Pushing).toByteArray();

    return this->m_IOPort->write(command);
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::retract() {
    QByteArray command =
        this->getConfigParameter(CHardware::Printer::Commands::Retraction).toByteArray();

    return this->m_IOPort->write(command);
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::waitAvailable() {
    int timeout = CPortPrinter::PrintingStringTimeout * this->m_ActualStringCount;

    if (!timeout) {
        return true;
    }

    TStatusCodes statusCodes;
    auto condition = std::bind(&PortPrinterBase<T>::getStatus, this, std::ref(statusCodes));

    return PollingExpector().wait(condition, CPortPrinter::WaitingPollingInterval, timeout);
}

//--------------------------------------------------------------------------------
