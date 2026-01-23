/* @file Системный принтер. */

// STL
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QBuffer>
#include <QtCore/QRegularExpression>
#include <QtGui/QFontDatabase>
#include <QtGui/QTextDocument>
#include <QtGui/QTransform>
#include <QtPrintSupport/QPrinter>
#include <Common/QtHeadersEnd.h>

// System
#include "SysUtils/ISysUtils.h"

// Project
#include "SystemPrinter.h"

namespace PrinterSettings = CHardware::Printer::Settings;

//--------------------------------------------------------------------------------
SystemPrinter::SystemPrinter() {
    // данные устройства
    mDeviceName = "System printer";
    setConfigParameter(CHardware::Printer::NeedSeparating, false);
    mLineFeed = false;
    mSideMargin = 1.0;

    // теги
    mTagEngine = Tags::PEngine(new CSystemPrinter::TagEngine());
}

//--------------------------------------------------------------------------------
bool SystemPrinter::isConnected() {
    return mPrinter.isValid();
}

//--------------------------------------------------------------------------------
bool SystemPrinter::updateParameters() {
    if (mPrinter.printerName().isEmpty()) {
        return true;
    }

    mDeviceName = QStringLiteral("System printer (%1)").arg(mPrinter.printerName());
    QVariantMap deviceData = ISysUtils::getPrinterData(mPrinter.printerName());

    for (auto it = deviceData.begin(); it != deviceData.end(); ++it) {
        setDeviceParameter(it.key(), it.value());
    }

    QString name = deviceData[CDeviceData::Name].toString();

    if (name.contains(QStringLiteral("VKP80 II"), Qt::CaseInsensitive)) {
        setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
        setConfigParameter(PrinterSettings::LeftMargin, 1);
        setConfigParameter(PrinterSettings::RightMargin, 1);
    }

    if (name.contains(QStringLiteral("TG2480-H"), Qt::CaseInsensitive)) {
        setConfigParameter(CHardwareSDK::Printer::LineSize, 40);
        setConfigParameter(PrinterSettings::LeftMargin, 0);
        setConfigParameter(PrinterSettings::RightMargin, 0);
    }

    QString errorMessage;
    if (!ISysUtils::setPrintingQueuedMode(mPrinter.printerName(), errorMessage)) {
        toLog(LogLevel::Warning, QStringLiteral("Failed to change printing queued mode, ") + errorMessage);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool SystemPrinter::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt) {
    QStringList receipt;

    for (const auto &lexemeCollection : aLexemeReceipt) {
        for (const auto &lexemes : lexemeCollection) {
            QVariant line;

            for (auto lexeme : lexemes) {
                if (lexeme.tags.contains(Tags::Type::Image)) {
                    QByteArray data = QByteArray::fromBase64(lexeme.data.toLatin1());
                    QImage image = QImage::fromData(data, "png");

                    if (!image.isNull()) {
                        // Рефакторинг QMatrix -> QTransform (Qt 6 совместимо)
                        QTransform transform;
                        transform.scale(CSystemPrinter::ImageScalingFactor, CSystemPrinter::ImageScalingFactor);
                        image = image.transformed(transform);

                        QBuffer buffer;
                        if (!image.isNull() && image.save(&buffer, "png")) {
                            lexeme.data = buffer.data().toBase64();
                        }
                    }
                }
                execTags(lexeme, line);
            }
            receipt << line.toString();
        }
    }

    QRegularExpression nonSpaceRegex(QStringLiteral("[^ ]"));
    QRegularExpression endSpaceRegex(QStringLiteral(" +$"));

    for (int i = 0; i < receipt.size(); ++i) {
        QRegularExpressionMatch match = nonSpaceRegex.match(receipt[i]);
        int index = match.hasMatch() ? static_cast<int>(match.capturedStart()) : -1;

        if (index > 0) {
            receipt[i] = receipt[i].replace(0, index, QStringLiteral("&nbsp;").repeated(index));
        }
        // Замена QRegExp на QRegularExpression
        receipt[i] = receipt[i].replace(endSpaceRegex, QString());
    }

    qreal bottomMargin =
        getConfigParameter(PrinterSettings::PrintPageNumber).toBool() ? 12.7 : CSystemPrinter::DefaultMargin;
    qreal leftMargin = getConfigParameter(PrinterSettings::LeftMargin, mSideMargin).toDouble();
    qreal rightMargin = getConfigParameter(PrinterSettings::RightMargin, mSideMargin).toDouble();

    // Qt 6: Использование QPageLayout и QMarginsF для установки полей
    mPrinter.setPageMargins(QMarginsF(leftMargin, CSystemPrinter::DefaultMargin, rightMargin, bottomMargin),
                            QPageLayout::Millimeter);

    QTextDocument document;
    QString toPrint = receipt.join(CSystemPrinter::BRtag) + CSystemPrinter::BRtag;

    int lineSpacing = getConfigParameter(PrinterSettings::LineSpacing).toInt();
    int fontSize = getConfigParameter(PrinterSettings::FontSize).toInt();

    QStringList textParameters;
    // Qt 6: families() теперь статический метод класса QFontDatabase
    if (QFontDatabase::families().contains(QStringLiteral("Terminal"))) {
        // textParameters << "font-family: Terminal";
    }

    if (lineSpacing)
        textParameters << QStringLiteral("line-height: %1%").arg(lineSpacing);
    if (fontSize)
        textParameters << QStringLiteral("font-size: %1px").arg(fontSize);

    toPrint =
        QStringLiteral("<style>p {%1} </style><p>%2</p>").arg(textParameters.join(QStringLiteral("; "))).arg(toPrint);
    document.setHtml(toPrint);
    document.print(&mPrinter);

    return true;
}

//--------------------------------------------------------------------------------
bool SystemPrinter::getStatus(TStatusCodes &aStatusCodes) {
    using namespace SDK::Driver;

    if (!isConnected()) {
        return false;
    }

    if (mPrinter.printerState() == QPrinter::Error) {
        aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
    }

    TStatusGroupNames groupNames;
    ISysUtils::getPrinterStatus(mPrinter.printerName(), aStatusCodes, groupNames);

    if (mLastStatusesNames != groupNames) {
        mLastStatusesNames = groupNames;
        TStatusCollection statuses;

        for (int statusCode : aStatusCodes) {
            statuses[mStatusCodesSpecification->value(statusCode).warningLevel].insert(statusCode);
        }

        EWarningLevel::Enum warningLevel = getWarningLevel(statuses);
        LogLevel::Enum logLevel =
            (warningLevel == EWarningLevel::Error)
                ? LogLevel::Error
                : ((warningLevel == EWarningLevel::Warning) ? LogLevel::Warning : LogLevel::Normal);

        QString log = QStringLiteral("Device codes has changed:");

        for (auto it = mLastStatusesNames.begin(); it != mLastStatusesNames.end(); ++it) {
            // Явно упаковываем значение в QVariant через статический метод
            QVariant val = QVariant::fromValue(it.value());
            QStringList statusNames;

            // 2. Явное извлечение данных (Qt 6 стиль)
            if (val.canConvert<QStringList>()) {
                statusNames = val.toStringList();
            } else {
                // Извлекаем QSet через шаблон value<T>().
                // В Qt 6 это безопасный способ получить доступ к кастомным типам в QVariant.
                QSet<QString> statusSet = val.value<QSet<QString>>();

                // В Qt 6 метод .toList() удален. Конструктор от итераторов — стандарт 2026 года.
                statusNames = QStringList(statusSet.begin(), statusSet.end());
            }

            // 3. Сортировка по стандарту C++14 (замена qSort, требуется #include <algorithm>)
            std::sort(statusNames.begin(), statusNames.end());

            // 4. Формирование лога с использованием QStringLiteral и корректным выравниванием
            log += QStringLiteral("\n%1 : %2").arg(it.key(), 12).arg(statusNames.join(QStringLiteral(", ")));
        }

        toLog(logLevel, log);
    }

    return true;
}
