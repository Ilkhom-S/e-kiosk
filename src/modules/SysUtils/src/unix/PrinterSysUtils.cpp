/* @file Реализация интерфейса SysUtils для принтеров (Linux/macOS).
 */

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <QtCore/QVariantMap>

#include <SysUtils/ISysUtils.h>

#include "SystemPrinterStatusCodes.h"

//---------------------------------------------------------------------------
QVariantMap ISysUtils::getPrinterData(const QString &aPrinterName) {
#if defined(__linux__) || defined(__APPLE__)
    QVariantMap result;
    QString cmd = QString("lpstat -l -p %1").arg(aPrinterName);
    FILE *fp = popen(cmd.toUtf8().constData(), "r");
    if (!fp) {
        return result;
    }
    char buffer[512];
    QString output;
    while (fgets(buffer, sizeof(buffer), fp)) {
        output += QString::fromLocal8Bit(buffer);
    }
    pclose(fp);
    if (output.contains("printer ")) {
        result["exists"] = true;
    }
    if (output.contains("enabled")) {
        result["enabled"] = true;
    }
    if (output.contains("disabled")) {
        result["enabled"] = false;
    }
    if (output.contains("idle")) {
        result["state"] = "idle";
    }
    if (output.contains("printing")) {
        result["state"] = "printing";
    }
    result["raw"] = output;
    return result;
#else
    Q_UNUSED(aPrinterName);
    return QVariantMap();
#endif
}

//---------------------------------------------------------------------------
void ISysUtils::getPrinterStatus(const QString &aPrinterName,
                                 TStatusCodes &aStatusCodes,
                                 TStatusGroupNames &aGroupNames) {
#if defined(__linux__) || defined(__APPLE__)
    aStatusCodes.clear();
    aGroupNames.clear();
    QString cmd = QString("lpstat -p %1").arg(aPrinterName);
    FILE *fp = popen(cmd.toUtf8().constData(), "r");
    if (!fp) {
        return;
    }
    char buffer[512];
    QString output;
    while (fgets(buffer, sizeof(buffer), fp)) {
        output += QString::fromLocal8Bit(buffer);
    }
    pclose(fp);
    // Map CUPS status to codes (see System_PrinterStatusCodes.h)
    if (output.contains("disabled")) {
        aStatusCodes.insert(1); // 1 = disabled
    }
    if (output.contains("enabled")) {
        aStatusCodes.insert(0); // 0 = enabled
    }
    if (output.contains("idle")) {
        aStatusCodes.insert(2); // 2 = idle
    }
    if (output.contains("printing")) {
        aStatusCodes.insert(3); // 3 = printing
    }
    aGroupNames["printer"] = TStatusNames() << aPrinterName;
#else
    Q_UNUSED(aPrinterName);
    Q_UNUSED(aStatusCodes);
    Q_UNUSED(aGroupNames);
#endif
}

//---------------------------------------------------------------------------
bool ISysUtils::setPrintingQueuedMode(const QString &aPrinterName, QString &aErrorMessage) {
#if defined(__linux__) || defined(__APPLE__)
    QString cmd = QString("lpadmin -p %1 -E").arg(aPrinterName);
    int ret = system(cmd.toUtf8().constData());
    if (ret != 0) {
        aErrorMessage = QString("lpadmin failed: %1").arg(ret);
        return false;
    }
    return true;
#else
    Q_UNUSED(aPrinterName);
    Q_UNUSED(aErrorMessage);
    return false;
#endif
}
