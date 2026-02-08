#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#include <QtCore/QDebug>

#include "ClassPrinter.h"

QStringList Printer_List;

ClassPrinter::ClassPrinter(QObject *parent) : QThread(parent) {
    Printer_List << PrinterModel::Custom_VKP80 << PrinterModel::CitizenCBM1000
                 << PrinterModel::Citizen_CTS2000 << PrinterModel::Custom_TG2480
                 << PrinterModel::Citizen_PPU700 << PrinterModel::AV_268
                 << PrinterModel::Phoenix_model;

    WpWidth = 90;
    WpHeight = 150;
    WpFont = 8;
    WpLeftMargin = 2;
    WpTopMargin = 1;
    WpRightMargin = 1;
    WpBottom_Margin = 1;
}

bool ClassPrinter::isItYou(QStringList &com_List,
                           QString &printer_name,
                           QString &com_str,
                           QString &printer_coment) {
    if ((printer_name != "") && (com_str != "") && (com_str.contains("COM"))) {
        this->setPrinterModel(printer_name);
        this->setPortName(com_str);

        if (this->printerOpen()) {
            if (this->CIsItYou(printer_coment)) {
                nowPrinterName = printer_name;
                nowPortName = com_str;
                nowComent = printer_coment;
                return true;
            }
        } else {
            return false;
        }
    }
    // qDebug() << "validator_name << " << printer_name;
    // qDebug() << "com_str << " << com_str;
    // qDebug() << "Printer_List count << " << Printer_List.count();

    for (int dev_count = 0; dev_count < Printer_List.count(); dev_count++) {
        // qDebug() << "dev_count << " << dev_count;

        this->setPrinterModel(Printer_List.at(dev_count));

        // qDebug() << "printerName << " << Printer_List.at(dev_count);

        for (int com_count = 0; com_count < com_List.count(); com_count++) {
            this->setPortName(com_List.at(com_count));
            // qDebug() << "com_Port << " << com_List.at(com_count);
            if (this->printerOpen()) {
                if (this->CIsItYou(printer_coment)) {
                    nowPrinterName = printer_name = Printer_List.at(dev_count);
                    nowPortName = com_str = com_List.at(com_count);
                    nowComent = printer_coment;
                    return true;
                }
            } else {
                return false;
            }
        }
    }
    return false;
}

void ClassPrinter::setCounterPrinterIndicator(bool sts_indicate) {
    if (system_Model == PrinterModel::Custom_VKP80)
        Custom_VKP80->counterIndicate = sts_indicate;
    if (system_Model == PrinterModel::Citizen_PPU700)
        CitizenPPU700->counterIndicate = sts_indicate;
    if (system_Model == PrinterModel::Citizen_CTS2000)
        CitizenCTS2000->counterIndicate = sts_indicate;
}

void ClassPrinter::setPrinterModel(const QString printerModel) {
    // Тут дикларируем принтера
    // qDebug() << "printerModel - " << printerModel;
    system_Model = printerModel;

    if (system_Model == PrinterModel::Custom_VKP80)
        Custom_VKP80 = new Custom_VKP80_PRINTER();
    if (system_Model == PrinterModel::Custom_TG2480)
        Custom_TG2480 = new TG2480_PRINTER();
    if (system_Model == PrinterModel::AV_268)
        AV268 = new AV268_PRINTER();
    if (system_Model == PrinterModel::CitizenCBM1000)
        CitizenCBM1000 = new CitizenCBM1000_PRINTER();
    if (system_Model == PrinterModel::Citizen_PPU700)
        CitizenPPU700 = new CitizenPPU700_PRINTER();
    if (system_Model == PrinterModel::Citizen_CTS2000)
        CitizenCTS2000 = new CitizenCTS2000_PRINTER();
    if (system_Model == PrinterModel::Phoenix_model)
        Phoenix = new Phoenix_PRINTER();
    if (system_Model == PrinterModel::KM1X)
        KM1X = new KM1X_PRINTER();

    if (system_Model == PrinterModel::Windows_Printer)
        textBrowser = new QTextBrowser();
}

void ClassPrinter::setChekWidth(const int width) {
    checkWidth = width;
    if (system_Model == PrinterModel::Custom_VKP80)
        Custom_VKP80->setCheckWidth(width);
}

void ClassPrinter::setFirm_Patern(const QString firm_name) {
    firm_Name = firm_name;
    if (system_Model == PrinterModel::Custom_VKP80)
        Custom_VKP80->setFirm_Pattern(firm_Name);
    if (system_Model == PrinterModel::Citizen_PPU700)
        CitizenPPU700->setFirm_Pattern(firm_Name);
    if (system_Model == PrinterModel::Citizen_CTS2000)
        CitizenCTS2000->setFirm_Pattern(firm_Name);
}

void ClassPrinter::setSmallBeetwenString(bool beet) {
    smallBeetwenStr = beet;

    if (system_Model == PrinterModel::Custom_VKP80)
        Custom_VKP80->setSmallBetweenString(beet);
    if (system_Model == PrinterModel::Citizen_PPU700)
        CitizenPPU700->setSmallBetweenString(beet);
    if (system_Model == PrinterModel::Citizen_CTS2000)
        CitizenCTS2000->setSmallBetweenString(beet);
}

void ClassPrinter::setSmallText(bool small_i) {
    if (system_Model == PrinterModel::Custom_VKP80)
        Custom_VKP80->setCheckSmall(small_i);
    if (system_Model == PrinterModel::Citizen_PPU700)
        CitizenPPU700->setCheckSmall(small_i);
    if (system_Model == PrinterModel::Citizen_CTS2000)
        CitizenCTS2000->setCheckSmall(small_i);
}

void ClassPrinter::setCom_List(const QStringList list) {
    com_List = list;
}

void ClassPrinter::setLeftMargin(int left_margin) {
    leftMargin = left_margin;
    if (system_Model == PrinterModel::Custom_VKP80)
        Custom_VKP80->setLeftMargin(leftMargin);
    if (system_Model == PrinterModel::Custom_TG2480)
        Custom_TG2480->setLeftMargin(leftMargin);
    if (system_Model == PrinterModel::AV_268)
        AV268->setLeftMargin(leftMargin);
    if (system_Model == PrinterModel::CitizenCBM1000)
        CitizenCBM1000->setLeftMargin(leftMargin);
    if (system_Model == PrinterModel::Citizen_PPU700)
        CitizenPPU700->setLeftMargin(leftMargin);
    if (system_Model == PrinterModel::Citizen_CTS2000)
        CitizenCTS2000->setLeftMargin(leftMargin);
    if (system_Model == PrinterModel::Phoenix_model)
        Phoenix->setLeftMargin(leftMargin);
}

void ClassPrinter::setPortName(const QString port_name) {
    portName = port_name;
}

void ClassPrinter::CMD_Print(QString text) {
    cmd_now = PrinterCommand::cmdPrint;
    textToPrint = text;
    this->start();
}

void ClassPrinter::CMD_GetStatus() {
    cmd_now = PrinterCommand::cmdGetStatus;
    this->start();
}

void ClassPrinter::CMD_FinedPrinters() {
    cmd_now = PrinterCommand::cmdFinedPrinter;
    this->start();
}

void ClassPrinter::CMD_IsItYou() {
    cmd_now = PrinterCommand::cmdIsItYou;
    this->start();
}

void ClassPrinter::run() {
    if (cmd_now == PrinterCommand::cmdGetStatus) {
        this->CGetStatus();
        // qDebug() << "AFTER GET STATUS";
    }

    if (cmd_now == PrinterCommand::cmdPrint) {
        this->CPrint();
    }
    if (cmd_now == PrinterCommand::cmdIsItYou) {
        this->CIsItYou();
    }
    return;
}

void ClassPrinter::tarminateThis() {
    this->terminate();
}

void ClassPrinter::CIsItYou() {
    if (system_Model == PrinterModel::Custom_VKP80) {
        if (Custom_VKP80->isItYou())
            qDebug() << QString("ClassPrinter -- Availabled");
        else
            qDebug() << QString("ClassPrinter -- NotAvailabled");
    }

    if (system_Model == PrinterModel::Custom_TG2480) {
        if (Custom_TG2480->isItYou())
            qDebug() << QString("ClassPrinter -- Availabled");
        else
            qDebug() << QString("ClassPrinter -- NotAvailabled");
    }

    if (system_Model == PrinterModel::AV_268) {
        if (AV268->isItYou())
            qDebug() << QString("ClassPrinter -- Availabled");
        else
            qDebug() << QString("ClassPrinter -- NotAvailabled");
    }

    if (system_Model == PrinterModel::CitizenCBM1000) {
        if (CitizenCBM1000->isItYou())
            qDebug() << QString("ClassPrinter -- Availabled");
        else
            qDebug() << QString("ClassPrinter -- NotAvailabled");
    }

    if (system_Model == PrinterModel::Citizen_PPU700) {
        if (CitizenPPU700->isItYou())
            qDebug() << QString("ClassPrinter -- Availabled");
        else
            qDebug() << QString("ClassPrinter -- NotAvailabled");
    }

    if (system_Model == PrinterModel::Citizen_CTS2000) {
        if (CitizenCTS2000->isItYou())
            qDebug() << QString("ClassPrinter -- Availabled");
        else
            qDebug() << QString("ClassPrinter -- NotAvailabled");
    }

    if (system_Model == PrinterModel::Phoenix_model) {
        if (Phoenix->isItYou())
            qDebug() << QString("ClassPrinter -- Availabled");
        else
            qDebug() << QString("ClassPrinter -- NotAvailabled");
    }
}

bool ClassPrinter::CIsItYou(QString &comment) {
    if (system_Model == PrinterModel::Custom_VKP80) {
        if (Custom_VKP80->isItYou()) {
            qDebug() << QString("ClassPrinter -- Availabled");
            if (Custom_VKP80->comment != "")
                comment = "ROM v. " + Custom_VKP80->comment;
            return true;
        } else {
            qDebug() << QString("ClassPrinter -- NotAvailabled");
            return false;
        }
    }

    if (system_Model == PrinterModel::Custom_TG2480) {
        if (Custom_TG2480->isItYou()) {
            qDebug() << QString("ClassPrinter -- Availabled");
            return true;
        } else {
            qDebug() << QString("ClassPrinter -- NotAvailabled");
            return false;
        }
    }

    if (system_Model == PrinterModel::AV_268) {
        if (AV268->isItYou()) {
            qDebug() << QString("ClassPrinter -- Availabled");
            return true;
        } else {
            qDebug() << QString("ClassPrinter -- NotAvailabled");
            return false;
        }
    }
    if (system_Model == PrinterModel::CitizenCBM1000) {
        if (CitizenCBM1000->isItYou()) {
            qDebug() << QString("ClassPrinter -- Availabled");
            return true;
        } else {
            qDebug() << QString("ClassPrinter -- NotAvailabled");
            return false;
        }
    }
    if (system_Model == PrinterModel::Citizen_PPU700) {
        if (CitizenPPU700->isItYou()) {
            qDebug() << QString("ClassPrinter -- Availabled");
            return true;
        } else {
            qDebug() << QString("ClassPrinter -- NotAvailabled");
            return false;
        }
    }
    if (system_Model == PrinterModel::Citizen_CTS2000) {
        if (CitizenCTS2000->isItYou()) {
            qDebug() << QString("ClassPrinter -- Availabled");
            return true;
        } else {
            qDebug() << QString("ClassPrinter -- NotAvailabled");
            return false;
        }
    }
    if (system_Model == PrinterModel::Phoenix_model) {
        if (Phoenix->isItYou()) {
            qDebug() << QString("ClassPrinter -- Availabled");
            return true;
        } else {
            qDebug() << QString("ClassPrinter -- NotAvailabled");
            return false;
        }
    }

    return false;
}

void ClassPrinter::CPrint() {
    if (system_Model == PrinterModel::Custom_VKP80) {
        Custom_VKP80->print(textToPrint);
    }

    if (system_Model == PrinterModel::Custom_TG2480) {
        Custom_TG2480->print(textToPrint);
    }

    if (system_Model == PrinterModel::AV_268) {
        AV268->print(textToPrint);
    }

    if (system_Model == PrinterModel::CitizenCBM1000) {
        CitizenCBM1000->print(textToPrint);
    }

    if (system_Model == PrinterModel::Citizen_PPU700) {
        CitizenPPU700->print(textToPrint);
    }

    if (system_Model == PrinterModel::Citizen_CTS2000) {
        CitizenCTS2000->print(textToPrint);
    }

    if (system_Model == PrinterModel::Windows_Printer) {
        this->winPrint(textToPrint);
    }

    if (system_Model == PrinterModel::Phoenix_model) {
        Phoenix->print(textToPrint);
    }

    if (system_Model == PrinterModel::KM1X) {
        textToPrint = textToPrint.replace("[p]", "")
                          .replace("[b]", "")
                          .replace("[/b]", "")
                          .replace("[u]", "")
                          .replace("[/u]", "");
        KM1X->print(textToPrint);
    }
}

void ClassPrinter::winPrint(QString text) {
    if (this->winPrinterName != "") {
        text.replace("\n", "<br>");
        text.replace("[p]", "");
        text.replace("[b]", "<b>");
        text.replace("[/b]", "</b>");
        text.replace("[u]", "<u>");
        text.replace("[/u]", "</u>");
        text.replace("[i]", "<i>");
        text.replace("[/i]", "</i>");

#ifndef QT_NO_PRINTER

        QPrinter printer(QPrinter::HighResolution);

        qreal width = WpWidth;
        qreal height = WpHeight;
        qreal left = WpLeftMargin;
        qreal top = WpTopMargin;
        qreal right = WpRightMargin;
        qreal bottom = WpBottom_Margin;

        printer.setPrinterName(winPrinterName);

        // Устанавливаем параметры
        printer.setPageSize(QPageSize(QSizeF(width, height), QPageSize::Millimeter));
        printer.setPageMargins(QMarginsF(left, top, right, bottom), QPageLayout::Millimeter);

        QFont font("Tahoma", this->WpFont, QFont::Normal);

        textBrowser->setFont(font);
        textBrowser->setHtml(text);

        if (printer.printerName() != "")
            textBrowser->print(&printer);

#endif
    }
}

void ClassPrinter::CGetStatus() {
    // qDebug() << "START TO GET STATUS";
    // Статус принтера глобал
    int i_status = PrinterState::PrinterNotAvailable;

    if (system_Model == PrinterModel::Custom_VKP80) {
        CMDCustom_VKP80::SStatus s_status;

        bool getSts = Custom_VKP80->isEnabled(s_status, i_status);
        if (getSts) {
            // qDebug() << QString("ClassPrinter -- get status true --
            // %1").arg(system_Model);
            if (s_status.NotAvailabled) {
                // qDebug() << QString("ClassPrinter -- NotAvailabled");
            }
            if (s_status.CoverOpen) {
                // qDebug() << QString("ClassPrinter -- CoverOpen");
            }
            if (s_status.PaperOut) {
                // qDebug() << QString("ClassPrinter -- PaperOut");
            }
            if (s_status.Offline) {
                // qDebug() << QString("ClassPrinter -- Offline");
            }
            if (s_status.Error) {
                // qDebug() << QString("ClassPrinter -- Error");
            }
            if (s_status.Paper.End) {
                // qDebug() << QString("ClassPrinter -- SPaper::End");
            }
            if (s_status.Paper.NearEnd) {
                // qDebug() << QString("ClassPrinter -- SPaper::NearEnd");
            }
            if (s_status.Failures.Cutter) {
                // qDebug() << QString("ClassPrinter -- SFailures::Cutter");
            }
            if (s_status.Failures.Unrecoverable) {
                // qDebug() << QString("ClassPrinter -- SFailures::Unrecoverable");
            }
            if (s_status.Failures.AutoRecovery) {
                // qDebug() << QString("ClassPrinter -- SFailures::AutoRecovery");
            }
        } else {
            // qDebug() << QString("ClassPrinter -- get status false --
            // %1").arg(system_Model);
            if (s_status.NotAvailabled) {
                // qDebug() << QString("ClassPrinter -- NotAvailabled");
            }
        }
    }

    if (system_Model == PrinterModel::Custom_TG2480) {
        i_status = PrinterState::PrinterOK;

        bool getSts = Custom_TG2480->isEnabled(i_status);
        if (getSts) {
            // qDebug() << "getSts true";
        } else {
            // qDebug() << "getSts false";
        }
    }

    if (system_Model == PrinterModel::AV_268) {
        i_status = PrinterState::PrinterOK;

        bool getSts = AV268->isEnabled(i_status);
        if (getSts) {
            // qDebug() << "getSts true";
        } else {
            // qDebug() << "getSts false";
        }
    }

    if (system_Model == PrinterModel::CitizenCBM1000) {
        i_status = PrinterState::PrinterOK;

        bool getSts = CitizenCBM1000->isEnabled(i_status);
        if (getSts) {
            // qDebug() << "getSts true";
        } else {
            // qDebug() << "getSts false";
        }
    }

    if (system_Model == PrinterModel::Citizen_PPU700) {
        CMDCitizenPPU700::SStatus s_status;

        bool getSts = CitizenPPU700->isEnabled(s_status, i_status);
        if (getSts) {
            // qDebug() << QString("ClassPrinter -- get status true --
            // %1").arg(system_Model);
            if (s_status.NotAvailabled) {
                // qDebug() << QString("ClassPrinter -- NotAvailabled");
            }
            if (s_status.CoverOpen) {
                // qDebug() << QString("ClassPrinter -- CoverOpen");
            }
            if (s_status.PaperOut) {
                // qDebug() << QString("ClassPrinter -- PaperOut");
            }
            if (s_status.Offline) {
                // qDebug() << QString("ClassPrinter -- Offline");
            }
            if (s_status.Error) {
                // qDebug() << QString("ClassPrinter -- Error");
            }
            if (s_status.Paper.InPresenter) {
                // qDebug() << QString("ClassPrinter -- SPaper::InPresenter");
            }
            if (s_status.Paper.NearEndSensor1) {
                // qDebug() << QString("ClassPrinter -- SPaper::NearEndSensor1");
            }
            if (s_status.Paper.NearEndSensor2) {
                // qDebug() << QString("ClassPrinter -- SPaper::NearEndSensor2");
            }
            if (s_status.Failures.Cutter) {
                // qDebug() << QString("ClassPrinter -- SFailures::Cutter");
            }
            if (s_status.Failures.Unrecoverable) {
                // qDebug() << QString("ClassPrinter -- SFailures::Unrecoverable");
            }
            if (s_status.Failures.AutoRecovery) {
                // qDebug() << QString("ClassPrinter -- SFailures::AutoRecovery");
            }
            if (s_status.Failures.DetectionPresenter) {
                // qDebug() << QString("ClassPrinter -- SFailures::DetectionPresenter");
            }
            if (s_status.Failures.HeadOverheat) {
                // qDebug() << QString("ClassPrinter -- SFailures::HeadOverheat");
            }
            if (s_status.Failures.LowVoltage) {
                // qDebug() << QString("ClassPrinter -- SFailures::LowVoltage");
            }
            if (s_status.Failures.HighVoltage) {
                // qDebug() << QString("ClassPrinter -- SFailures::HighVoltage");
            }
            if (s_status.Failures.Memory) {
                // qDebug() << QString("ClassPrinter -- SFailures::Memory");
            }
            if (s_status.Failures.CRC) {
                // qDebug() << QString("ClassPrinter -- SFailures::CRC");
            }
            if (s_status.Failures.Presentor) {
                // qDebug() << QString("ClassPrinter -- SFailures::Presentor");
            }
            if (s_status.Failures.CPU) {
                // qDebug() << QString("ClassPrinter -- SFailures::CPU");
            }
        } else {
            // qDebug() << QString("ClassPrinter -- get status false --
            // %1").arg(system_Model);
            if (s_status.NotAvailabled) {
                // qDebug() << QString("ClassPrinter -- NotAvailabled");
            }
        }
    }

    if (system_Model == PrinterModel::Citizen_CTS2000) {
        bool getSts = CitizenCTS2000->isEnabled(i_status);
        //        //qDebug() << "-------i_status - " << QString::number(i_status);
        if (getSts) {
            // qDebug() << "-------getSts true-------";
        } else {
            // qDebug() << "-------getSts false-------";
        }
    }
    if (system_Model == PrinterModel::Windows_Printer) {
        i_status = PrinterState::PrinterOK;
    }
    if (system_Model == PrinterModel::Phoenix_model) {
        i_status = PrinterState::PrinterOK;

        bool getSts = Phoenix->isEnabled(i_status);
        if (getSts) {
            // qDebug() << "getSts true";
        } else {
            // qDebug() << "getSts false";
        }
    }

    if (system_Model == PrinterModel::KM1X) {
        i_status = PrinterState::PrinterOK;

        bool getSts = KM1X->isEnabled();
        if (getSts) {
            // qDebug() << "getSts true";
        } else {
            // qDebug() << "getSts false";
        }
    }
    status = i_status;
    emit this->emit_status(i_status);
}

bool ClassPrinter::printerOpen() {
    // qDebug() << "Start to open printer port";
    // qDebug() << "PrinterModel - " << system_Model;
    bool open_port = false;
    if (system_Model == PrinterModel::Custom_VKP80) {
        Custom_VKP80->setPortName(portName);
        bool port_printer = false;
        port_printer = Custom_VKP80->OpenPrinterPort();
        if (port_printer) {
            // qDebug() << QString("ClassPrinter -- port opened --
            // %1").arg(system_Model);
            open_port = true;
        } else {
            // qDebug() << QString("ClassPrinter -- error port opened --
            // %1").arg(system_Model);
            open_port = false;
        }
    }

    if (system_Model == PrinterModel::Custom_TG2480) {
        Custom_TG2480->setPortName(portName);
        bool port_printer = false;
        port_printer = Custom_TG2480->OpenPrinterPort();
        if (port_printer) {
            // qDebug() << QString("ClassPrinter -- port opened --
            // %1").arg(system_Model);
            open_port = true;
        } else {
            // qDebug() << QString("ClassPrinter -- error port opened --
            // %1").arg(system_Model);
            open_port = false;
        }
    }

    if (system_Model == PrinterModel::AV_268) {
        AV268->setPortName(portName);
        bool port_printer = false;
        port_printer = AV268->OpenPrinterPort();
        if (port_printer) {
            // qDebug() << QString("ClassPrinter -- port opened --
            // %1").arg(system_Model);
            open_port = true;
        } else {
            // qDebug() << QString("ClassPrinter -- error port opened --
            // %1").arg(system_Model);
            open_port = false;
        }
    }
    if (system_Model == PrinterModel::CitizenCBM1000) {
        CitizenCBM1000->setPortName(portName);
        bool port_printer = false;
        port_printer = CitizenCBM1000->OpenPrinterPort();
        if (port_printer) {
            // qDebug() << QString("ClassPrinter -- port opened --
            // %1").arg(system_Model);
            open_port = true;
        } else {
            // qDebug() << QString("ClassPrinter -- error port opened --
            // %1").arg(system_Model);
            open_port = false;
        }
    }
    if (system_Model == PrinterModel::Citizen_PPU700) {
        CitizenPPU700->setPortName(portName);
        bool port_printer = false;
        port_printer = CitizenPPU700->OpenPrinterPort();
        if (port_printer) {
            // qDebug() << QString("ClassPrinter -- port opened --
            // %1").arg(system_Model);
            open_port = true;
        } else {
            // qDebug() << QString("ClassPrinter -- error port opened --
            // %1").arg(system_Model);
            open_port = false;
        }
    }
    if (system_Model == PrinterModel::Citizen_CTS2000) {
        CitizenCTS2000->setPortName(portName);
        bool port_printer = false;
        port_printer = CitizenCTS2000->OpenPrinterPort();
        if (port_printer) {
            // qDebug() << QString("ClassPrinter -- port opened --
            // %1").arg(system_Model);
            open_port = true;
        } else {
            // qDebug() << QString("ClassPrinter -- error port opened --
            // %1").arg(system_Model);
            open_port = false;
        }
    }
    if (system_Model == PrinterModel::Phoenix_model) {
        Phoenix->setPortName(portName);
        bool port_printer = false;
        port_printer = Phoenix->OpenPrinterPort();
        if (port_printer) {
            // qDebug() << QString("ClassPrinter -- port opened --
            // %1").arg(system_Model);
            open_port = true;
        } else {
            // qDebug() << QString("ClassPrinter -- error port opened --
            // %1").arg(system_Model);
            open_port = false;
        }
    }

    if (system_Model == PrinterModel::KM1X) {
        KM1X->port_speed = this->portSpeed;
        KM1X->setPortName(portName);
        bool port_printer = false;
        port_printer = KM1X->OpenPrinterPort();
        if (port_printer) {
            // qDebug() << QString("ClassPrinter -- port opened --
            // %1").arg(system_Model);
            open_port = true;
        } else {
            // qDebug() << QString("ClassPrinter -- error port opened --
            // %1").arg(system_Model);
            open_port = false;
        }
    }

    return open_port;
}

void ClassPrinter::closeThis() {
    if (system_Model == PrinterModel::Custom_VKP80) {
        Custom_VKP80->closePort();
    }

    if (system_Model == PrinterModel::Custom_TG2480) {
        Custom_TG2480->closePort();
    }

    if (system_Model == PrinterModel::CitizenCBM1000) {
        CitizenCBM1000->closePort();
    }

    if (system_Model == PrinterModel::AV_268) {
        AV268->closePort();
    }

    if (system_Model == PrinterModel::Citizen_PPU700) {
        CitizenPPU700->closePort();
    }

    if (system_Model == PrinterModel::Citizen_CTS2000) {
        CitizenCTS2000->closePort();
    }

    if (system_Model == PrinterModel::Phoenix_model) {
        Phoenix->closePort();
    }

    if (system_Model == PrinterModel::KM1X) {
        KM1X->closePort();
    }

    if (system_Model == PrinterModel::Windows_Printer) {
        clearListPrinterData(this->winPrinterName);
    }
}

#ifdef Q_OS_WIN32
void ClassPrinter::clearListPrinterData(QString name) {
    QString prtNameIn = name;

    //    long bufsize = 0x100;

    std::wstring wStr = prtNameIn.toStdWString();
    LPWSTR lpszStr = const_cast<LPWSTR>(wStr.c_str());

    //    char buf[300];
    //    int temp = GetLastError();

    HANDLE hPrinter;

    if (OpenPrinterW(lpszStr, &hPrinter, NULL) == 0) {
        // qDebug() << "OpenPrinter call failed";

        return;
    }

    DWORD dwBufsize = 0;

    // Getting some info from printer - particularly how many jobs
    GetPrinter(hPrinter, 2, NULL, 0, &dwBufsize);

    PRINTER_INFO_2 *pinfo = (PRINTER_INFO_2 *)malloc(dwBufsize);
    GetPrinter(hPrinter, 2, (LPBYTE)pinfo, dwBufsize, &dwBufsize);
    DWORD num_Jobs = pinfo->cJobs;
    free(pinfo); // free now

    if (num_Jobs == 0) {
        // qDebug() << "No printer jobs found.";
    } else {
        // Some Jobs in queue

        JOB_INFO_1 *pJobInfo = 0;
        DWORD bytesNeeded = 0, jobsReturned = 0;

        // Get info about jobs in queue.
        Enum_Jobs(hPrinter, 0, num_Jobs, 1, (LPBYTE)pJobInfo, 0, &bytesNeeded, &jobsReturned);
        pJobInfo = (JOB_INFO_1 *)malloc(bytesNeeded);
        Enum_Jobs(
            hPrinter, 0, num_Jobs, 1, (LPBYTE)pJobInfo, bytesNeeded, &bytesNeeded, &jobsReturned);

        // Loop and delete each waiting job
        for (int count = 0; count < int(jobsReturned); count++) {
            // qDebug() << "Deleting JobID  " << pJobInfo[count].JobId;

            if (SetJob(hPrinter, pJobInfo[count].JobId, 0, NULL, JOB_CONTROL_DELETE) != 0) {
                // qDebug() << "...... Deleted OK";
            } else {
                // qDebug() << "...... Failed to Delete";
            }
        }

        free(pJobInfo); // free now
    }

    // Finished with the printer
    ClosePrinter(hPrinter);
}

bool GetJobs(HANDLE hPrinter, /* Handle to the printer. */

             JOB_INFO_2 **ppJobInfo, /* Pointer to be filled.  */
             int *pcJobs,            /* Count of jobs filled.  */
             DWORD *pStatus)         /* Print Queue status.    */

{
    DWORD cByteNeeded, nReturned, cByteUsed;
    JOB_INFO_2 *pJobStorage = NULL;
    PRINTER_INFO_2 *pPrinterInfo = NULL;

    /* Get the buffer size needed. */
    if (!GetPrinter(hPrinter, 2, NULL, 0, &cByteNeeded)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return false;
    }

    pPrinterInfo = (PRINTER_INFO_2 *)malloc(cByteNeeded);
    if (!(pPrinterInfo))
        /* Failure to allocate memory. */
        return false;

    /* Get the printer information. */
    if (!GetPrinter(hPrinter, 2, (LPBYTE)pPrinterInfo, cByteNeeded, &cByteUsed)) {
        /* Failure to access the printer. */
        free(pPrinterInfo);
        pPrinterInfo = NULL;
        return false;
    }

    /* Get job storage space. */
    if (!Enum_Jobs(hPrinter,
                   0,
                   pPrinterInfo->cJobs,
                   2,
                   NULL,
                   0,
                   (LPDWORD)&cByteNeeded,
                   (LPDWORD)&nReturned)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            free(pPrinterInfo);
            pPrinterInfo = NULL;
            return false;
        }
    }

    pJobStorage = (JOB_INFO_2 *)malloc(cByteNeeded);
    if (!pJobStorage) {
        /* Failure to allocate Job storage space. */
        free(pPrinterInfo);
        pPrinterInfo = NULL;
        return false;
    }

    ZeroMemory(pJobStorage, cByteNeeded);

    /* Get the list of jobs. */
    if (!Enum_Jobs(hPrinter,
                   0,
                   pPrinterInfo->cJobs,
                   2,
                   (LPBYTE)pJobStorage,
                   cByteNeeded,
                   (LPDWORD)&cByteUsed,
                   (LPDWORD)&nReturned)) {
        free(pPrinterInfo);
        free(pJobStorage);
        pJobStorage = NULL;
        pPrinterInfo = NULL;
        return false;
    }

    /*
     *  Return the information.
     */
    *pcJobs = nReturned;
    *pStatus = pPrinterInfo->Status;
    *ppJobInfo = pJobStorage;
    free(pPrinterInfo);

    return true;
}

bool IsPrinterError(HANDLE hPrinter) {
    JOB_INFO_2 *pJobs;
    int cJobs, i;
    DWORD dwPrinterStatus;

    /*
     *  Get the state information for the Printer Queue and
     *  the jobs in the Printer Queue.
     */
    if (!GetJobs(hPrinter, &pJobs, &cJobs, &dwPrinterStatus))
        return false;

    /*
     *  If the Printer reports an error, believe it.
     */
    if (dwPrinterStatus == PRINTER_STATUS_ERROR || dwPrinterStatus == PRINTER_STATUS_PAPER_JAM ||
        dwPrinterStatus == PRINTER_STATUS_PAPER_OUT ||
        dwPrinterStatus == PRINTER_STATUS_PAPER_PROBLEM ||
        dwPrinterStatus == PRINTER_STATUS_OUTPUT_BIN_FULL ||
        dwPrinterStatus == PRINTER_STATUS_NOT_AVAILABLE ||
        dwPrinterStatus == PRINTER_STATUS_NO_TONER ||
        dwPrinterStatus == PRINTER_STATUS_OUT_OF_MEMORY ||
        dwPrinterStatus == PRINTER_STATUS_OFFLINE || dwPrinterStatus == PRINTER_STATUS_DOOR_OPEN) {
        free(pJobs);
        return true;
    }

    /*
     *  Find the Job in the Queue that is printing.
     */
    for (i = 0; i < cJobs; i++) {
        if (pJobs[i].Status == JOB_STATUS_PRINTING) {
            /*
             *  If the job is in an error state,
             *  report an error for the printer.
             *  Code could be inserted here to
             *  attempt an interpretation of the
             *  pStatus member as well.
             */
            if (pJobs[i].Status == JOB_STATUS_ERROR || pJobs[i].Status == JOB_STATUS_OFFLINE ||
                pJobs[i].Status == JOB_STATUS_PAPEROUT ||
                pJobs[i].Status == JOB_STATUS_BLOCKED_DEVQ) {
                free(pJobs);
                return true;
            }
        }
    }

    /*
     *  No error condition.
     */
    free(pJobs);
    return false;
}
#endif // Q_OS_WIN32

#ifndef Q_OS_WIN32
void ClassPrinter::clearListPrinterData(QString /*name*/) {
    // Stub implementation for non-Windows platforms
    // Printer job management is Windows-specific
}
#endif // !Q_OS_WIN32
