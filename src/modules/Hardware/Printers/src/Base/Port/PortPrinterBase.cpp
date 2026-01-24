/* @file Базовый принтер с портовой реализацией протокола. */

// System
#include <Hardware/Printers/PortPrinterBase.h>

template class PortPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>>;

//---------------------------------------------------------------------------
template <class T> PortPrinterBase<T>::PortPrinterBase() {
    this->mIOMessageLogging = ELoggingType::ReadWrite;

    // кодек - удалено для Qt 6 совместимости
    // this->mCodec = CodecByName[CHardware::Codepages::CP866].get();
}

//--------------------------------------------------------------------------------
template <class T> void PortPrinterBase<T>::finalizeInitialization() {
    this->addPortData();

    if (this->mOperatorPresence) {
        if (!this->mConnected) {
            this->processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
        } else {
            this->onPoll();
        }

        this->mIOPort->close();
    } else {
        T::finalizeInitialization();
    }
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::isPossible(bool aOnline, QVariant aCommand) {
    bool result = T::isPossible(aOnline, aCommand);

    if (this->mOperatorPresence && aOnline) {
        this->mIOPort->close();
    }

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::print(const QStringList &aReceipt) {
    bool result = T::print(aReceipt);

    if (this->mOperatorPresence) {
        this->mIOPort->close();
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::getAnswer(QByteArray &aAnswer, int aTimeout) const {
    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::Write));
    this->mIOPort->setDeviceConfiguration(configuration);

    if (!this->mIOPort->read(aAnswer, aTimeout)) {
        return false;
    }

    this->toLog(aAnswer.isEmpty() ? LogLevel::Warning : LogLevel::Normal,
                QString("%1: << {%2}").arg(this->mDeviceName).arg(aAnswer.toHex().data()));

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void PortPrinterBase<T>::execTags(Tags::SLexeme &aTagLexeme, QVariant &aLine) {
    // 1. В Qt 6 для превращения QString в QByteArray используем QStringEncoder.
    // Если mDecoder — это std::shared_ptr<QStringDecoder>, создаем энкодер на его основе.
    QByteArray data;

    if (this->mDecoder) {
        // Создаем временный энкодер с тем же именем кодировки (напр. "IBM 866")
        QStringEncoder encoder(this->mDecoder->name());
        data = encoder(aTagLexeme.data);
    } else {
        // Фолбэк на Latin1, если кодек не задан
        data = aTagLexeme.data.toLatin1();
    }

    // 2. В Qt 6 макрос foreach объявлен устаревшим. Используем стандартный range-based for.
    // mTagEngine->groupsTypesByPrefix возвращает контейнер групп типов.
    auto groups = this->mTagEngine->groupsTypesByPrefix(aTagLexeme.tags);

    for (const Tags::TTypes &types : groups) {
        // Получаем открывающий и закрывающий теги (байтовые последовательности)
        QByteArray openTag = this->mTagEngine->getTag(types, Tags::Direction::Open);
        QByteArray closeTag = this->mTagEngine->getTag(types, Tags::Direction::Close);

        // Обертываем данные тегами
        data = openTag + data + closeTag;
    }

    // 3. Формируем итоговую строку. В Qt 6 QVariant::toByteArray() безопасен.
    aLine = aLine.toByteArray() + data;
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt) {
    this->mActualStringCount = 0;

    return T::printReceipt(aLexemeReceipt);
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::printOut(const SPrintingOutData &aPrintingOutData) {
    this->setLog(aPrintingOutData.log);
    this->setDeviceConfiguration(aPrintingOutData.configuration);
    this->mConnected = true;
    this->mInitialized = ERequestStatus::Success;

    if (!this->checkConnectionAbility()) {
        return false;
    }

    QVariantMap configuration;
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::ReadWrite));
    this->mIOPort->setDeviceConfiguration(configuration);
    int feeding = this->getConfigParameter(CHardware::Printer::FeedingAmount).toInt();

    bool result = this->updateParametersOut() &&
                  this->processReceipt(aPrintingOutData.receipt, aPrintingOutData.receiptProcessing);

    this->setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
    configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(aPrintingOutData.IOMessageLogging));
    this->mIOPort->setDeviceConfiguration(configuration);

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::printLine(const QVariant &aLine) {
    QByteArray request = aLine.toByteArray();

    if (this->mLineFeed) {
        request += CPrinters::LineSpacer;
    }

    return this->printLine(request);
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::printLine(const QByteArray &aLine) {
    return this->mIOPort->write(aLine);
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::cut() {
    if (!this->mIOPort->write(this->getConfigParameter(CHardware::Printer::Commands::Cutting).toByteArray())) {
        this->toLog(LogLevel::Error, this->mDeviceName + ": Failed to cut paper");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::present() {
    using namespace CHardware::Printer;

    QByteArray command = this->getConfigParameter(Commands::Presentation).toByteArray() +
                         char(this->getConfigParameter(Settings::PresentationLength).toInt());

    if (!this->mIOPort->write(command)) {
        this->toLog(LogLevel::Error, this->mDeviceName + ": Failed to present paper");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::push() {
    QByteArray command = this->getConfigParameter(CHardware::Printer::Commands::Pushing).toByteArray();

    return this->mIOPort->write(command);
}

//---------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::retract() {
    QByteArray command = this->getConfigParameter(CHardware::Printer::Commands::Retraction).toByteArray();

    return this->mIOPort->write(command);
}

//--------------------------------------------------------------------------------
template <class T> bool PortPrinterBase<T>::waitAvailable() {
    int timeout = CPortPrinter::PrintingStringTimeout * this->mActualStringCount;

    if (!timeout) {
        return true;
    }

    TStatusCodes statusCodes;
    auto condition = std::bind(&PortPrinterBase<T>::getStatus, this, std::ref(statusCodes));

    return PollingExpector().wait(condition, CPortPrinter::WaitingPollingInterval, timeout);
}

//--------------------------------------------------------------------------------
