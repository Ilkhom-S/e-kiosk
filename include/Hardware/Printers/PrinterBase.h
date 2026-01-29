/* @file Базовый принтер. */

#pragma once

// Qt
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtCore/QVariantMap>
#include <QtCore/QRegularExpression>
#include <QtGui/QImage>

// SDK
#include <SDK/Drivers/PrintingModes.h>

// Modules
#include "Hardware/Common/DeviceBase.h"
#include "Hardware/Printers/PrinterConstants.h"
#include "Hardware/Printers/PrinterStatusCodes.h"
#include "Hardware/Printers/PrinterStatusesDescriptions.h"
#include "Hardware/Printers/Tags.h"

namespace CPrinter
{
    /// Спец-теги.
    const Tags::TTypes SpecialTags = Tags::TTypes() << Tags::Type::Image << Tags::Type::BarCode;
} // namespace CPrinter

//--------------------------------------------------------------------------------
template <class T> class PrinterBase : public T
{
  public:
    PrinterBase();

    /// Напечатать массив строк.
    virtual bool print(const QStringList &aReceipt) override;

    /// Готов ли к печати.
    virtual bool isDeviceReady(bool aOnline) override;

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration) override;

  protected:
    /// Идентифицирует устройство.
    virtual bool isConnected() override;

    /// Завершение инициализации.
    virtual void finalizeInitialization() override;

    /// Выполнить нереентерабельную команду.
    virtual bool processNonReentrant(TBoolMethod aCommand);

    /// Обработка чека после печати.
    virtual bool receiptProcessing();

    /// Фоновая логика при появлении определенных состояний устройства.
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection) override;

    /// Возможно ли печать.
    virtual bool isPossible(bool aOnline, QVariant aCommand = QVariant());

    /// Напечатать чек.
    virtual bool printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt);

    /// Напечатать [и выдать] чек.
    virtual bool processReceipt(const QStringList &aReceipt, bool aProcessing = true);

    /// Проверить необходимость печати.
    virtual bool isPrintingNeed(const QStringList &aReceipt);

    /// Удалить пустые строки и лишние пробелы.
    QStringList simplifyReceipt(const QStringList &aReceipt);

    virtual bool canForceStatusBufferEnable()
    {
        return false;
    }

    /// Разбивает строки по разделителям тега BR.
    void separate(QStringList &aReceipt) const;

    virtual void cleanStatusCodes(TStatusCodes &aStatusCodes) override;
    virtual bool execSpecialTag(const Tags::SLexeme &aTagLexeme);
    void adjustToLineSize(Tags::TLexemesBuffer &aTagLexemes, Tags::TLexemesCollection &aLexemesCollection);
    void makeLexemeReceipt(const QStringList &aReceipt, Tags::TLexemeReceipt &aLexemeReceipt);
    bool clearDispenser(const QString &aCondition);
    virtual void execTags(Tags::SLexeme &aTagLexeme, QVariant &aLine);
    virtual bool feed();
    virtual bool printLine(const QVariant &)
    {
        return true;
    }
    virtual bool printImage(const QImage &, const Tags::TTypes &)
    {
        return true;
    }
    virtual bool printBarcode(const QString &)
    {
        return true;
    }
    virtual bool cut()
    {
        return true;
    }
    virtual bool present()
    {
        return true;
    }
    virtual bool push()
    {
        return true;
    }
    virtual bool retract()
    {
        return true;
    }
    bool canCheckReady(bool aOnline);

    QDateTime mPaperInPresenter;
    Tags::PEngine mTagEngine;
    DeviceStatusCode::PSpecifications mStatusCodesSpecification;
    int mMaxBadAnswers;
    typedef QMap<int, TStatusCodes> TUnnecessaryErrors;
    TUnnecessaryErrors mUnnecessaryErrors;
    int mLineSize;
    bool mLineFeed;
    Tags::TTypes mLineTags;
    int mActualStringCount;
    SDK::Driver::EPrintingModes::Enum mPrintingMode;
    bool mClearingDispenserTurnOff;
};

//--------------------------------------------------------------------------------
// РЕАЛИЗАЦИЯ (IMPLEMENTATION)
//--------------------------------------------------------------------------------

template <class T> PrinterBase<T>::PrinterBase()
{
    // Использование this-> обязательно для доступа к членам шаблона на Linux/Mac
    this->mTagEngine = Tags::PEngine(new Tags::Engine());
    this->mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new PrinterStatusCode::CSpecifications());
    this->mMaxBadAnswers = 2;

    // Инициализируем настройки устройства
    this->mPollingInterval = 5 * 1000;
    this->mLineSize = 0;
    this->mLineFeed = true;
    this->mPaperInPresenter = QDateTime::currentDateTime();
    this->mActualStringCount = 0;
    this->mPrintingMode = SDK::Driver::EPrintingModes::None;
    this->mClearingDispenserTurnOff = false;

    // Параметры конфигурации по умолчанию
    this->setConfigParameter(CHardware::Printer::NeedSeparating, true);
    this->setConfigParameter(CHardware::Printer::PresenterEnable, false);
    this->setConfigParameter(CHardware::Printer::RetractorEnable, false);
    this->setConfigParameter(CHardware::Printer::FeedingAmount, 0);
    this->setConfigParameter(CHardware::Printer::NeedCutting, true);
}

template <class T> bool PrinterBase<T>::print(const QStringList &aReceipt)
{
    if (!this->isPrintingNeed(aReceipt))
    {
        return true;
    }

    QStringList receipt = this->simplifyReceipt(aReceipt);

    // Используем лямбду вместо std::bind для лучшей совместимости с макросами потоков Qt
    return this->processNonReentrant([this, receipt]() { return this->processReceipt(receipt, true); });
}

template <class T>
void PrinterBase<T>::makeLexemeReceipt(const QStringList &aReceipt, Tags::TLexemeReceipt &aLexemeReceipt)
{
    QStringList receipt = aReceipt;

    if (this->getConfigParameter(CHardware::Printer::NeedSeparating).toBool())
    {
        this->separate(receipt);
    }

    for (const QString &line : receipt)
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

template <class T> QStringList PrinterBase<T>::simplifyReceipt(const QStringList &aReceipt)
{
    QStringList result = aReceipt;

    // Qt 6 замена для QRegExp. QStringLiteral оптимизирует работу на Win7.
    QRegularExpression regExpEmptyLine(QStringLiteral("^[ \\n\\r\\t]*$"));
    QRegularExpression regExpTrailingSpace(QStringLiteral("[ \\n\\r\\t]+$"));

    for (int i = 0; i < result.size(); ++i)
    {
        // Удаляем пробельные символы в конце строки
        auto match = regExpTrailingSpace.match(result[i]);
        if (match.hasMatch())
        {
            result[i] = result[i].left(static_cast<int>(match.capturedStart()));
        }

        // Удаляем полностью пустые строки
        if (regExpEmptyLine.match(result[i]).hasMatch())
        {
            result.removeAt(i--);
        }
    }
    return result;
}

template <class T> void PrinterBase<T>::separate(QStringList &aReceipt) const
{
    for (int i = 0; i < aReceipt.size(); ++i)
    {
        if (aReceipt[i].contains(Tags::BR))
        {
            QStringList lines = aReceipt[i].split(Tags::BR);
            aReceipt.removeAt(i);
            for (int j = 0; j < lines.size(); ++j)
            {
                aReceipt.insert(i + j, lines[j]);
            }
            i += lines.size() - 1;
        }
    }
}

template <class T>
void PrinterBase<T>::postPollingAction(const TStatusCollection &aNewStatusCollection,
                                       const TStatusCollection &aOldStatusCollection)
{
    // Используем пространство имен из вашего PrinterStatusCodes.h
    if (aNewStatusCollection.contains(PrinterStatusCode::OK::PaperInPresenter))
    {
        QDateTime current = QDateTime::currentDateTime();

        // В шаблонах обязательно используем this-> для методов базового класса
        int timeout = this->getConfigParameter(CHardware::Printer::Settings::LeftReceiptTimeout).toInt();

        // Проверяем таймаут нахождения чека в презентере
        if ((this->mPaperInPresenter.secsTo(current) > timeout) &&
            !this->clearDispenser(CHardware::Printer::Settings::NotTakenReceipt))
        {
            // Рассчитываем время следующей попытки очистки
            int repeatTimeout = CPrinters::ClearingPresenterRepeatTimeout;
            this->mPaperInPresenter = current.addMSecs((repeatTimeout - timeout) * 1000 - this->mPollingInterval);
        }
    }
    else
    {
        // Сброс таймера, если бумага забрана или отсутствует
        this->mPaperInPresenter = QDateTime::currentDateTime();
    }

    // Вызов реализации базового класса (PortPollingDeviceBase или PollingDeviceBase)
    T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

template <class T> bool PrinterBase<T>::isPrintingNeed(const QStringList &aReceipt)
{
    for (const QString &line : aReceipt)
    {
        if (!line.trimmed().isEmpty())
            return true;
    }
    return false;
}

// Заглушки для базовых методов, чтобы обеспечить компиляцию шаблона
template <class T> void PrinterBase<T>::finalizeInitialization()
{
    T::finalizeInitialization();
}
template <class T> bool PrinterBase<T>::isConnected()
{
    return T::isConnected();
}
template <class T> bool PrinterBase<T>::isDeviceReady(bool aOnline)
{
    Q_UNUSED(aOnline)
    return true;  // Default implementation
}
template <class T> bool PrinterBase<T>::isPossible(bool aO, QVariant aC)
{
    Q_UNUSED(aO)
    Q_UNUSED(aC)
    return true;  // Default implementation
}
template <class T> void PrinterBase<T>::setDeviceConfiguration(const QVariantMap &aC)
{
    T::setDeviceConfiguration(aC);
}
template <class T> bool PrinterBase<T>::processNonReentrant(TBoolMethod aC)
{
    return this->processNonReentrant(aC);
}

// Default implementations for virtual methods
template <class T> bool PrinterBase<T>::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt)
{
    Q_UNUSED(aLexemeReceipt)
    return true;
}

template <class T> bool PrinterBase<T>::processReceipt(const QStringList &aReceipt, bool aProcessing)
{
    Q_UNUSED(aReceipt)
    Q_UNUSED(aProcessing)
    return true;
}

template <class T> bool PrinterBase<T>::receiptProcessing()
{
    return true;
}

template <class T> bool PrinterBase<T>::clearDispenser(const QString &aCondition)
{
    Q_UNUSED(aCondition)
    return true;
}

template <class T> void PrinterBase<T>::execTags(Tags::SLexeme &aTagLexeme, QVariant &aLine)
{
    Q_UNUSED(aTagLexeme)
    Q_UNUSED(aLine)
}

template <class T> bool PrinterBase<T>::feed()
{
    return true;
}

template <class T> void PrinterBase<T>::cleanStatusCodes(TStatusCodes &aStatusCodes)
{
    Q_UNUSED(aStatusCodes)
}

template <class T> bool PrinterBase<T>::execSpecialTag(const Tags::SLexeme &aTagLexeme)
{
    Q_UNUSED(aTagLexeme)
    return true;
}

template <class T> void PrinterBase<T>::adjustToLineSize(Tags::TLexemesBuffer &aTagLexemes, Tags::TLexemesCollection &aLexemesCollection)
{
    // Default implementation: add the tag lexemes as a single collection
    if (!aTagLexemes.isEmpty())
    {
        aLexemesCollection << aTagLexemes;
    }
}
