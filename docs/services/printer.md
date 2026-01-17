# Printer Service

The Printer Service manages printing operations for the EKiosk system.

## Overview

The Printer Service (`IPrinterService`) handles:

- Receipt and document printing
- Printer status monitoring
- Print queue management
- Printer configuration and maintenance
- Multiple printer support

## Interface

```cpp
class IPrinterService : public QObject {
    Q_OBJECT

public:
    enum PrinterStatus { Ready, Busy, OutOfPaper, Error, Offline, Unknown };

    /// Get available printers
    virtual QStringList getAvailablePrinters() const = 0;

    /// Get printer status
    virtual PrinterStatus getPrinterStatus(const QString &printerName = QString()) const = 0;

    /// Print receipt
    virtual bool printReceipt(const QString &receiptData,
                            const QString &printerName = QString()) = 0;

    /// Print document
    virtual bool printDocument(const QString &documentData,
                             const QString &printerName = QString()) = 0;

    /// Print test page
    virtual bool printTestPage(const QString &printerName = QString()) = 0;

    /// Cancel print job
    virtual bool cancelPrintJob(int jobId) = 0;

    /// Get print queue status
    virtual QList<QVariantMap> getPrintQueue() const = 0;

    /// Set default printer
    virtual bool setDefaultPrinter(const QString &printerName) = 0;

    // ... additional methods for printer management
};
```

## Printer Management

### Available Printers

```cpp
// Get printer service from core
auto printerService = core->getPrinterService();

if (!printerService) {
    LOG(log, LogLevel::Error, "Printer service not available");
    return;
}

// Get available printers
QStringList printers = printerService->getAvailablePrinters();

LOG(log, LogLevel::Info, QString("Available printers: %1").arg(printers.join(", ")));

// Set default printer if available
if (!printers.isEmpty()) {
    printerService->setDefaultPrinter(printers.first());
}
```

### Printer Status

```cpp
// Check printer status
IPrinterService::PrinterStatus status = printerService->getPrinterStatus();

switch (status) {
    case IPrinterService::Ready:
        LOG(log, LogLevel::Info, "Printer is ready");
        enablePrinting();
        break;
    case IPrinterService::Busy:
        LOG(log, LogLevel::Info, "Printer is busy");
        showPrinterBusyMessage();
        break;
    case IPrinterService::OutOfPaper:
        LOG(log, LogLevel::Error, "Printer is out of paper");
        showOutOfPaperMessage();
        break;
    case IPrinterService::Error:
        LOG(log, LogLevel::Error, "Printer error");
        handlePrinterError();
        break;
    case IPrinterService::Offline:
        LOG(log, LogLevel::Warning, "Printer is offline");
        showPrinterOfflineMessage();
        break;
    case IPrinterService::Unknown:
        LOG(log, LogLevel::Warning, "Printer status unknown");
        break;
}
```

## Printing Operations

### Printing Receipts

```cpp
// Print receipt after successful payment
bool printReceipt(const QString &transactionId, double amount, const QStringList &items) {
    // Create receipt content
    QString receiptData;
    receiptData += "================================\n";
    receiptData += "          EKiosk Receipt        \n";
    receiptData += "================================\n";
    receiptData += QString("Transaction ID: %1\n").arg(transactionId);
    receiptData += QString("Date: %1\n").arg(QDateTime::currentDateTime().toString());
    receiptData += "--------------------------------\n";

    // Add items
    for (const QString &item : items) {
        receiptData += QString("%1\n").arg(item);
    }

    receiptData += "--------------------------------\n";
    receiptData += QString("Total: $%1\n").arg(amount);
    receiptData += "================================\n";
    receiptData += "Thank you for your purchase!\n";

    // Print receipt
    bool printed = printerService->printReceipt(receiptData);

    if (printed) {
        LOG(log, LogLevel::Info, QString("Receipt printed for transaction: %1").arg(transactionId));
        return true;
    } else {
        LOG(log, LogLevel::Error, QString("Failed to print receipt for transaction: %1").arg(transactionId));
        return false;
    }
}
```

### Printing Documents

```cpp
// Print custom document
bool printReport(const QString &reportData, const QString &printerName = QString()) {
    // Add document formatting
    QString documentData;
    documentData += "EKiosk Daily Report\n";
    documentData += "===================\n\n";
    documentData += reportData;
    documentData += "\n\nGenerated: " + QDateTime::currentDateTime().toString();

    // Print document
    bool printed = printerService->printDocument(documentData, printerName);

    if (printed) {
        LOG(log, LogLevel::Info, "Report printed successfully");
        return true;
    } else {
        LOG(log, LogLevel::Error, "Failed to print report");
        return false;
    }
}
```

### Test Printing

```cpp
// Print test page to verify printer functionality
bool testPrinter(const QString &printerName = QString()) {
    LOG(log, LogLevel::Info, "Printing test page");

    bool success = printerService->printTestPage(printerName);

    if (success) {
        LOG(log, LogLevel::Info, "Test page printed successfully");
        return true;
    } else {
        LOG(log, LogLevel::Error, "Failed to print test page");
        return false;
    }
}
```

## Print Queue Management

### Checking Print Queue

```cpp
// Get current print queue
QList<QVariantMap> queue = printerService->getPrintQueue();

LOG(log, LogLevel::Info, QString("Print queue has %1 jobs").arg(queue.size()));

for (const QVariantMap &job : queue) {
    int jobId = job["id"].toInt();
    QString document = job["document"].toString();
    QString status = job["status"].toString();

    LOG(log, LogLevel::Info,
        QString("Job %1: %2 - Status: %3").arg(jobId).arg(document).arg(status));
}
```

### Cancelling Print Jobs

```cpp
// Cancel specific print job
bool cancelJob(int jobId) {
    bool cancelled = printerService->cancelPrintJob(jobId);

    if (cancelled) {
        LOG(log, LogLevel::Info, QString("Print job %1 cancelled").arg(jobId));
        return true;
    } else {
        LOG(log, LogLevel::Error, QString("Failed to cancel print job %1").arg(jobId));
        return false;
    }
}
```

## Usage in Plugins

Printer Service is commonly used in receipt and reporting plugins:

```cpp
class ReceiptPrinterPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mPrinterService = mCore->getPrinterService();
            mPaymentService = mCore->getPaymentService();
            mEventService = mCore->getEventService();
            mLog = kernel->getLog("ReceiptPrinter");
        }

        return true;
    }

    void onPaymentCompleted(const QVariantMap &paymentData) {
        QString transactionId = paymentData["transactionId"].toString();
        double amount = paymentData["amount"].toDouble();

        // Generate and print receipt
        bool printed = printReceipt(transactionId, amount, paymentData);

        if (printed) {
            LOG(mLog, LogLevel::Info, QString("Receipt printed for: %1").arg(transactionId));

            // Publish receipt printed event
            QVariantMap eventData;
            eventData["transactionId"] = transactionId;
            eventData["printed"] = true;
            mEventService->publish("receipt.printed", eventData);
        } else {
            LOG(mLog, LogLevel::Error, QString("Failed to print receipt for: %1").arg(transactionId));

            // Handle print failure
            handlePrintFailure(transactionId);
        }
    }

    bool printReceipt(const QString &transactionId, double amount, const QVariantMap &paymentData) {
        if (!mPrinterService) {
            LOG(mLog, LogLevel::Error, "Printer service not available");
            return false;
        }

        // Check printer status
        IPrinterService::PrinterStatus status = mPrinterService->getPrinterStatus();

        if (status != IPrinterService::Ready) {
            LOG(mLog, LogLevel::Error,
                QString("Printer not ready. Status: %1").arg(getStatusString(status)));
            return false;
        }

        // Create receipt content
        QString receiptData = generateReceiptContent(transactionId, amount, paymentData);

        // Print receipt
        return mPrinterService->printReceipt(receiptData);
    }

    bool printDailyReport() {
        // Generate daily sales report
        QString reportData = generateDailyReport();

        if (reportData.isEmpty()) {
            LOG(mLog, LogLevel::Warning, "No sales data for daily report");
            return false;
        }

        // Print report
        bool printed = mPrinterService->printDocument(reportData, "report_printer");

        if (printed) {
            LOG(mLog, LogLevel::Info, "Daily report printed successfully");
        } else {
            LOG(mLog, LogLevel::Error, "Failed to print daily report");
        }

        return printed;
    }

private:
    QString generateReceiptContent(const QString &transactionId, double amount,
                                 const QVariantMap &paymentData) {
        QString content;
        content += "================================\n";
        content += "          EKiosk Receipt        \n";
        content += "================================\n";
        content += QString("Transaction: %1\n").arg(transactionId);
        content += QString("Date: %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        content += QString("Amount: $%1\n").arg(amount, 0, 'f', 2);
        content += QString("Method: %1\n").arg(paymentData["method"].toString());
        content += "================================\n";
        content += "Thank you for your purchase!\n";

        return content;
    }

    QString generateDailyReport() {
        // Query database for daily sales
        // Return formatted report data
        return "Daily Sales Report\n==================\nTotal Sales: $1250.50\nTransactions: 45\n";
    }

    void handlePrintFailure(const QString &transactionId) {
        // Store receipt data for later printing
        // Send notification to operator
        // Offer digital receipt option

        QVariantMap failureData;
        failureData["transactionId"] = transactionId;
        failureData["reason"] = "printer_error";
        mEventService->publish("receipt.print_failed", failureData);
    }

    QString getStatusString(IPrinterService::PrinterStatus status) {
        switch (status) {
            case IPrinterService::Ready: return "Ready";
            case IPrinterService::Busy: return "Busy";
            case IPrinterService::OutOfPaper: return "Out of Paper";
            case IPrinterService::Error: return "Error";
            case IPrinterService::Offline: return "Offline";
            default: return "Unknown";
        }
    }

private:
    IPrinterService *mPrinterService;
    IPaymentService *mPaymentService;
    IEventService *mEventService;
    ILog *mLog;
};
```

## Error Handling

```cpp
try {
    // Check printer availability
    QStringList printers = printerService->getAvailablePrinters();

    if (printers.isEmpty()) {
        throw std::runtime_error("No printers available");
    }

    // Check printer status
    IPrinterService::PrinterStatus status = printerService->getPrinterStatus();

    if (status != IPrinterService::Ready) {
        throw std::runtime_error(QString("Printer not ready: %1").arg(getStatusString(status)));
    }

    // Attempt printing
    if (!printerService->printReceipt(receiptData)) {
        throw std::runtime_error("Print operation failed");
    }

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Printer service error: %1").arg(e.what()));

    // Handle error - show user message, queue for later, etc.
    handlePrinterError(e.what());

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected printer error: %1").arg(e.what()));
}
```

## Printer Configuration

```cpp
// Configure printers through settings
void configurePrinters() {
    auto settings = core->getSettingsService();

    // Configure receipt printer
    QString receiptPrinter = settings->getValue("Printer/ReceiptPrinter", "default").toString();
    printerService->setDefaultPrinter(receiptPrinter);

    // Configure paper size
    QString paperSize = settings->getValue("Printer/PaperSize", "80mm").toString();

    // Configure print quality
    int dpi = settings->getValue("Printer/DPI", 203).toInt();

    // Configure auto-cut
    bool autoCut = settings->getValue("Printer/AutoCut", true).toBool();
}
```

## Maintenance Operations

```cpp
// Perform printer maintenance
void performPrinterMaintenance() {
    // Print test page
    printerService->printTestPage();

    // Check paper levels
    checkPaperLevels();

    // Clean print head (if supported)
    cleanPrintHead();

    // Update printer firmware (if supported)
    updatePrinterFirmware();
}

void checkPaperLevels() {
    // Query printer for paper status
    IPrinterService::PrinterStatus status = printerService->getPrinterStatus();

    if (status == IPrinterService::OutOfPaper) {
        LOG(log, LogLevel::Warning, "Printer is out of paper");

        // Notify operator
        QVariantMap alertData;
        alertData["type"] = "paper_out";
        alertData["printer"] = "receipt";
        eventService->publish("printer.alert", alertData);
    }
}
```

## Security Considerations

- Validate print data to prevent malicious content
- Implement print job auditing
- Secure printer communications
- Consider print data encryption for sensitive receipts

## Dependencies

- Device Service (for printer hardware management)
- Settings Service (for printer configuration)
- Event Service (for print status notifications)
- Database Service (for print job logging)

## See Also

- [Device Service](device.md) - Printer hardware management
- [Settings Service](settings.md) - Printer configuration
- [Event Service](event.md) - Print status notifications
- [Payment Service](payment.md) - Receipt printing integration
