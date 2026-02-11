#pragma once

#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QTextBrowser>

#include "dev/AV268.h"
#include "dev/CitizenCBM1000.h"
#include "dev/CitizenCTS2000.h"
#include "dev/CitizenPPU700.h"
#include "dev/CustomTG2480.h"
#include "dev/CustomVKP80.h"
#include "dev/KM1X.h"
#include "dev/Phoenix.h"

class Custom_VKP80_PRINTER;
class AV268_PRINTER;
class CitizenPPU700_PRINTER;
class CitizenCTS2000_PRINTER;
class CitizenCBM1000_PRINTER;
class Phoenix_PRINTER;

namespace PrinterModel {
const QString Custom_VKP80 = "Custom_VKP80";
const QString Star_TUP900 = "StarTUP900";
const QString AV_268 = "AV268";
const QString Citizen_CTS2000 = "CitizenCTS2000";
const QString Custom_TG2480 = "Custom_TG2480";
const QString Citizen_PPU700 = "CitizenPPU700";
const QString Windows_Printer = "WindowsPrinter";
const QString Phoenix_model = "Phoenix PHX-POG";
const QString CitizenCBM1000 = "CitizenCBM1000";
const QString KM1X = "KM1X";
} // namespace PrinterModel

namespace PrinterCommand {
const QString cmdPrint = "print";
const QString cmdGetStatus = "get_status";
const QString cmdInit = "init";
const QString cmdFinedPrinter = "fined_printer";
const QString cmdIsItYou = "is_it_you";
} // namespace PrinterCommand

class ClassPrinter : public QThread {
    Q_OBJECT

public:
    ClassPrinter(QObject *parent = 0);

    void setPrinterModel(const QString printerModel);
    void setChekWidth(const int width);
    void setFirm_Patern(const QString firmName);
    void setSmallText(bool smallI);
    void setCom_List(const QStringList list);
    void setPortName(const QString portName);
    void setCounterPrinterIndicator(bool stsIndicate);
    bool printerOpen();
    void closeThis();
    void setLeftMargin(int leftMargin);
    void setSmallBeetwenString(bool beet);
    bool isItYou(QStringList &comList,
                 QString &printerName,
                 QString &comStr,
                 QString &printerComent);
    void tarminateThis();
    void clearListPrinterData(QString name);

    int WpWidth;
    int WpHeight;
    int WpFont;
    int WpLeftMargin;
    int WpTopMargin;
    int WpRightMargin;
    int WpBottom_Margin;

    QString winPrinterName;
    QString nowPrinterName;
    QString nowPortName;
    QString nowComent;
    int status{};

    QString portSpeed;

    QString textToPrint;

public slots:

    void CMD_Print(QString text);
    void CMD_GetStatus();
    void CMD_IsItYou();
    void CMD_FinedPrinters();
    void CPrint();

    void CGetStatus();

signals:
    void emit_status(int status);

private:
    Custom_VKP80_PRINTER *Custom_VKP80{};
    AV268_PRINTER *AV268{};
    CitizenPPU700_PRINTER *CitizenPPU700{};
    CitizenCTS2000_PRINTER *CitizenCTS2000{};
    Phoenix_PRINTER *Phoenix{};
    TG2480_PRINTER *Custom_TG2480{};
    CitizenCBM1000_PRINTER *CitizenCBM1000{};
    KM1X_PRINTER *KM1X{};

    QString system_Model;
    int checkWidth{};
    QString firm_Name;
    QStringList com_List;
    QString portName;
    QString cmd_now;
    int leftMargin{};
    bool smallBeetwenStr{};
    QTextBrowser *textBrowser{};

    virtual void run();

    void CIsItYou();
    bool CIsItYou(QString &comment);
    void winPrint(QString text);
};
