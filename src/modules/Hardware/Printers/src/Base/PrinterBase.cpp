/* @file Базовый принтер. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegularExpression>
#include <QtCore/QtAlgorithms>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// System
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Common/TCPDeviceBase.h"
#include "Hardware/Printers/PrinterStatusesDescriptions.h"

// Project
#include "PrinterBase.h"

using namespace SDK::Driver;
using namespace PrinterStatusCode;

//---------------------------------------------------------------------------
template class PrinterBase<PollingDeviceBase<ProtoPrinter>>;
template class PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>;

//---------------------------------------------------------------------------
template <class T> PrinterBase<T>::PrinterBase()
{
    // теги по умолчанию
    this->mTagEngine = Tags::PEngine(new Tags::Engine());

    // описания для кодов статусов
    this->mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new CSpecifications());
    this->mMaxBadAnswers = 2;

    // восстановимые ошибки
    QMap<int, SStatusCodeSpecification> &statusCodesData = this->mStatusCodesSpecification->data();

    for (auto it = statusCodesData.begin(); it != statusCodesData.end(); ++it)
    {
        if (it->warningLevel == EWarningLevel::Warning)
        {
            this->mRecoverableErrors.insert(it.key());
        }
    }

    // настройки устройства
    this->mPollingInterval = 5 * 1000;
    this->mLineSize = 0;
    this->mLineFeed = true;
    this->mPaperInPresenter = QDateTime::currentDateTime();
    this->mActualStringCount = 0;
    this->mPrintingMode = EPrintingModes::None;
    this->mClearingDispenserTurnOff = false;

    // настройки для плагинов
    this->setConfigParameter(CHardware::Printer::NeedSeparating, true);
    this->setConfigParameter(CHardware::Printer::PresenterEnable, false);
    this->setConfigParameter(CHardware::Printer::RetractorEnable, false);
    this->setConfigParameter(CHardware::Printer::FeedingAmount, 0);
    this->setConfigParameter(CHardware::Printer::NeedCutting, true);

    this->mExcessStatusCollection[EWarningLevel::OK].insert(PrinterStatusCode::OK::PaperInPresenter);
    this->mExcessStatusCollection[EWarningLevel::OK].insert(PrinterStatusCode::OK::MotorMotion);
}

//--------------------------------------------------------------------------------
template <class T> void PrinterBase<T>::setDeviceConfiguration(const QVariantMap &aConfiguration)
{
    T::setDeviceConfiguration(aConfiguration);

    if (aConfiguration.contains(CHardwareSDK::Printer::PrintingMode))
    {
        this->mPrintingMode = EPrintingModes::Enum(aConfiguration[CHardwareSDK::Printer::PrintingMode].toInt());

        if (this->mPrintingMode == EPrintingModes::Glue)
        {
            this->mClearingDispenserTurnOff = true;
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> void PrinterBase<T>::finalizeInitialization()
{
    if (this->containsConfigParameter(CHardwareSDK::Printer::LineSize))
    {
        this->setDeviceParameter(CHardwareSDK::Printer::LineSize,
                                 this->getConfigParameter(CHardwareSDK::Printer::LineSize));
    }

    T::finalizeInitialization();
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::isConnected()
{
    TStatusCodes statusCodes;

    return this->getStatus(statusCodes) && !statusCodes.contains(DeviceStatusCode::Error::NotAvailable);
}

//---------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::processNonReentrant(TBoolMethod aCommand)
{
    MutexLocker externalLocker(&this->mExternalMutex);

    this->stopPolling(true);

    {
        MutexLocker resourceLocker(&this->mResourceMutex);

        if (!this->checkConnectionAbility() || !PrinterBase<T>::isConnected())
        {
            if (this->mOperatorPresence)
            {
                this->processStatusCodes(TStatusCodes() << DeviceStatusCode::Error::NotAvailable);
            }

            this->startPolling(true);

            return false;
        }
    }

    if (this->mStatusCollection.contains(OK::PaperInPresenter))
    {
        this->clearDispenser(CHardware::Printer::Settings::PreviousReceipt);
    }

    bool result = aCommand();

    this->mForceStatusBufferEnabled = this->mForceStatusBufferEnabled || this->canForceStatusBufferEnable();
    this->simplePoll();
    this->mForceStatusBufferEnabled = false;

    if (this->getConfigParameter(CHardware::Printer::PresenterEnable).toBool() ||
        this->getConfigParameter(CHardware::Printer::RetractorEnable).toBool())
    {
        this->mPaperInPresenter = QDateTime::currentDateTime();
    }

    this->startPolling(true);

    return result;
}

//---------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::print(const QStringList &aReceipt)
{
    if (!this->isPrintingNeed(aReceipt))
    {
        return true;
    }

    QStringList receipt = this->simplifyReceipt(aReceipt);

    if (!this->processNonReentrant(std::bind(&PrinterBase<T>::processReceipt, this, std::ref(receipt), true)))
    {
        this->toLog(LogLevel::Error, "Printing completed unsuccessfully.");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::makeLexemeReceipt(const QStringList &aReceipt, Tags::TLexemeReceipt &aLexemeReceipt)
{
    QStringList receipt(aReceipt);

    if (this->getConfigParameter(CHardware::Printer::NeedSeparating).toBool())
    {
        this->separate(receipt);
    }

    foreach (auto line, receipt)
    {
        Tags::TLexemesBuffer tagLexemes;
        this->mTagEngine->splitForLexemes(line, tagLexemes);

        if (!tagLexemes.isEmpty())
        {
            Tags::TLexemesCollection lexemesCollection;
            this->adjustToLineSize(tagLexemes, lexemesCollection);

            aLexemeReceipt << lexemesCollection;
        }
    }
}

//--------------------------------------------------------------------------------
template <class T> QStringList PrinterBase<T>::simplifyReceipt(const QStringList &aReceipt)
{
    QStringList result(aReceipt);

    // 1. Используем QRegularExpression. QStringLiteral предотвращает лишние аллокации.
    QRegularExpression regExpEmptyLine(QStringLiteral("^[ \\n\\r\\t]+$"));
    QRegularExpression regExpFreeSpace(QStringLiteral("[ \\n\\r\\t]+$"));

    for (int i = 0; i < result.size(); ++i)
    {
        // В Qt 6 split() по умолчанию возвращает QStringList.
        // Используем Qt::KeepEmptyParts для сохранения структуры.
        QStringList lines = result[i].split(Tags::BR, Qt::KeepEmptyParts);

        // В Qt 6 метод replaceInStrings(QRegExp, ...) удален.
        // Используем явный цикл для обработки каждой строки.
        for (QString &line : lines)
        {
            if (regExpEmptyLine.match(line).hasMatch())
            {
                line.clear();
            }
        }
        lines.removeAll(QString());

        if (lines.isEmpty())
        {
            lines << QString();
        }

        result.removeAt(i--);

        for (const QString &line : lines)
        {
            // Находим начало хвостовых пробелов через QRegularExpressionMatch
            QRegularExpressionMatch match = regExpFreeSpace.match(line);
            int index = match.hasMatch() ? static_cast<int>(match.capturedStart()) : line.length();

            result.insert(++i, line.left(index));
        }
    }

    for (int i = 0; i < result.size(); ++i)
    {
        result[i].replace(ASCII::TAB, ASCII::Space);

        // Работа с картой автокоррекции
        const auto &correctionData = CPrinters::AutoCorrection.constData();
        for (auto it = correctionData.begin(); it != correctionData.end(); ++it)
        {
            result[i].replace(it.key(), it.value());
        }
    }

    // Удаление пустых строк
    for (int i = 0; i < result.size(); ++i)
    {
        if (result[i].simplified().isEmpty())
        {
            result.removeAt(i--);
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::isPrintingNeed(const QStringList &aReceipt)
{
    if (aReceipt.isEmpty())
    {
        this->toLog(LogLevel::Normal, this->mDeviceName + ": receipt is empty");
        return false;
    }

    QStringList receipt = this->simplifyReceipt(aReceipt);

    if (receipt.isEmpty())
    {
        this->toLog(LogLevel::Normal, this->mDeviceName + ": simplified receipt is empty");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::processReceipt(const QStringList &aReceipt, bool aProcessing)
{
    if (!this->isPrintingNeed(aReceipt))
    {
        return true;
    }

    this->toLog(LogLevel::Normal, "Printing receipt:\n" + aReceipt.join("\n"));

    Tags::TLexemeReceipt lexemeReceipt;
    QStringList receipt = this->simplifyReceipt(aReceipt);
    this->makeLexemeReceipt(receipt, lexemeReceipt);

    bool printing = this->printReceipt(lexemeReceipt);

    if (this->mPrintingMode == EPrintingModes::Glue)
    {
        return printing;
    }

    bool processing = (printing && !aProcessing) || this->receiptProcessing();

    if (printing && aProcessing && (this->mPrintingMode == EPrintingModes::Continuous))
    {
        if (this->getConfigParameter(CHardware::Printer::PresenterEnable).toBool())
        {
            this->push();
        }
        else if (this->getConfigParameter(CHardware::Printer::RetractorEnable).toBool())
        {
            this->retract();
        }
    }

    return printing && processing;
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt)
{
    bool result = true;

    foreach (auto lexemeCollection, aLexemeReceipt)
    {
        foreach (auto lexemes, lexemeCollection)
        {
            QVariant line;

            foreach (auto lexeme, lexemes)
            {
                Tags::TTypes specialTags = CPrinter::SpecialTags & lexeme.tags;

                if (!specialTags.isEmpty())
                {
                    if (!line.isNull())
                    {
                        if (!this->printLine(line))
                        {
                            result = false;
                        }

                        line.clear();
                    }

                    if (!this->execSpecialTag(lexeme))
                    {
                        result = false;
                    }
                }
                else
                {
                    this->execTags(lexeme, line);
                }
            }

            this->mLineTags.clear();

            foreach (auto lexeme, lexemes)
            {
                this->mLineTags += lexeme.tags;
            }

            bool containsDH = this->mLineTags.contains(Tags::Type::DoubleHeight) &&
                              this->mTagEngine->contains(Tags::Type::DoubleHeight);
            this->mActualStringCount += containsDH ? 2 : 1;

            if (!this->printLine(line))
            {
                if (!this->isDeviceReady(true))
                {
                    this->toLog(LogLevel::Error, this->mDeviceName + ": Printer is not ready for printing to continue");
                    return false;
                }

                result = this->printLine(line);
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::receiptProcessing()
{
    bool needCutting = this->getConfigParameter(CHardware::Printer::NeedCutting).toBool();
    bool needPresenting =
        !this->getConfigParameter(CHardware::Printer::Commands::Presentation).toByteArray().isEmpty() &&
        (this->getConfigParameter(CHardware::Printer::Settings::Loop) == CHardwareSDK::Values::Use) &&
        this->getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt();

    bool feeding = this->feed();
    bool cutting = !needCutting || this->cut();
    bool presenting = !needPresenting || this->present();

    return feeding && cutting && presenting;
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::feed()
{
    int amount = this->getConfigParameter(CHardware::Printer::FeedingAmount).toInt();
    bool result = true;

    for (int i = 0; i < amount; ++i)
    {
        if (!this->printLine(" "))
        {
            result = false;
        }
    }

    if (!result)
    {
        this->toLog(LogLevel::Error, this->mDeviceName + ": Failed to feed paper");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::adjustToLineSize(Tags::TLexemesBuffer &aTagLexemes, Tags::TLexemesCollection &aLexemesCollection)
{
    if (!this->mLineSize)
    {
        aLexemesCollection.append(aTagLexemes);

        return;
    }

    int linesize = 0;
    int index = 0;
    int rest = 0;

    for (int i = 0; i < aTagLexemes.size(); ++i)
    {
        if (aTagLexemes[i].tags.contains(Tags::Type::HR))
        {
            bool OK;
            int size = aTagLexemes[i].data.toInt(&OK);

            if (!OK)
            {
                size = this->mLineSize ? this->mLineSize : CPrinters::DefaultHRSize;
            }

            aTagLexemes[i].data = QString("-").repeated(size);
        }

        if (aTagLexemes[i].tags.contains(Tags::Type::Image) || aTagLexemes[i].tags.contains(Tags::Type::BarCode))
        {
            aLexemesCollection.append(aTagLexemes);

            continue;
        }

        if (aLexemesCollection.size() == index)
        {
            aLexemesCollection.append(Tags::TLexemesBuffer());
        }

        int factor = 1 + int(aTagLexemes[i].tags.contains(Tags::Type::DoubleWidth) &&
                             this->mTagEngine->data().keys().contains(Tags::Type::DoubleWidth));
        linesize += aTagLexemes[i].data.size() * factor;
        aLexemesCollection[index].append(aTagLexemes[i]);

        do
        {
            rest = qCeil(double(linesize - this->mLineSize) / factor);

            if (rest > 0)
            {
                if (aLexemesCollection.size() == ++index)
                {
                    aLexemesCollection.append(Tags::TLexemesBuffer());
                }

                aLexemesCollection[index].append(aLexemesCollection[index - 1].last());
                aLexemesCollection[index][0].data = aLexemesCollection[index][0].data.right(rest);
                aLexemesCollection[index - 1].last().data.chop(rest);

                linesize = rest * factor;
            }
        } while ((rest > 0));
    }
}

//--------------------------------------------------------------------------------
template <class T> void PrinterBase<T>::execTags(Tags::SLexeme &aTagLexeme, QVariant &aLine)
{
    QString data = aTagLexeme.data;

    foreach (const Tags::TTypes types, this->mTagEngine->groupsTypesByPrefix(aTagLexeme.tags))
    {
        QByteArray openTag = this->mTagEngine->getTag(types, Tags::Direction::Open);
        QByteArray closeTag = this->mTagEngine->getTag(types, Tags::Direction::Close);
        data = openTag + data + closeTag;
    }

    aLine = aLine.toString() + data;
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::execSpecialTag(const Tags::SLexeme &aTagLexeme)
{
    if (aTagLexeme.tags.contains(Tags::Type::Image))
    {
        QImage image;

        if (image.loadFromData(QByteArray::fromBase64(aTagLexeme.data.toLatin1())) && !image.isNull())
        {
            for (int i = 0; i < image.width(); ++i)
            {
                for (int j = 0; j < image.height(); ++j)
                {
                    if (!qAlpha(image.pixel(i, j)))
                    {
                        image.setPixel(i, j, CPrinters::White);
                    }
                }
            }

            image = image.convertToFormat(QImage::Format_Mono);
            this->printImage(image, aTagLexeme.tags);
        }
    }
    else if (aTagLexeme.tags.contains(Tags::Type::BarCode))
    {
        return this->printBarcode(aTagLexeme.data);
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void PrinterBase<T>::separate(QStringList &aReceipt) const
{
    QStringList receipt;

    foreach (QString line, aReceipt)
    {
        QString resultLine = line.replace(ASCII::CR, "");

        while (resultLine.indexOf(ASCII::TAB) != -1)
        {
            int tPoint = resultLine.indexOf(ASCII::TAB);
            int numberOfSpaces = qCeil(tPoint / (double)CPrinters::SpacesInTAB) * CPrinters::SpacesInTAB - tPoint;

            if (tPoint % CPrinters::SpacesInTAB == 0)
            {
                numberOfSpaces = CPrinters::SpacesInTAB;
            }

            resultLine = resultLine.replace(tPoint, 1, QString(ASCII::Space).repeated(numberOfSpaces));
        }

        if (resultLine.indexOf(ASCII::LF) != -1)
        {
            receipt.append(resultLine.split(ASCII::LF));
        }
        else
        {
            receipt.append(resultLine);
        }
    }

    aReceipt = receipt;
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::canCheckReady(bool aOnline)
{
    bool notConnected = !this->mConnected && (!this->mOperatorPresence || !aOnline);

    return (this->mInitialized != ERequestStatus::InProcess) && !notConnected;
}

//---------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::isDeviceReady(bool aOnline)
{
    return this->canCheckReady(aOnline) && this->isPossible(aOnline);
}

//---------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::isPossible(bool aOnline, QVariant aCommand)
{
    if (this->mConnected && (this->mInitialized == ERequestStatus::Fail))
    {
        MutexLocker locker(&this->mResourceMutex);

        if (this->mUnnecessaryErrors[aCommand.toInt()].isEmpty())
        {
            return false;
        }
    }

    if (aOnline)
    {
        if (!this->checkConnectionAbility())
        {
            return false;
        }

        TStatusCodes statusCodes;
        this->mOperatorPresence ? this->onPoll() : this->doPoll(statusCodes);
    }

    MutexLocker locker(&this->mResourceMutex);

    TStatusCodes errorCodes = this->mStatusCollection.value(EWarningLevel::Error);

    if (aCommand.metaType().id() == QMetaType::Int)
    {
        errorCodes -= this->mUnnecessaryErrors[aCommand.toInt()];
    }

    return errorCodes.isEmpty();
}

//---------------------------------------------------------------------------
template <class T> void PrinterBase<T>::cleanStatusCodes(TStatusCodes &aStatusCodes)
{
    if (aStatusCodes.contains(Error::PrinterFRNotAvailable))
    {
        TStatusCodes availableErrors =
            this->mStatusCodesSpecification.dynamicCast<PrinterStatusCode::CSpecifications>()->getAvailableErrors();
        aStatusCodes -= availableErrors;
    }

    if (this->containsConfigParameter(CHardware::Printer::Settings::PaperJamSensor) &&
        (this->getConfigParameter(CHardware::Printer::Settings::PaperJamSensor) == CHardwareSDK::Values::NotUse))
    {
        aStatusCodes.remove(Error::PaperJam);
    }

    if (this->containsConfigParameter(CHardware::Printer::Settings::RemotePaperSensor) &&
        (this->getConfigParameter(CHardware::Printer::Settings::RemotePaperSensor) == CHardwareSDK::Values::NotUse))
    {
        aStatusCodes.remove(Warning::PaperNearEnd);
    }

    if (aStatusCodes.isEmpty())
    {
        aStatusCodes.insert(DeviceStatusCode::OK::OK);
    }

    if (this->getConfigParameter(CHardwareSDK::Printer::OFDNotSentError, false).toBool())
    {
        bool error = this->getConfigParameter(CHardwareSDK::Printer::BlockTerminalOnError, true).toBool();
        aStatusCodes.insert(error ? Error::OFDNotSent : Warning::OFDNotSent);
    }

    T::cleanStatusCodes(aStatusCodes);

    if (aStatusCodes.contains(Error::PaperEnd))
    {
        aStatusCodes.remove(Warning::PaperNearEnd);
        aStatusCodes.remove(Warning::PaperEndVirtual);
    }

    if (aStatusCodes.contains(Error::Temperature))
    {
        aStatusCodes.remove(Error::PrintingHead);
    }
}

//--------------------------------------------------------------------------------
template <class T> bool PrinterBase<T>::clearDispenser(const QString &aCondition)
{
    if (this->mClearingDispenserTurnOff)
    {
        this->mClearingDispenserTurnOff = this->mPrintingMode == EPrintingModes::Glue;

        return true;
    }

    bool presenterEnable = this->getConfigParameter(CHardware::Printer::PresenterEnable).toBool();
    bool retractorEnable = this->getConfigParameter(CHardware::Printer::RetractorEnable).toBool();

    if (!this->containsConfigParameter(aCondition) && (presenterEnable || retractorEnable))
    {
        this->toLog(LogLevel::Warning, this->mDeviceName + ": Unknown condition for clearing dispenser: " + aCondition);
        return true;
    }

    if (this->getConfigParameter(aCondition) == CHardware::Printer::Values::Retract)
    {
        if (retractorEnable && !this->retract())
        {
            this->toLog(LogLevel::Error, this->mDeviceName + ": Failed to retract the document");
            return false;
        }
    }
    else if (this->getConfigParameter(aCondition) == CHardware::Printer::Values::Push)
    {
        if (presenterEnable && !this->push())
        {
            this->toLog(LogLevel::Error, this->mDeviceName + ": Failed to push the document");
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PrinterBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                       const TStatusCollection &aOldStatusCollection)
{
    if (aNewStatusCollection.contains(OK::PaperInPresenter))
    {
        QDateTime current = QDateTime::currentDateTime();
        int timeout = this->getConfigParameter(CHardware::Printer::Settings::LeftReceiptTimeout).toInt();

        if ((this->mPaperInPresenter.secsTo(current) > timeout) &&
            !this->clearDispenser(CHardware::Printer::Settings::NotTakenReceipt))
        {
            this->mPaperInPresenter =
                current.addMSecs((CPrinters::ClearingPresenterRepeatTimeout - timeout) * 1000 - this->mPollingInterval);
        }
    }
    else
    {
        this->mPaperInPresenter = QDateTime::currentDateTime();
    }

    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
