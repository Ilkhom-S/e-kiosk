/* @file POS-принтер. */

#include "POSPrinter.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QSet>

#include <algorithm>

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

POSPrinters::TModelIds POSPrinters::ModelData::mModelIds;

//--------------------------------------------------------------------------------
template class POSPrinter<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template <class T> POSPrinter<T>::POSPrinter() : T(), mModelID(0), mPrintingStringTimeout(0) {
    // данные устройства
    this->mDeviceName = CPOSPrinter::DefaultName;
    this->setConfigParameter(CHardware::Printer::FeedingAmount, 4);
    this->setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x69");

    this->mModelData.setDefault(
        POSPrinters::SModelData(CPOSPrinter::DefaultName, false, "Default"));

    this->mOverflow = false;
    this->mRussianCodePage = CPOSPrinter::RussianCodePage;
}

//--------------------------------------------------------------------------------
template <class T> bool POSPrinter<T>::isConnected() {
    if (!this->mIOPort->write(CPOSPrinter::Command::Initialize)) {
        return false;
    }

    if (!this->waitReady(CPOSPrinter::AvailableWaiting)) {
        return false;
    }

    /*
    bool errors = std::find_if(statusCodes.begin(), statusCodes.end(), [&] (int aStatusCode) -> bool
            { return mStatusCodesSpecification->value(aStatusCode).warningLevel ==
    SDK::Driver::EWarningLevel::Error; })
    != statusCodes.end();

    //TODO: выяснить причины отсутствия ответа на данную команду для некоторых принтеров
    QByteArray answer;

    if (!errors && (!mIOPort->write(CPOSPrinter::Command::GetPaperStatus) || !getAnswer(answer,
    CPOSPrinter::Timeouts::Status) || answer.isEmpty()))
    {
            return false;
    }
    */
    QByteArray answer;

    if (!this->getModelId(answer)) {
        this->checkVerifying();

        return false;
    }

    bool onlyDefaultModels = this->isOnlyDefaultModels();
    this->mVerified = false;

    if (answer.isEmpty()) {
        bool isLoading = !this->isAutoDetecting();
        bool result = onlyDefaultModels || isLoading;
        LogLevel::Enum logLevel = result ? LogLevel::Warning : LogLevel::Error;

        this->toLog(
            logLevel,
            QString("Unknown POS printer has detected, it is in error state (possibly), the plugin "
                    "contains %1only default models and the type of searching is %2")
                .arg(onlyDefaultModels ? "" : "not ")
                .arg(isLoading ? "loading" : "autodetecting"));

        if (!onlyDefaultModels && isLoading) {
            this->mDeviceName = this->getConfigParameter(CHardwareSDK::ModelName).toString();
        }

        this->mVerified = result;

        if (result) {
            this->mTagEngine->data() = POSPrinters::CommonParameters.tagEngine.constData();
        }

        return result;
    }

    char modelId = this->parseModelId(answer);

    if (!this->mModelData.getModelIds().contains(modelId)) {
        LogLevel::Enum logLevel = onlyDefaultModels ? LogLevel::Warning : LogLevel::Error;
        this->toLog(logLevel,
                    QString("Unknown POS printer has detected, it model id = %1 is unknown and the "
                            "plugin contains "
                            "%2only default models")
                        .arg(ProtocolUtils::toHexLog(modelId))
                        .arg(onlyDefaultModels ? "" : "not "));

        if (onlyDefaultModels) {
            this->mTagEngine->data() = POSPrinters::CommonParameters.tagEngine.constData();
        }

        return onlyDefaultModels;
    }

    QString name = this->mModelData[modelId].name;
    QString description = this->mModelData[modelId].description;

    if (!description.isEmpty()) {
        name += QString(" (%1)").arg(description);
    }

    if (!this->mModelData.data().keys().contains(modelId)) {
        this->mModelCompatibility = false;
        this->toLog(LogLevel::Error, name + " is detected, but not supported by this pligin");

        this->mTagEngine->data() = this->mParameters.tagEngine.data();

        return true;
    }

    this->mModelID = modelId;
    this->mDeviceName = this->mModelData[this->mModelID].name;
    this->mTagEngine->data() = this->mParameters.tagEngine.data();

    this->processDeviceData();

    this->mVerified = this->mModelData[this->mModelID].verified;

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void POSPrinter<T>::processDeviceData() {
    QByteArray answer;

    if (this->mIOPort->write(CPOSPrinter::Command::GetTypeId) &&
        this->getAnswer(answer, CPOSPrinter::Timeouts::Info) && (answer.size() == 1)) {
        this->setDeviceParameter(CDeviceData::Printers::Unicode, ProtocolUtils::getBit(answer, 0));
        this->setDeviceParameter(CDeviceData::Printers::Cutter, ProtocolUtils::getBit(answer, 1));
        this->setDeviceParameter(CDeviceData::Printers::LabelPrinting,
                                 ProtocolUtils::getBit(answer, 2));
    }

    if (this->mIOPort->write(CPOSPrinter::Command::GetROMVersion) &&
        this->getAnswer(answer, CPOSPrinter::Timeouts::Info) && !answer.isEmpty()) {
        QString firmware = (answer.size() > 1) ? answer : ProtocolUtils::toHexLog(char(answer[0]));
        this->setDeviceParameter(CDeviceData::Firmware, firmware);
    }
}

//--------------------------------------------------------------------------------
template <class T> bool POSPrinter<T>::getModelId(QByteArray &aAnswer) const {
    return this->mIOPort->write(CPOSPrinter::Command::GetModelId) &&
           this->getAnswer(aAnswer, CPOSPrinter::Timeouts::Info) && (aAnswer.size() <= 1);
}

//--------------------------------------------------------------------------------
template <class T> char POSPrinter<T>::parseModelId(QByteArray &aAnswer) {
    return aAnswer[0];
}

//--------------------------------------------------------------------------------
template <class T> void POSPrinter<T>::checkVerifying() {
    if (!this->isAutoDetecting() && this->containsConfigParameter(CHardwareSDK::ModelName)) {
        QString modelName = this->getConfigParameter(CHardwareSDK::ModelName).toString();

        if (!modelName.isEmpty()) {
            POSPrinters::TModelData::iterator modelDataIt =
                std::find_if(this->mModelData.data().begin(),
                             this->mModelData.data().end(),
                             [&](const POSPrinters::SModelData &aModelData) -> bool {
                                 return aModelData.name == modelName;
                             });

            if (modelDataIt != this->mModelData.data().end()) {
                this->mVerified = modelDataIt->verified;
            }
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> bool POSPrinter<T>::updateParameters() {
    // TODO: устанавливать размеры ячейки сетки для печати, с учетом выбранного режима.
    QByteArray command = QByteArray(CPOSPrinter::Command::Initialize) +
                         CPOSPrinter::Command::SetEnabled +
                         CPOSPrinter::Command::SetCodePage(this->mRussianCodePage) +
                         CPOSPrinter::Command::SetUSCharacterSet +
                         CPOSPrinter::Command::SetStandardMode + CPOSPrinter::Command::AlignLeft;

    int lineSpacing =
        this->getConfigParameter(CHardware::Printer::Settings::LineSpacing, 0).toInt();

    if (lineSpacing) {
        command += CPOSPrinter::Command::SetLineSpacing(lineSpacing);
    }

    if (!this->mIOPort->write(command)) {
        return false;
    }

    this->mVerified = this->mVerified && !this->isOnlyDefaultModels();
    SleepHelper::msleep(CPOSPrinter::InitializationPause);

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool POSPrinter<T>::isOnlyDefaultModels() {
    return this->mModelData.data().keys().isEmpty();
}

//--------------------------------------------------------------------------------
template <class T> bool POSPrinter<T>::printLine(const QByteArray &aString) {
    QDateTime beginning = QDateTime::currentDateTime();

    if (!this->mIOPort->write(aString)) {
        return false;
    }

    if (this->mPrintingStringTimeout) {
        int count =
            aString.contains(this->mTagEngine->value(Tags::Type::DoubleHeight).open) ? 2 : 1;
        int pause = count * this->mPrintingStringTimeout -
                    int(beginning.msecsTo(QDateTime::currentDateTime()));

        if (pause > 0) {
            this->toLog(LogLevel::Debug,
                        this->mDeviceName +
                            QString(": Pause after printing line = %1 ms").arg(pause));
            SleepHelper::msleep(pause);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> QByteArray POSPrinter<T>::prepareBarcodePrinting() {
    return QByteArray() + CPOSPrinter::Command::Barcode::Height + CPOSPrinter::Barcode::Height +
           CPOSPrinter::Command::Barcode::HRIPosition + CPOSPrinter::Barcode::HRIPosition +
           CPOSPrinter::Command::Barcode::FontSize + CPOSPrinter::Barcode::FontSize +
           CPOSPrinter::Command::Barcode::Width + CPOSPrinter::Barcode::Width;
}

//--------------------------------------------------------------------------------
template <class T> bool POSPrinter<T>::printBarcode(const QString &aBarcode) {
    // В Qt 6 для отправки данных в принтер нужен ENCODER
    // Если mDecoder это QStringDecoder, он не сможет выполнить fromUnicode.

    QByteArray data;

    // Создаем временный энкодер на основе кодировки декодера
    // В Qt 6 это делается так:
    if (this->mDecoder) {
        QStringEncoder encoder(this->mDecoder->name());
        data = encoder(aBarcode); // Вызов как функции
    } else {
        data = aBarcode.toLatin1();
    }

    QByteArray barcodePrinting =
        QByteArray() + CPOSPrinter::Command::Barcode::Print + CPOSPrinter::Barcode::CodeSystem128 +
        static_cast<char>(data.size() + 2) + CPOSPrinter::Barcode::Code128Spec + data + '\x0A';

    return this->mIOPort->write(this->prepareBarcodePrinting() + barcodePrinting);
}

//--------------------------------------------------------------------------------

template <class T> bool POSPrinter<T>::getStatus(TStatusCodes &aStatusCodes) {
    // В Qt 6 итераторы QMap/QHash могут работать иначе, используем константные итераторы для
    // безопасности
    for (auto it = this->mParameters.errors.begin(); it != this->mParameters.errors.end(); ++it) {
        SleepHelper::msleep(CPOSPrinter::StatusPause);

        // keys() в Qt 6 возвращает QList
        QList<char> statusCommands = it.value().keys();
        QByteArray answer;

        // В Qt 6 qSort удален. Используем std::sort.
        QList<char> bytes = it.value().keys();
        std::sort(bytes.begin(), bytes.end());

        if (bytes.isEmpty()) {
            continue;
        }

        // Вызываем команду статуса. Используем it.key()
        if (!this->mIOPort->write(CPOSPrinter::Command::GetStatus(it.key())) ||
            !this->readStatusAnswer(
                answer, CPOSPrinter::Timeouts::Status, static_cast<int>(bytes.last())) ||
            answer.isEmpty()) {
            return false;
        }

        // Проверка XOff
        this->mOverflow = answer.startsWith(ASCII::XOff);

        if (this->mOverflow && (this->mInitialized == ERequestStatus::Success) &&
            !this->mStatusCollection.isEmpty() &&
            !this->mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable)) {
            aStatusCodes = this->getStatusCodes(this->mStatusCollection);
            return true;
        }

        // Заменяем max_element на использование уже отсортированного списка bytes для
        // производительности
        if (answer.size() != bytes.last()) {
            return false;
        }

        // Тройной вложенный цикл парсинга бит статуса
        for (auto jt = it.value().begin(); jt != it.value().end(); ++jt) {
            for (auto kt = jt.value().begin(); kt != jt.value().end(); ++kt) {
                // answer[i] возвращает char, используем побитовое И
                if (static_cast<unsigned char>(answer[jt.key() - 1]) &
                    static_cast<unsigned char>(kt.key())) {
                    aStatusCodes.insert(kt.value());
                }
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------

template <class T>
bool POSPrinter<T>::readStatusAnswer(QByteArray &aAnswer, int aTimeout, int aBytesCount) const {
    QVariantMap configuration;
    // В Qt 6 конструкторы QVariant часто explicit, используем fromValue
    configuration.insert(CHardware::Port::IOLogging, QVariant::fromValue(ELoggingType::Write));
    this->mIOPort->setDeviceConfiguration(configuration);

    // 1. В Qt 6 методы QTime::start() и elapsed() удалены.
    // Используем QElapsedTimer — он точнее и доступен в Qt 5.15 и 6.
    QElapsedTimer timer;
    timer.start();

    do {
        QByteArray data;

        // Используем 10 мс как таймаут чтения одного блока
        if (!this->mIOPort->read(data, 10)) {
            return false;
        }

        aAnswer.append(data);

        // 2. Если получили достаточно данных, выходим не дожидаясь таймаута
        if (aAnswer.size() >= aBytesCount) {
            break;
        }

    } while (timer.elapsed() < aTimeout);

    // 3. Логика определения уровня лога
    LogLevel::Enum logLevel =
        (aAnswer.size() < aBytesCount)
            ? LogLevel::Error
            : ((aAnswer.size() > aBytesCount) ? LogLevel::Warning : LogLevel::Normal);

    // 4. Оптимизация лога:
    // - QStringLiteral для производительности
    // - В Qt 6 toHex() возвращает QByteArray, который можно передать в .arg() напрямую
    this->toLog(logLevel,
                QStringLiteral("%1: << {%2}")
                    .arg(this->mDeviceName)
                    .arg(QString::fromLatin1(aAnswer.toHex())));

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool POSPrinter<T>::printImage(const QImage &aImage, const Tags::TTypes &aTags) {
    int widthInBytes = aImage.width() / 8 + ((aImage.width() % 8) ? 1 : 0);
    bool doubleWidth =
        aTags.contains(Tags::Type::DoubleWidth) || aTags.contains(Tags::Type::DoubleWidth);
    bool doubleHeight = aTags.contains(Tags::Type::DoubleHeight);
    char imageTagFactors = (doubleWidth * CPOSPrinter::ImageFactors::DoubleWidth) |
                           (doubleHeight * CPOSPrinter::ImageFactors::DoubleHeight);

    QByteArray request;
    request.append(CPOSPrinter::Command::PrintImage);
    request.append(imageTagFactors);
    request.append(uchar(widthInBytes));
    request.append(char(0));
    request.append(uchar(aImage.height() % 256));
    request.append(uchar(aImage.height() / 256));

    for (int i = 0; i < aImage.height(); ++i) {
        request.append((const char *)aImage.scanLine(i), widthInBytes);
    }

    if (!this->mIOPort->write(request)) {
        return false;
    }

    // TODO: под рефакторинг.
    // Причины необходимости задержки ясны не до конца, т.к. задержка начинает работать после
    // фактической печати картинки. Задержка нужна тем большая, чем больше картинок печатается
    // одновременно или почти одновременно, через какое-то количество строк текста.
    int pause = qMin(int(double(request.size()) / 2), 5000);
    this->toLog(LogLevel::Debug,
                this->mDeviceName +
                    QString(": size = %1, pause = %2").arg(request.size()).arg(pause));
    SleepHelper::msleep(pause);

    return true;
}

//--------------------------------------------------------------------------------
