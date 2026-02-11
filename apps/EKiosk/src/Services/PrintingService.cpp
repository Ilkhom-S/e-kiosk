/* @file Сервис печати и формирования чеков. */

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QBuffer>
#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QFuture>
#include <QtCore/QMutexLocker>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringDecoder>
#include <QtCore/QXmlStreamWriter>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QtXml/QDomDocument>

// Thirdparty
#include <qzint.h>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Security.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/ExtensionsSettings.h>

// Driver SDK
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/FR/FiscalPrinterConstants.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/IFiscalPrinter.h>

#include "Common/ExitAction.h"
#include "FRReportConstants.h"
#include "PrintingCommands.h"
#include "Services/DatabaseService.h"
#include "Services/EventService.h"
#include "Services/PaymentService.h"
#include "Services/PluginService.h"
#include "Services/PrintConstants.h"
#include "Services/PrintingService.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
const char *PPSDK::IFiscalRegister::OFDNotSentSignal = SIGNAL(OFDNotSent(bool));

//------------------------------------------------------------------------------
namespace CPrintingService {
const QString ReceiptDateTimeFormat = "dd.MM.yyyy hh:mm:ss";
const QString ConditionTag = "@@";

const QString MaskedFieldPostfix = "_MASKED";
const QString DisplayPostfix = "_DISPLAY";
} // namespace CPrintingService

//---------------------------------------------------------------------------
PrintingService::PrintingService(IApplication *aApplication)
    : m_Application(aApplication), m_DatabaseUtils(nullptr), m_DeviceService(nullptr),
      m_PrintingMode(DSDK::EPrintingModes::None), m_ServiceOperation(false),
      m_Random_ReceiptsID(false), m_NextReceiptIndex(1),
      m_Random_Generator(
          static_cast<unsigned>(QDateTime::currentDateTime().currentMSecsSinceEpoch())),
      m_EnableBlankFiscalData(false), m_FiscalRegister(nullptr) {
    setLog(m_Application->getLog());
}

//---------------------------------------------------------------------------
PrintingService::~PrintingService() = default;

//---------------------------------------------------------------------------
QString PrintingService::getName() const {
    return CServices::PrintingService;
}

//---------------------------------------------------------------------------
const QSet<QString> &PrintingService::getRequiredServices() const {
    static QSet<QString> requiredServices =
        QSet<QString>() << CServices::DeviceService << CServices::DatabaseService
                        << CServices::SettingsService << CServices::PluginService
                        << CServices::PaymentService;

    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap PrintingService::getParameters() const {
    QVariantMap parameters;

    parameters[PPSDK::CServiceParameters::Printing::ReceiptCount] = getReceiptCount();

    return parameters;
}

//---------------------------------------------------------------------------
void PrintingService::resetParameters(const QSet<QString> &aParameters) {
    if (aParameters.contains(PPSDK::CServiceParameters::Printing::ReceiptCount)) {
        m_DatabaseUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                        PPSDK::CDatabaseConstants::Parameters::ReceiptCount,
                                        0);
    }
}

//---------------------------------------------------------------------------
bool PrintingService::initialize() {
    // Запрашиваем доступные устройства.
    m_DeviceService = m_Application->getCore()->getDeviceService();

    loadReceiptTemplates();
    loadTags();

    updateHardwareConfiguration();
    createFiscalRegister();

    m_DatabaseUtils =
        DatabaseService::instance(m_Application)->getDatabaseUtils<IHardwareDatabaseUtils>();

    connect(m_DeviceService, SIGNAL(configurationUpdated()), SLOT(updateHardwareConfiguration()));
    connect(&m_FutureWatcher, SIGNAL(finished()), this, SLOT(taskFinished()));

    return true;
}

//------------------------------------------------------------------------------
void PrintingService::finishInitialize() {}

//---------------------------------------------------------------------------
bool PrintingService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool PrintingService::shutdown() {
    m_FutureWatcher.waitForFinished();

    foreach (DSDK::IPrinter *printer, m_PrinterDevices) {
        m_DeviceService->releaseDevice(printer);
    }

    if (m_FiscalRegister) {
        SDK::Plugin::IPluginLoader *pluginLoader =
            PluginService::instance(m_Application)->getPluginLoader();

        pluginLoader->destroyPlugin(dynamic_cast<SDK::Plugin::IPlugin *>(m_FiscalRegister));
        m_FiscalRegister = nullptr;
    }

    return true;
}

//---------------------------------------------------------------------------
PrintCommand *PrintingService::getPrintCommand(const QString &aReceiptType) {
    if (aReceiptType == PPSDK::CReceiptType::Payment) {
        return new PrintPayment(aReceiptType, this);
    }
    if (aReceiptType == PPSDK::CReceiptType::Balance ||
        aReceiptType == PPSDK::CReceiptType::XReport) {
        return new PrintBalance(aReceiptType, this);
    }
    if (aReceiptType == PPSDK::CReceiptType::DispenserBalance ||
        aReceiptType == PPSDK::CReceiptType::DispenserEncashment) {
        auto *command = new PrintBalance(aReceiptType, this);
        command->setFiscal(false);
        return command;
    }
    if (aReceiptType == PPSDK::CReceiptType::Encashment) {
        return new PrintEncashment(aReceiptType, this);
    }
    if (aReceiptType == PPSDK::CReceiptType::ZReport) {
        return new PrintZReport(aReceiptType, this, false);
    }
    if (aReceiptType == PPSDK::CReceiptType::ZReportFull) {
        return new PrintZReport(aReceiptType, this, true);
    }
    return new PrintReceipt(aReceiptType, this);
}

//---------------------------------------------------------------------------
bool PrintingService::canPrintReceipt(const QString &aReceiptType, bool aRealCheck) {
    DSDK::IPrinter *printer = takePrinter(aReceiptType, aRealCheck);
    giveBackPrinter(printer);

    return printer != nullptr;
}

//---------------------------------------------------------------------------
int PrintingService::printReceipt(const QString &aReceiptType,
                                  const QVariantMap &aParameters,
                                  const QString &aReceiptTemplate,
                                  DSDK::EPrintingModes::Enum aPrintingMode,
                                  bool aServiceOperation) {
    m_PrintingMode = aPrintingMode;
    m_ServiceOperation = aServiceOperation;

    QStringList receiptTemplates;
    receiptTemplates
        // Извлекаем шаблон 'PROCESSING_TYPE'_aReceiptTemplate
        << (QString("%1_%2")
                .arg(aParameters[PPSDK::CPayment::Parameters::Type].toString())
                .arg(aReceiptTemplate))
               .toLower()
        // Извлекаем обычный шаблон для чека нужного типа.
        << aReceiptTemplate.toLower()
        // Резервный шаблон
        << SDK::PaymentProcessor::CReceiptType::Payment;

    foreach (auto templateName, receiptTemplates) {
        if (templateName == SDK::PaymentProcessor::CReceiptType::Disabled) {
            break;
        }

        if (m_CachedReceipts.contains(templateName)) {
            auto *printCommand = getPrintCommand(aReceiptType);
            printCommand->setReceiptTemplate(templateName);

            return perform_Print(printCommand, aParameters, m_CachedReceipts[templateName]);
        }
    }

    toLog(LogLevel::Error,
          QString("Failed to print receipt. Missing receipt template : %1.").arg(aReceiptTemplate));

    QMetaObject::invokeMethod(
        this, "printEmptyReceipt", Qt::QueuedConnection, Q_ARG(int, 0), Q_ARG(bool, true));

    return 0;
}

//---------------------------------------------------------------------------
void PrintingService::printEmptyReceipt(int aJobIndex, bool aError) {
    emit receiptPrinted(aJobIndex, aError);
}

//---------------------------------------------------------------------------
bool PrintingService::printReceiptDirected(DSDK::IPrinter *aPrinter,
                                           const QString &aReceiptTemplate,
                                           const QVariantMap &aParameters) {
    m_PrintingMode = DSDK::EPrintingModes::None;

    // Извлекаем шаблон для чека нужного типа.
    if (!m_CachedReceipts.contains(aReceiptTemplate.toLower())) {
        toLog(LogLevel::Error, QString("Missing receipt template : %1.").arg(aReceiptTemplate));
        return false;
    }

    QScopedPointer<PrintCommand> printCommand(getPrintCommand(QString()));
    printCommand->setReceiptTemplate(aReceiptTemplate);

    QVariantMap configuration;
    configuration.insert(CHardwareSDK::Printer::PrintingMode, m_PrintingMode);
    configuration.insert(CHardwareSDK::Printer::ServiceOperation, m_ServiceOperation);
    aPrinter->setDeviceConfiguration(configuration);

    // TODO: нужно ли увеличивать счетчик чеков?
    return printCommand->print(aPrinter, aParameters);
}

//---------------------------------------------------------------------------
template <class ResultT, class T> ResultT &joinMap(ResultT &aResult, const T &aParameters2) {
    for (auto it = aParameters2.begin(); it != aParameters2.end(); ++it) {
        aResult.insert(it.key(), it.value());
    }

    return aResult;
}

//---------------------------------------------------------------------------
int PrintingService::perform_Print(PrintCommand *aCommand,
                                   const QVariantMap &aParameters,
                                   QStringList aReceiptTemplate) {
    // Функция, в которой производится печать. Должно быть исключено обращение к общим для разных
    // принтеров данным.
    m_PrintingFunction = [this, aReceiptTemplate](int aJobIndex,
                                                  PrintCommand *aCommand,
                                                  QVariantMap aParameters) -> bool {
        QVariantMap staticParameters;
        joinMap(staticParameters, m_StaticParameters);
        QVariantMap paymentParameters = joinMap(aParameters, staticParameters);

        auto *printer = takePrinter(aCommand->getReceiptType(), false);
        auto *paymentPrintingCommand = dynamic_cast<PrintPayment *>(aCommand);

        auto makeResult = [&](bool result, const QString &aLog) -> bool {
            if (aCommand) {
                delete aCommand;
            }
            toLog(result ? LogLevel::Normal : LogLevel::Error, aLog);
            emit receiptPrinted(aJobIndex, !result);
            return result;
        };

        if (!printer) {
            if (!getFiscalRegister()) {
                return makeResult(
                    false, "Failed to process receipt without printer due to no fiscal register");
            }
            if (!paymentPrintingCommand) {
                return makeResult(
                    false, "Failed to process receipt without printer due to no printing command");
            }

            bool result = paymentPrintingCommand->makeFiscalByFR(paymentParameters);

            return makeResult(result, "Send receipt printed without printer");
        }

        QVariantMap configuration;
        configuration.insert(CHardwareSDK::Printer::PrintingMode, m_PrintingMode);
        configuration.insert(CHardwareSDK::Printer::ServiceOperation, m_ServiceOperation);
        configuration.insert(CHardwareSDK::Printer::ReceiptTemplate, aReceiptTemplate);
        printer->setDeviceConfiguration(configuration);

        bool result = aCommand->print(printer, paymentParameters);

        if (result) {
            incrementReceiptCount(printer);
        }

        giveBackPrinter(printer);

        return makeResult(result, "Send receipt printed");
    };

    int taskIndex = m_NextReceiptIndex.fetchAndAddOrdered(1);

    if (taskIndex != 1 && !m_FutureWatcher.isFinished()) {
        Task task = {taskIndex, aCommand, aParameters};

        m_Queue.enqueue(task);
    } else {
        m_FutureWatcher.setFuture(
            QtConcurrent::run(m_PrintingFunction, taskIndex, aCommand, aParameters));
    }

    return taskIndex;
}

//---------------------------------------------------------------------------
void PrintingService::taskFinished() {
    if (!m_Queue.isEmpty()) {
        Task task = m_Queue.dequeue();

        m_FutureWatcher.setFuture(
            QtConcurrent::run(m_PrintingFunction, task.index, task.command, task.parameters));
    }
}

//---------------------------------------------------------------------------
int PrintingService::printReport(const QString &aReceiptType, const QVariantMap &aParameters) {
    auto *printCommand = getPrintCommand(aReceiptType);

    auto *balanceCommand = dynamic_cast<PrintBalance *>(printCommand);
    if (balanceCommand && aParameters.contains(CPrintConstants::NoFiscal)) {
        // Включаем нефискальный режим
        balanceCommand->setFiscal(false);
    }

    printCommand->setReceiptTemplate(aReceiptType);

    QVariantMap parameters;

    return perform_Print(printCommand,
                         joinMap(joinMap(parameters, m_StaticParameters), aParameters));
}

//---------------------------------------------------------------------------
bool PrintingService::hasFiscalRegister() {
    return m_FiscalRegister && m_FiscalRegister->hasCapability(PPSDK::ERequestType::Receipt);
}

//---------------------------------------------------------------------------
void PrintingService::giveBackPrinter(DSDK::IPrinter *aPrinter) {
    if (aPrinter) {
        QMutexLocker lock(&m_AvailablePrintersMutex);

        m_AvailablePrinters.insert(aPrinter);

        m_PrintersAvailable.wakeOne();
    }
}

//---------------------------------------------------------------------------
DSDK::IPrinter *PrintingService::takePrinter(const QString &aReceiptType, bool aCheckOnline) {
    if (m_PrinterDevices.empty()) {
        toLog(LogLevel::Error, "Printers are not found in current configuration.");
        return nullptr;
    }

    // Пытаемся найти предпочтительный принтер.
    auto *settings = SettingsService::instance(m_Application)
                         ->getAdapter<SDK::PaymentProcessor::TerminalSettings>();
    QString preferredName = settings->getPrinterForReceipt(aReceiptType);

    QList<DSDK::IPrinter *> printers;
    auto *printCommand = getPrintCommand(aReceiptType);
    DSDK::IPrinter *preferred = nullptr;

    if (!preferredName.isEmpty()) {
        preferred = dynamic_cast<DSDK::IPrinter *>(m_DeviceService->acquireDevice(preferredName));

        if (preferred && m_PrinterDevices.contains(preferred) &&
            printCommand->canPrint(preferred, aCheckOnline)) {
            printers << preferred;
        }
    }

    foreach (auto printer, m_PrinterDevices) {
        if (printer && (printer != preferred) && printCommand->canPrint(printer, aCheckOnline)) {
            printers << printer;
        }
    }

    delete printCommand;

    if (printers.isEmpty()) {
        toLog(LogLevel::Error,
              QString("No any available printer for type %1 with %2 checking.")
                  .arg(aReceiptType)
                  .arg(aCheckOnline ? "online" : "offline"));
        return nullptr;
    }

    DSDK::IPrinter *printer = printers.first();

    // Ждем, пока принтер не появится в списке доступных.
    QMutexLocker lock(&m_AvailablePrintersMutex);

    if (!m_AvailablePrinters.contains(printer)) {
        m_PrintersAvailable.wait(&m_AvailablePrintersMutex);
    }

    m_AvailablePrinters.remove(printer);

    return printer;
}

//---------------------------------------------------------------------------
void PrintingService::incrementReceiptCount(DSDK::IPrinter *aPrinter) const {
    QString deviceConfigName = m_DeviceService->getDeviceConfigName(aPrinter);

    // Увеличиваем количество напечатанных чеков для конкретного принтера.
    QVariant receiptCount = m_DatabaseUtils->getDeviceParam(
        deviceConfigName, PPSDK::CDatabaseConstants::Parameters::ReceiptCount);
    receiptCount = QVariant::fromValue(receiptCount.toInt() + 1);
    m_DatabaseUtils->setDeviceParam(
        deviceConfigName, PPSDK::CDatabaseConstants::Parameters::ReceiptCount, receiptCount);

    // Увеличиваем количество напечатанных чеков для терминала.
    receiptCount =
        m_DatabaseUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                        PPSDK::CDatabaseConstants::Parameters::ReceiptCount);
    receiptCount = QVariant::fromValue(receiptCount.toInt() + 1);
    m_DatabaseUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                    PPSDK::CDatabaseConstants::Parameters::ReceiptCount,
                                    receiptCount);
}

//---------------------------------------------------------------------------
unsigned PrintingService::getReceiptID() const {
    if (m_Random_ReceiptsID) {
        return m_Random_Generator();
    }
    return getReceiptCount() + 1;
}

//---------------------------------------------------------------------------
int PrintingService::getReceiptCount() const {
    QVariant receiptCount =
        m_DatabaseUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                        PPSDK::CDatabaseConstants::Parameters::ReceiptCount);

    return receiptCount.isNull() ? 0 : receiptCount.toInt();
}

//---------------------------------------------------------------------------
bool PrintingService::loadTags() {
    m_StaticParameters.clear();

    SettingsService *settingsService = SettingsService::instance(m_Application);

    auto *terminalSettings = settingsService->getAdapter<PPSDK::TerminalSettings>();

    PPSDK::SKeySettings key0 = terminalSettings->getKeys().value(0);

    if (key0.ap.isEmpty()) {
        toLog(LogLevel::Error, "Failed to retrieve terminal number from configs.");
        return false;
    }
    m_StaticParameters.insert(CPrintConstants::Term_Number, key0.ap);

    auto *dealerSettings = settingsService->getAdapter<PPSDK::DealerSettings>();

    // TODO: проверить на валидность.
    PPSDK::SPersonalSettings dealer = dealerSettings->getPersonalSettings();

    joinMap(m_StaticParameters, dealer.m_PrintingParameters);

    m_StaticParameters.insert(CPrintConstants::DealerName, dealer.name);
    m_StaticParameters.insert(CPrintConstants::DealerPhone, dealer.phone);
    m_StaticParameters.insert(CPrintConstants::DealerSupportPhone, dealer.phone);
    m_StaticParameters.insert(CPrintConstants::DealerAddress, dealer.address);
    m_StaticParameters.insert(CPrintConstants::DealerBusinessAddress,
                              dealer.businessAddress.isEmpty() ? dealer.address
                                                               : dealer.businessAddress);
    m_StaticParameters.insert(CPrintConstants::DealerInn, dealer.inn);
    m_StaticParameters.insert(CPrintConstants::DealerKbk, dealer.kbk);
    m_StaticParameters.insert(CPrintConstants::DealerIsBank, dealer.isBank);
    m_StaticParameters.insert(CPrintConstants::PointAddress, dealer.pointAddress);
    m_StaticParameters.insert(CPrintConstants::PointName, dealer.pointName);
    m_StaticParameters.insert(CPrintConstants::PointExternalID, dealer.pointExternalID);
    m_StaticParameters.insert(CPrintConstants::BankName, dealer.bankName);
    m_StaticParameters.insert(CPrintConstants::BankBik, dealer.bankBik);
    m_StaticParameters.insert(CPrintConstants::BankPhone, dealer.bankPhone);
    m_StaticParameters.insert(CPrintConstants::BankAddress, dealer.bankAddress);
    m_StaticParameters.insert(CPrintConstants::BankInn, dealer.bankInn);

    QString currency = terminalSettings->getCurrencySettings().code;

    if (!currency.isEmpty()) {
        m_StaticParameters.insert(CPrintConstants::Currency,
                                  terminalSettings->getCurrencySettings().name);
    } else {
        toLog(LogLevel::Warning,
              "Failed to retrieve currency settings from configs. Tag <CURRENCY> will be printed "
              "empty on all receipts!");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
QStringList PrintingService::getReceipt(const QString &aReceiptTemplate,
                                        const QVariantMap &aParameters) {
    if (!m_CachedReceipts.contains(aReceiptTemplate.toLower())) {
        toLog(LogLevel::Error, QString("Missing receipt template %1.").arg(aReceiptTemplate));
    }

    QStringList receipt = m_CachedReceipts.value(aReceiptTemplate.toLower());
    expandTags(receipt, aParameters);

    return receipt;
}

//---------------------------------------------------------------------------
QString maskedString(const QString &aString, bool aNeedMask) {
    if (aNeedMask) {
        int oneFourthLen = qMin(aString.size() / 4, 4);

        return aString.left(oneFourthLen) +
               QString("*").repeated(aString.size() - oneFourthLen * 2) +
               aString.right(oneFourthLen);
    }

    return aString;
}

//---------------------------------------------------------------------------
QString PrintingService::convertImage2base64(const QString &aString) {
    QString result = aString;

    for (int extLen = 3; extLen <= 4; extLen++) {
        // \[img\s*\].*((?:[\w]\:|\\)?((\\|/)?[a-z_\-\s0-9\.]+)+\.[a-z]{3,4})
        QRegularExpression imgPattern = QRegularExpression(
            QString(
                "\\[img\\s*\\].*((?:[\\w]\\:|\\\\)?((\\\\|/)?[a-z_\\-\\s0-9\\.]+)+\\.[a-z]{%1})")
                .arg(extLen),
            QRegularExpression::CaseInsensitiveOption);

        ////////imgPattern.setMinimal(true); // Removed for Qt5/6 compatibility // Removed for Qt5/6
        /// compatibility //
        /// Removed for Qt5/6 compatibility // Removed for Qt5/6 compatibility

        int offset = 0;
        QRegularExpressionMatch match;
        while ((match = imgPattern.match(result, offset)).hasMatch()) {
            QString img = "<image>";

            QFile file(match.captured(1));
            if (file.open(QIODevice::ReadOnly)) {
                img = QString::fromLatin1(file.readAll().toBase64());
            } else {
                toLog(LogLevel::Error,
                      QString("Error load image '%1': %2")
                          .arg(match.captured(1))
                          .arg(file.errorString()));
            }

            result.replace(match.capturedStart(1), match.captured(1).length(), img);
            offset = match.capturedStart(1) + img.size();
        }
    }

    return result;
}

//---------------------------------------------------------------------------
QString PrintingService::generateQR(const QString &aString) {
    QString result = aString;

    auto generateQRCode = [=](const QString &aText, int aSize, int aLeftMargin) -> QString {
        QImage image(QSize(aSize + aLeftMargin, aSize), QImage::Format_ARGB32);
        image.fill(QColor("transparent"));

        Zint::QZint zint;

        zint.setWidth(0);
        zint.setHeight(static_cast<float>(aSize));
        zint.setText(aText);
        zint.setWhitespace(0);
        zint.setBorderType(0); // Zint::QZint::NO_BORDER
        zint.setInputMode(DATA_MODE);
        zint.setHideText(true);
        zint.setSymbol(BARCODE_QRCODE);
        zint.setFgColor(QColor("black"));
        zint.setBgColor(QColor("white"));

        {
            QPainter painter(&image);
            painter.fillRect(QRectF(0, 0, image.width(), image.height()), QColor("white"));
            zint.render(painter,
                        QRectF(aLeftMargin, 0, image.width() - aLeftMargin, image.height()),
                        Zint::QZint::KeepAspectRatio);
            painter.end();
        }

        if (zint.hasErrors()) {
            toLog(LogLevel::Error, QString("Failed render QR code: %1.").arg(zint.lastError()));
        } else {
            QBuffer buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                image.save(&buffer, "png");

                return buffer.data().toBase64();
            }
        }

        return "";
    };

    QRegularExpression qrPattern(
        "\\[qr(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?\\](.*)\\[/qr\\]",
        QRegularExpression::CaseInsensitiveOption);

    ////////qrPattern.setMinimal(true); // Removed for Qt5/6 compatibility // Removed for Qt5/6
    /// compatibility // Removed
    /// for Qt5/6 compatibility // Removed for Qt5/6 compatibility

    int offset = 0;
    QRegularExpressionMatch match;
    while ((match = qrPattern.match(result, offset)).hasMatch()) {
        offset = match.capturedStart();
        int size = 200;
        int left_margin = 0;

        for (int i = 2; i < 6; i += 3) {
            if (match.captured(i).toLower() == "size") {
                size = match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : size;
            } else if (match.captured(i).toLower() == "left_margin") {
                left_margin =
                    match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : left_margin;
            }
        }

        QString content = match.captured(7);
        QString qrImage = generateQRCode(content, size, left_margin);

        QString img = qrImage.isEmpty() ? "<qr-code>" : QString("[img]%1[/img]").arg(qrImage);

        result.replace(offset, match.captured(0).length(), img);
        offset += img.size();
    }

    return result;
}

//---------------------------------------------------------------------------
QString PrintingService::generatePDF417(const QString &aString) {
    QString result = aString;

    auto generatePDFCode = [=](const QString &aText, int aSize, int aLeftMargin) -> QString {
        int divider = aText.size() > 100 ? 2 : 3;

        QImage image(QSize(aSize + aLeftMargin, aSize / divider), QImage::Format_ARGB32);
        image.fill(QColor("transparent"));

        Zint::QZint zint;

        zint.setWidth(aSize);
        zint.setHeight(static_cast<float>(aSize) / divider);
        zint.setText(aText);
        zint.setWhitespace(0);
        zint.setBorderType(0); // Zint::QZint::NO_BORDER
        zint.setInputMode(UNICODE_MODE);
        zint.setHideText(true);
        zint.setSymbol(BARCODE_PDF417);
        zint.setFgColor(QColor("black"));
        zint.setBgColor(QColor("white"));

        QPainter painter(&image);
        painter.fillRect(QRectF(0, 0, image.width(), image.height()), QColor("white"));
        zint.render(painter, QRectF(aLeftMargin, 0, image.width() - aLeftMargin, image.height()));
        painter.end();

        if (zint.hasErrors()) {
            toLog(LogLevel::Error,
                  QString("Failed render PDF-417 code: %1.").arg(zint.lastError()));
        } else {
            QBuffer buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                image.save(&buffer, "png");

                return buffer.data().toBase64();
            }
        }

        return "";
    };

    QRegularExpression qrPattern("\\[pdf417(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?(\\s*(\\w+)\\s*=\\s*("
                                 "\\d+)\\s*)?\\](.*)\\[/pdf417\\]",
                                 QRegularExpression::CaseInsensitiveOption);

    ////////qrPattern.setMinimal(true); // Removed for Qt5/6 compatibility // Removed for Qt5/6
    /// compatibility // Removed
    /// for Qt5/6 compatibility // Removed for Qt5/6 compatibility

    int offset = 0;
    QRegularExpressionMatch match;
    while ((match = qrPattern.match(result, offset)).hasMatch()) {
        int size = 200;
        int left_margin = 0;

        for (int i = 2; i < 6; i += 3) {
            if (match.captured(i).toLower() == "size") {
                size = match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : size;
            } else if (match.captured(i).toLower() == "left_margin") {
                left_margin =
                    match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : left_margin;
            }
        }

        QString content = match.captured(7);
        QString qrImage = generatePDFCode(content, size, left_margin);

        QString img = qrImage.isEmpty() ? "<pdf417-code>" : QString("[img]%1[/img]").arg(qrImage);

        result.replace(offset, match.captured(0).length(), img);
        offset += img.size();
    }

    return result;
}

//---------------------------------------------------------------------------
QString PrintingService::generate1D(const QString &aString) {
    QString result = aString;

    auto generatePDFCode = [=](const QString &aText, int aSize, int aLeftMargin) -> QString {
        int divider = aText.size() > 100 ? 2 : 3;

        QImage image(QSize(aSize + aLeftMargin, aSize / divider), QImage::Format_ARGB32);
        image.fill(QColor("transparent"));

        Zint::QZint zint;

        zint.setWidth(aSize);
        zint.setHeight(static_cast<float>(aSize) / divider);
        zint.setText(aText);
        zint.setWhitespace(0);
        zint.setBorderType(0); // Zint::QZint::NO_BORDER
        zint.setInputMode(UNICODE_MODE);
        zint.setHideText(true);
        zint.setSymbol(BARCODE_CODE128);
        zint.setFgColor(QColor("black"));
        zint.setBgColor(QColor("white"));

        QPainter painter(&image);
        painter.fillRect(QRectF(0, 0, image.width(), image.height()), QColor("white"));
        zint.render(painter, QRectF(aLeftMargin, 0, image.width() - aLeftMargin, image.height()));
        painter.end();

        if (zint.hasErrors()) {
            toLog(LogLevel::Error,
                  QString("Failed render PDF-417 code: %1.").arg(zint.lastError()));
        } else {
            QBuffer buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                image.save(&buffer, "png");

                return buffer.data().toBase64();
            }
        }

        return "";
    };

    QRegularExpression qrPattern(
        "\\[1d(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?\\](.*)\\[/1d\\]",
        QRegularExpression::CaseInsensitiveOption);

    ////////qrPattern.setMinimal(true); // Removed for Qt5/6 compatibility // Removed for Qt5/6
    /// compatibility // Removed
    /// for Qt5/6 compatibility // Removed for Qt5/6 compatibility

    int offset = 0;
    QRegularExpressionMatch match;
    while ((match = qrPattern.match(result, offset)).hasMatch()) {
        int size = 200;
        int left_margin = 0;

        for (int i = 2; i < 6; i += 3) {
            if (match.captured(i).toLower() == "size") {
                size = match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : size;
            } else if (match.captured(i).toLower() == "left_margin") {
                left_margin =
                    match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : left_margin;
            }
        }

        QString content = match.captured(7);
        QString qrImage = generatePDFCode(content, size, left_margin);

        QString img = qrImage.isEmpty() ? "<pdf417-code>" : QString("[img]%1[/img]").arg(qrImage);

        result.replace(offset, match.captured(0).length(), img);
        offset += img.size();
    }

    return result;
}

//---------------------------------------------------------------------------
QString PrintingService::generateLine(const QString &aString) {
    QString result = aString;

    auto generateLine = [](int aSize, int aHeight, int aDense) -> QString {
        QImage image(QSize(aSize, aHeight), QImage::Format_ARGB32);
        image.fill(QColor("transparent"));

        Qt::BrushStyle style;

        if (aDense <= 0) {
            style = Qt::SolidPattern;
        } else if (aDense >= 6) {
            style = Qt::Dense7Pattern;
        } else {
            style = static_cast<Qt::BrushStyle>(aDense + 1);
        }

        QPainter painter(&image);
        painter.setBrush(style);
        painter.setPen(QColor("transparent"));
        painter.drawRect(0, 0, image.width(), image.height());
        painter.end();

        QBuffer buffer;
        if (buffer.open(QIODevice::WriteOnly)) {
            image.save(&buffer, "png");
            return buffer.data().toBase64();
        }

        return {};
    };

    QRegularExpression qrPattern(
        "\\[hr(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?\\](.*)\\[/hr\\]",
        QRegularExpression::CaseInsensitiveOption);

    ////////qrPattern.setMinimal(true); // Removed for Qt5/6 compatibility // Removed for Qt5/6
    /// compatibility // Removed
    /// for Qt5/6 compatibility // Removed for Qt5/6 compatibility

    int offset = 0;
    QRegularExpressionMatch match;
    while ((match = qrPattern.match(result, offset)).hasMatch()) {
        int size = 220;
        int height = 1;
        int dense = 0;

        for (int i = 2; i < 6; i += 3) {
            if (match.captured(i).toLower() == "size") {
                size = match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : size;
            } else if (match.captured(i).toLower() == "height") {
                height = match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : height;
            } else if (match.captured(i).toLower() == "ds") {
                dense = match.captured(i + 1).toInt() ? match.captured(i + 1).toInt() : dense;
            }
        }

        QString qrImage = generateLine(size, height, dense);

        QString img = qrImage.isEmpty() ? "<hr-code>" : QString("[img]%1[/img]").arg(qrImage);

        result.replace(offset, match.captured(0).length(), img);
        offset += img.size();
    }

    return result;
}

//---------------------------------------------------------------------------
void PrintingService::expandTags(QStringList &aReceipt, const QVariantMap &aParameters) {
    int operatorFieldIndex = 0;

    QVariantMap userParameters = aParameters;

    // Если есть дата создания платежа и она валидна, то подставляем на чек дату платежа
    userParameters[CPrintConstants::DateTime] =
        aParameters.contains(PPSDK::CPayment::Parameters::CreationDate) &&
                aParameters.value(PPSDK::CPayment::Parameters::CreationDate, "")
                    .toDateTime()
                    .isValid()
            ? aParameters.value(PPSDK::CPayment::Parameters::CreationDate, "")
                  .toDateTime()
                  .toString(CPrintingService::ReceiptDateTimeFormat)
            : QDateTime::currentDateTime().toLocalTime().toString(
                  CPrintingService::ReceiptDateTimeFormat);

    // Добавляем поля, зависящие от конкретного вызова.
    userParameters[CPrintConstants::ReceiptNumber] = getReceiptID();

    int providerId = aParameters.value(PPSDK::CPayment::Parameters::Provider, -1).toInt();
    PPSDK::SProvider provider = SettingsService::instance(m_Application)
                                    ->getAdapter<PPSDK::DealerSettings>()
                                    ->getProvider(providerId);
    PPSDK::SProvider mnpProvider =
        SettingsService::instance(m_Application)
            ->getAdapter<PPSDK::DealerSettings>()
            ->getMNPProvider(
                providerId,
                aParameters.value(PPSDK::CPayment::Parameters::MNPGatewayIn, 0).toLongLong(),
                aParameters.value(PPSDK::CPayment::Parameters::MNPGatewayOut, 0).toLongLong());

    if (!mnpProvider.isNull()) {
        userParameters[CPrintConstants::OpBrand] = mnpProvider.name;
    }

    PPSDK::SecurityFilter filter(mnpProvider, PPSDK::SProviderField::SecuritySubsystem::Printer);

    // Набор строк, полученных в результате применения всех подстановок.
    QStringList result;

    // Для каждого тега в шаблоне чека, заменяем его значением из параметров.
    for (auto it = aReceipt.begin(); it != aReceipt.end(); ++it) {
        QRegularExpression tagPattern("%(.*)%", QRegularExpression::CaseInsensitiveOption);

        ////////tagPattern.setMinimal(true); // Removed for Qt5/6 compatibility // Removed for Qt5/6
        /// compatibility //
        /// Removed for Qt5/6 compatibility // Removed for Qt5/6 compatibility

        int offset = 0;
        QRegularExpressionMatch match;
        while ((match = tagPattern.match(*it, offset)).hasMatch()) {
            QString tag = match.captured(1);

            // %% заменяем на %
            if (tag.isEmpty()) {
                it->replace(offset, match.captured(0).length(), "%");
                offset += 1;
                continue;
            }

            bool isMasked = tag.endsWith(CPrintingService::MaskedFieldPostfix);
            if (isMasked) {
                tag.remove(CPrintingService::MaskedFieldPostfix);
            }

            // Параметр пользователя?
            auto userParameter = userParameters.find(tag);

            if (userParameter != userParameters.end()) {
                // Параметр список?
                if (tag.startsWith("[") && tag.endsWith("]")) {
                    // удаляем строку с параметром-списком
                    aReceipt.erase(it, it);
                    it++;

                    QStringList listItems = userParameter.value().toStringList();
                    QStringList::const_iterator listIt;
                    for (listIt = listItems.constBegin(); listIt != listItems.constEnd();
                         listIt++) {
                        result.append(filter.apply(tag, *listIt));
                    }
                } else {
                    QString masked =
                        isMasked
                            ? maskedString(userParameter.value().toString(), isMasked)
                            : filter.apply(userParameter.key(), userParameter.value().toString());

                    it->replace(offset, match.captured(0).length(), masked);
                    offset += masked.length();
                }

                continue;
            }

            // Статический параметр?
            auto staticParameter = m_StaticParameters.find(tag);

            if (staticParameter != m_StaticParameters.end()) {
                QString masked = isMasked
                                     ? maskedString(staticParameter.value(), isMasked)
                                     : filter.apply(staticParameter.key(), staticParameter.value());

                it->replace(offset, match.captured(0).length(), masked);
                offset += masked.length();

                continue;
            }

            // Название или значение поля оператора?
            QString targetParameter;

            QRegularExpression operatorFieldPattern("FIELD_(.+)",
                                                    QRegularExpression::CaseInsensitiveOption);

            ////////operatorFieldPattern.setMinimal(true); // Removed for Qt5/6 compatibility //
            /// Removed for Qt5/6
            /// compatibility // Removed for Qt5/6 compatibility // Removed for Qt5/6 compatibility

            if (operatorFieldPattern.match(tag).capturedStart() != -1) {
                targetParameter = operatorFieldPattern.match(tag).captured(1);

                if (!filter.haveFilter(targetParameter)) {
                    targetParameter += CPrintingService::DisplayPostfix;
                }
            } else {
                operatorFieldPattern.setPattern("RAWFIELD_(.+)");

                if (operatorFieldPattern.match(tag).capturedStart() != -1) {
                    targetParameter = operatorFieldPattern.match(tag).captured(1);
                }
            }

            if (!targetParameter.isEmpty()) {
                if (!aParameters.contains(targetParameter)) {
                    toLog(LogLevel::Error,
                          QString("Operator parameter %1 required in receipt but missing in "
                                  "parameters list.")
                              .arg(targetParameter));
                } else {
                    QString masked =
                        isMasked ? maskedString(aParameters[targetParameter].toString(), isMasked)
                                 : filter.apply(targetParameter,
                                                aParameters[targetParameter].toString());

                    it->replace(offset, match.captured(0).length(), masked);
                    offset += masked.length();

                    continue;
                }
            }

            // Тег OPERATOR_FIELD
            if (tag == "OPERATOR_FIELD") {
                if (provider.isNull()) {
                    toLog(LogLevel::Error,
                          QString("Failed to expand operator field. Provider id %1 is not valid.")
                              .arg(providerId));
                }

                // Вставляем поля оператора.
                if (operatorFieldIndex < provider.fields.size()) {
                    PPSDK::SProviderField field = provider.fields.at(operatorFieldIndex);

                    QString fieldValue = aParameters.value(field.id, QString()).toString();

                    bool findNextField = true;

                    while (fieldValue.isEmpty()) {
                        if (operatorFieldIndex >= provider.fields.size()) {
                            findNextField = false;
                            break;
                        }

                        field = provider.fields.at(operatorFieldIndex);
                        fieldValue = aParameters.value(field.id, QString()).toString();

                        if (fieldValue.isEmpty()) {
                            ++operatorFieldIndex;
                        }
                    }

                    if (findNextField) {
                        QString masked;

                        if (filter.haveFilter(field.id)) {
                            masked = filter.apply(
                                field.id, aParameters.value(field.id, QString()).toString());
                        } else {
                            QString value =
                                aParameters
                                    .value(field.id + CPrintingService::DisplayPostfix, QString())
                                    .toString();
                            masked = isMasked ? maskedString(value, isMasked) : value;
                        }

                        QString replaceString = QString("%1: %2").arg(field.title).arg(masked);

                        it->replace(offset, match.captured(0).length(), replaceString);
                        offset = replaceString.length();

                        ++operatorFieldIndex;
                    }
                } else {
                    it->replace(match.captured(0), QString());
                }
            }

            // Оставляем поле пустым.
            it->replace(match.captured(0), QString());
        }

        // Предзагружаем содержимое тегов [IMG]
        *it = convertImage2base64(*it);

        // Преобразуем [QR] теги в [IMG]
        *it = generateQR(*it);

        // Преобразуем [PDF417] теги в [IMG]
        *it = generatePDF417(*it);

        // Преобразуем [1d] теги в [IMG]
        *it = generate1D(*it);

        // Преобразуем [hr] тег в [IMG]
        *it = generateLine(*it);

        // Обработаем тег с условием
        if (it->contains(CPrintingService::ConditionTag)) {
            QStringList l = it->split(CPrintingService::ConditionTag);

            toLog(LogLevel::Debug, QString("Evaluate receipt condition %1").arg(l.join(";")));

            if (QJSEngine().evaluate(l.first()).toBool()) {
                toLog(LogLevel::Debug, QString("Evaluate receipt result %1").arg(l.last()));
                result.append(l.last());
            } else {
                toLog(LogLevel::Debug, QString("Evaluate condition nothing for %1").arg(l.last()));
            }

            continue;
        }

        if (it->length() > 0) {
            result.append(*it);
        }
    }

    aReceipt = result;
}

//---------------------------------------------------------------------------
bool PrintingService::loadReceiptTemplate(const QFileInfo &aFileInfo) {
    if (aFileInfo.suffix().compare("xml", Qt::CaseInsensitive)) {
        toLog(LogLevel::Error,
              QString("Bad receipt template file extension : %1").arg(aFileInfo.fileName()));
        return false;
    }

    QDomDocument xmlFile(aFileInfo.fileName());
    QFile file(aFileInfo.filePath());

    if (!file.open(QIODevice::ReadOnly)) {
        toLog(LogLevel::Error, QString("Failed to open file %1").arg(aFileInfo.filePath()));
        return false;
    }

    xmlFile.setContent(&file);

    QDomElement body = xmlFile.documentElement();

    QStringList receiptContents;

    for (QDomNode node = body.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement row = node.toElement();

        if (row.tagName() == "string" || row.tagName() == "else") {
            QString prefix =
                row.attribute("if").isEmpty()
                    ? ""
                    : QString("%1%2").arg(row.attribute("if")).arg(CPrintingService::ConditionTag);
            receiptContents.append(QString("%1%2").arg(prefix).arg(row.text()));
        } else if (row.tagName() == "hr") {
            receiptContents.append("[hr]-[/hr]");
        }
    }

    if (receiptContents.isEmpty()) {
        toLog(LogLevel::Error,
              QString("Bad receipt template '%1': parse xml error").arg(aFileInfo.fileName()));
        return false;
    }

    m_CachedReceipts.insert(aFileInfo.baseName().toLower(), receiptContents);

    return true;
}

//---------------------------------------------------------------------------
void PrintingService::loadReceiptTemplates() {
    // Загружаем все шаблоны чеков в папке ./receipts
    QDir receiptDirectory;

    receiptDirectory.setPath(IApplication::getWorkingDirectory() + "/data/receipts");

    // Загружаем все файлы, которые есть в каталоге.
    foreach (const QFileInfo &fileInfo, receiptDirectory.entryInfoList(QDir::Files)) {
        loadReceiptTemplate(fileInfo);
    }

    // загружаем все шаблоны из папки пользовательских шаблонов чеков
    receiptDirectory.setPath(IApplication::getWorkingDirectory() + "/user/receipts");

    // Загружаем все файлы, которые есть в каталоге.
    foreach (const QFileInfo &fileInfo, receiptDirectory.entryInfoList(QDir::Files)) {
        loadReceiptTemplate(fileInfo);
    }
}

//---------------------------------------------------------------------------
void PrintingService::saveReceiptContent(const QString &aReceiptName,
                                         const QStringList &aContents) {
    // Получаем имя папки с чеками.
    QString suffix = QDate::currentDate().toString("yyyy.MM.dd");

    QDir path(IApplication::getWorkingDirectory() + "/receipts/" + suffix);

    if (!path.exists()) {
        if (!QDir().mkpath(path.path())) {
            toLog(LogLevel::Error, "Failed to create printed receipts folder.");
            return;
        }
    }

    auto fileName = path.path() + QDir::separator() + aReceiptName + ".txt";
    QFile file(fileName);

    // Сохраняем чек.
    if (file.open(QIODevice::WriteOnly)) {
        file.write(aContents.join("\r\n").toUtf8());
        file.close();
    } else {
        toLog(LogLevel::Error, QString("Failed to open file %1 for receipt.").arg(fileName));
    }
}

//---------------------------------------------------------------------------
void PrintingService::saveReceipt(const QVariantMap &aParameters, const QString &aReceiptTemplate) {
    QStringList receipt = getReceipt(aReceiptTemplate, aParameters);

    QString fileName = QTime::currentTime().toString("hhmmsszzz");

    if (aParameters.contains(PPSDK::CPayment::Parameters::ID)) {
        fileName += QString("_%1").arg(aParameters[PPSDK::CPayment::Parameters::ID].toString());
    }

    fileName += "_not_printed";

    saveReceiptContent(fileName, receipt);
}

//---------------------------------------------------------------------------
QString replaceTags(QString aMessage) {
    aMessage.replace("[br]", "\n", Qt::CaseInsensitive);
    aMessage.remove(
        QRegularExpression("\\[(b|dw|dh)\\]", QRegularExpression::CaseInsensitiveOption));
    aMessage.remove(
        QRegularExpression("\\[/(b|dw|dh)\\]", QRegularExpression::CaseInsensitiveOption));

    aMessage.remove(QRegularExpression("\\[img.?\\].*\\[/img\\]"));

    return aMessage;
}

//---------------------------------------------------------------------------
QString PrintingService::loadReceipt(qint64 aPaymentId) {
    // Получаем имя папки с чеками.
    QString suffix = QDate::currentDate().toString("yyyy.MM.dd");

    QDir path(IApplication::getWorkingDirectory() + "/receipts/" + suffix);

    if (!path.exists()) {
        toLog(LogLevel::Error, "Failed to find printed receipts folder.");
        return {};
    }

    QDir dir(path.path() + QDir::separator());
    QStringList receiptFiles;
    receiptFiles << QString("*_%1.txt").arg(aPaymentId) << QString("*_%1_*.txt").arg(aPaymentId);

    QStringList receiptsBody;

    QStringList receipts = dir.entryList(receiptFiles);
    while (!receipts.isEmpty()) {
        QFile f(dir.absolutePath() + QDir::separator() + receipts.takeFirst());
        if (f.open(QIODevice::ReadOnly)) {
            receiptsBody << replaceTags(QString::fromUtf8(f.readAll()));
        }
    }

    return receiptsBody.join("\n------------------------------\n");
}

//---------------------------------------------------------------------------
void PrintingService::onOFDNotSent(bool aExist) {
    auto *service = SettingsService::instance(m_Application);
    auto *adapter = service ? service->getAdapter<PPSDK::TerminalSettings>() : nullptr;
    bool block =
        adapter ? adapter->getCommonSettings().blockOn(PPSDK::SCommonSettings::PrinterError) : true;

    QVariantMap configuration;
    configuration.insert(CHardwareSDK::Printer::OFDNotSentError, aExist);
    configuration.insert(CHardwareSDK::Printer::BlockTerminalOnError, block);

    foreach (auto printer, m_PrinterDevices) {
        if (printer) {
            printer->setDeviceConfiguration(configuration);
        }
    }
}

//---------------------------------------------------------------------------
void PrintingService::onStatusChanged(DSDK::EWarningLevel::Enum aWarningLevel,
                                      const QString & /*aTranslation*/,
                                      int /*aStatus*/) {
    emit printerStatus(aWarningLevel == DSDK::EWarningLevel::OK);
}

//---------------------------------------------------------------------------
void PrintingService::onFRSessionClosed(const QVariantMap &aParameters) {
    // Получаем имя папки с отчётами.
    QDir path(IApplication::getWorkingDirectory() + "/receipts/reports");

    if (!path.exists()) {
        if (!QDir().mkpath(path.path())) {
            toLog(LogLevel::Error, "Failed to create printed reports folder.");
            return;
        }
    }

    auto fileName = path.path() + QDir::separator() +
                    QDateTime::currentDateTime().toString("yyyy.MM.dd hhmmsszzz") + ".xml";
    QFile file(fileName);

    // Сохраняем отчёт.
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    using namespace SDK::Driver;

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement(CFRReport::ZReport);
    stream.writeAttribute(CFRReport::Number, aParameters[CFiscalPrinter::ZReportNumber].toString());

    stream.writeStartElement(CFRReport::FR);
    stream.writeAttribute(CFRReport::Serial, aParameters[CFiscalPrinter::Serial].toString());
    stream.writeAttribute(CFRReport::RNM, aParameters[CFiscalPrinter::RNM].toString());
    stream.writeEndElement(); // CFRReport::FR

    stream.writeTextElement(CFRReport::PaymentCount,
                            aParameters[CFiscalPrinter::PaymentCount].toString());
    double amount = aParameters[CFiscalPrinter::PaymentAmount].toDouble();
    stream.writeTextElement(CFRReport::PaymentAmount, QString::number(amount, 'f', 2));
    amount = aParameters[CFiscalPrinter::NonNullableAmount].toDouble();
    stream.writeTextElement(CFRReport::NonNullableAmount, QString::number(amount, 'f', 2));
    stream.writeTextElement(CFRReport::FRDateTime,
                            aParameters[CFiscalPrinter::FRDateTime].toString());
    stream.writeTextElement(CFRReport::System_DateTime,
                            aParameters[CFiscalPrinter::System_DateTime].toString());

    stream.writeEndElement(); // CFRReport::ZReport
    stream.writeEndDocument();

    file.flush();
    file.close();
}

//---------------------------------------------------------------------------
void PrintingService::updateHardwareConfiguration() {
    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();

    // Получаем информацию о принтерах из конфигов.
    QString regExpData = QString("(%1|%2|%3)")
                             .arg(DSDK::CComponents::Printer)
                             .arg(DSDK::CComponents::DocumentPrinter)
                             .arg(DSDK::CComponents::FiscalRegistrator);
    QStringList printerNames = settings->getDeviceList().filter(QRegularExpression(regExpData));

    auto commonSettings = settings->getCommonSettings();

    m_Random_ReceiptsID = commonSettings.random_ReceiptsID;
    QTime autoZReportTime = commonSettings.autoZReportTime;
    m_EnableBlankFiscalData = commonSettings.enableBlankFiscalData;

    m_PrinterDevices.clear();
    m_AvailablePrinters.clear();

    // Запрашиваем устройства.
    foreach (const QString &printerName, printerNames) {
        auto *device = dynamic_cast<DSDK::IPrinter *>(m_DeviceService->acquireDevice(printerName));

        if (!device) {
            toLog(LogLevel::Error, QString("Failed to acquire device %1 .").arg(printerName));
            continue;
        }

        QVariantMap dealerSettings;
        if (m_StaticParameters.contains(CPrintConstants::DealerTaxSystem)) {
            dealerSettings.insert(CHardwareSDK::FR::DealerTaxSystem,
                                  m_StaticParameters[CPrintConstants::DealerTaxSystem]);
        }
        if (m_StaticParameters.contains(CPrintConstants::DealerAgentFlag)) {
            dealerSettings.insert(CHardwareSDK::FR::DealerAgentFlag,
                                  m_StaticParameters[CPrintConstants::DealerAgentFlag]);
        }
        if (m_StaticParameters.contains(CPrintConstants::DealerVAT)) {
            dealerSettings.insert(CHardwareSDK::FR::DealerVAT,
                                  m_StaticParameters[CPrintConstants::DealerVAT]);
        }
        if (m_StaticParameters.contains(CPrintConstants::DealerSupportPhone)) {
            dealerSettings.insert(CHardwareSDK::FR::DealerSupportPhone,
                                  m_StaticParameters[CPrintConstants::DealerSupportPhone]);
        }

        m_PrinterDevices.append(device);

        // Подписываемся на события принтера.
        device->subscribe(
            SDK::Driver::IDevice::StatusSignal,
            this,
            SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

        // Подписываемся на события фискальника.
        if (dynamic_cast<DSDK::IFiscalPrinter *>(device)) {
            device->subscribe(SDK::Driver::IFiscalPrinter::FRSessionClosedSignal,
                              this,
                              SLOT(onFRSessionClosed(const QVariantMap &)));

            if (autoZReportTime.isValid() && !autoZReportTime.isNull()) {
                toLog(LogLevel::Normal,
                      QString("Setup auto z-report time: %1.")
                          .arg(autoZReportTime.toString("hh:mm:ss")));

                dealerSettings.insert(CHardwareSDK::FR::ZReportTime, autoZReportTime);
            }
        }

        device->setDeviceConfiguration(dealerSettings);
    }

    m_AvailablePrinters =
        QSet<SDK::Driver::IPrinter *>(m_PrinterDevices.begin(), m_PrinterDevices.end());
}

//---------------------------------------------------------------------------
void PrintingService::createFiscalRegister() {
    if (m_FiscalRegister) {
        return;
    }

    // Получаем информацию о фискальных регистраторах.
    auto *extSettings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::ExtensionsSettings>();
    SDK::Plugin::IPluginLoader *pluginLoader =
        PluginService::instance(m_Application)->getPluginLoader();
    QStringList frPlugins =
        pluginLoader->getPluginList(QRegularExpression(PPSDK::CComponents::FiscalRegister));

    foreach (auto fr, frPlugins) {
        auto *plugin = pluginLoader->createPlugin(fr);

        if (!plugin) {
            continue;
        }

        auto *frPlugin = dynamic_cast<PPSDK::IFiscalRegister *>(plugin);

        if (!frPlugin) {
            toLog(LogLevel::Error, QString("FR %1 not have IFiscalRegister interface.").arg(fr));
            pluginLoader->destroyPlugin(plugin);
            continue;
        }

        auto parameters = extSettings->getSettings(plugin->getPluginName());

        if (parameters.isEmpty()) {
            toLog(LogLevel::Warning,
                  QString("FR %1 not have extensions settings. Skip it. (check config.xml).")
                      .arg(plugin->getPluginName()));
            pluginLoader->destroyPlugin(plugin);
            continue;
        }

        frPlugin->subscribe(
            PPSDK::IFiscalRegister::OFDNotSentSignal, this, SLOT(onOFDNotSent(bool)));

        if (!frPlugin->initialize(parameters)) {
            frPlugin->unsubscribe(PPSDK::IFiscalRegister::OFDNotSentSignal, this);

            toLog(LogLevel::Warning,
                  QString("FR %1 error initialize. Skip it.").arg(plugin->getPluginName()));
            pluginLoader->destroyPlugin(plugin);
            continue;
        }

        m_FiscalRegister = frPlugin;

        toLog(LogLevel::Normal, QString("FR %1 loaded successful.").arg(plugin->getPluginName()));

        break;
    }
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IFiscalRegister *PrintingService::getFiscalRegister() const {
    return m_FiscalRegister;
}

//---------------------------------------------------------------------------
void PrintingService::setFiscalNumber(qint64 aPaymentId, const QVariantMap &aParameters) {
    QList<PPSDK::IPayment::SParameter> parameters;

    foreach (auto name, aParameters.keys()) {
        parameters.push_back(PPSDK::IPayment::SParameter(name, aParameters.value(name), true));
    }

    if (!PaymentService::instance(m_Application)->updatePaymentFields(aPaymentId, parameters)) {
        toLog(LogLevel::Error,
              QString("Payment %1: Error update fiscal parameters.").arg(aPaymentId));
    }
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::SCurrencySettings PrintingService::getCurrencySettings() const {
    SettingsService *settingsService = SettingsService::instance(m_Application);

    auto *terminalSettings = settingsService->getAdapter<PPSDK::TerminalSettings>();

    return terminalSettings->getCurrencySettings();
}

//---------------------------------------------------------------------------
