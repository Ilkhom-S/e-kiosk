/* @file Системный принтер. */

#include "SystemPrinter.h"

#include <QtCore/QBuffer>
#include <QtCore/QRegularExpression>
#include <QtGui/QFontDatabase>
#include <QtGui/QTextDocument>
#include <QtGui/QTransform>
#include <QtPrintSupport/QPrinter>

#include <algorithm>

#include "SysUtils/ISysUtils.h"

namespace PrinterSettings = CHardware::Printer::Settings;

//--------------------------------------------------------------------------------
/// Константы системного принтера.
namespace CSystem_Printer {
const char BRtag[] = "<br>";
} // namespace CSystem_Printer

//--------------------------------------------------------------------------------
System_Printer::System_Printer() : m_SideMargin(1.0) {
    // данные устройства
    m_DeviceName = "System printer";
    setConfigParameter(CHardware::Printer::NeedSeparating, false);
    m_LineFeed = false;

    // теги
    m_TagEngine = Tags::PEngine(new CSystem_Printer::TagEngine());
}

//--------------------------------------------------------------------------------
bool System_Printer::isConnected() {
    return m_Printer.isValid();
}

//--------------------------------------------------------------------------------
bool System_Printer::updateParameters() {
    if (m_Printer.printerName().isEmpty()) {
        return true;
    }

    m_DeviceName = QStringLiteral("System printer (%1)").arg(m_Printer.printerName());
    QVariantMap deviceData = ISysUtils::getPrinterData(m_Printer.printerName());

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
    if (!ISysUtils::setPrintingQueuedMode(m_Printer.printerName(), errorMessage)) {
        toLog(LogLevel::Warning,
              QStringLiteral("Failed to change printing queued mode, ") + errorMessage);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool System_Printer::printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt) {
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
                        transform.scale(CSystem_Printer::ImageScalingFactor,
                                        CSystem_Printer::ImageScalingFactor);
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

    for (auto &i : receipt) {
        QRegularExpressionMatch match = nonSpaceRegex.match(i);
        int index = match.hasMatch() ? static_cast<int>(match.capturedStart()) : -1;

        if (index > 0) {
            i = i.replace(0, index, QStringLiteral("&nbsp;").repeated(index));
        }
        // Замена QRegExp на QRegularExpression
        i = i.replace(endSpaceRegex, QString());
    }

    qreal bottomMargin = getConfigParameter(PrinterSettings::PrintPageNumber).toBool()
                             ? 12.7
                             : CSystem_Printer::DefaultMargin;
    qreal leftMargin = getConfigParameter(PrinterSettings::LeftMargin, m_SideMargin).toDouble();
    qreal rightMargin = getConfigParameter(PrinterSettings::RightMargin, m_SideMargin).toDouble();

    // Qt 6: Использование QPageLayout и QMarginsF для установки полей
    m_Printer.setPageMargins(
        QMarginsF(leftMargin, CSystem_Printer::DefaultMargin, rightMargin, bottomMargin),
        QPageLayout::Millimeter);

    QTextDocument document;
    QString toPrint = receipt.join(CSystem_Printer::BRtag) + CSystem_Printer::BRtag;

    int lineSpacing = getConfigParameter(PrinterSettings::LineSpacing).toInt();
    int fontSize = getConfigParameter(PrinterSettings::FontSize).toInt();

    QStringList textParameters;
    // Qt 6: families() теперь статический метод класса QFontDatabase
    if (QFontDatabase::families().contains(QStringLiteral("Terminal"))) {
        // textParameters << "font-family: Terminal";
    }

    if (lineSpacing != 0) {
        textParameters << QStringLiteral("line-height: %1%").arg(lineSpacing);
    }
    if (fontSize != 0) {
        textParameters << QStringLiteral("font-size: %1px").arg(fontSize);
    }

    toPrint = QStringLiteral("<style>p {%1} </style><p>%2</p>")
                  .arg(textParameters.join(QStringLiteral("; ")))
                  .arg(toPrint);
    document.setHtml(toPrint);
    document.print(&m_Printer);

    return true;
}

//--------------------------------------------------------------------------------
bool System_Printer::getStatus(TStatusCodes &aStatusCodes) {
    using namespace SDK::Driver;

    if (!isConnected()) {
        return false;
    }

    if (m_Printer.printerState() == QPrinter::Error) {
        aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
    }

    TStatusGroupNames groupNames;
    ISysUtils::getPrinterStatus(m_Printer.printerName(), aStatusCodes, groupNames);

    if (m_LastStatusesNames != groupNames) {
        m_LastStatusesNames = groupNames;
        TStatusCollection statuses;

        for (int statusCode : aStatusCodes) {
            statuses[m_StatusCodesSpecification->value(statusCode).warningLevel].insert(statusCode);
        }

        EWarningLevel::Enum warningLevel = getWarningLevel(statuses);
        LogLevel::Enum logLevel =
            (warningLevel == EWarningLevel::Error)
                ? LogLevel::Error
                : ((warningLevel == EWarningLevel::Warning) ? LogLevel::Warning : LogLevel::Normal);

        QString log = QStringLiteral("Device codes has changed:");

        for (auto it = m_LastStatusesNames.begin(); it != m_LastStatusesNames.end(); ++it) {
            // Явно упаковываем значение в QVariant через статический метод
            QVariant val = QVariant::fromValue(it.value());
            QStringList statusNames;

            // 2. Явное извлечение данных (Qt 6 стиль)
            if (val.canConvert<QStringList>()) {
                statusNames = val.toStringList();
            } else {
                // Извлекаем QSet через шаблон value<T>().
                // В Qt 6 это безопасный способ получить доступ к кастомным типам в QVariant.
                auto statusSet = val.value<QSet<QString>>();

       auto� Qt; 6 метод .toList() удален. Конструктор от итераторов стандарт 2026 года.
                statusNames = QStringList(statusSet.begin(), statusSet.end());
            }

            // 3. Сортировка по стандарту C++14 (замена qSort, требуется #include <algorithm>)
            std::sort(statusNames.begin(), statusNames.end());

            // 4. Формирование лога с использованием QStringLiteral и корректным выравниванием
            log += QStringLiteral("\n%1 : %2")
                       .arg(it.key(), 12)
                       .arg(statusNames.join(QStringLiteral(", ")));
        }

        toLog(logLevel, log);
    }

    return true;
}
