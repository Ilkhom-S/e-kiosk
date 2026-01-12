// Project
#include "incasaciyaform.h"
#include "ui_incasaciyaform.h"

IncasaciyaForm::IncasaciyaForm(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint),
    ui(new Ui::IncasaciyaForm)
{
    ui->setupUi(this);

    connect(ui->btnGetIncasaciya,SIGNAL(clicked()),SLOT(btnIncashClc()));
    connect(ui->btnInterToAdmin,SIGNAL(clicked()),SLOT(btnInterAdminClc()));
    connect(ui->btnPrinterTest,SIGNAL(clicked()),SLOT(btnTestPrintClc()));
    connect(ui->btnNullingCheck,SIGNAL(clicked()),SLOT(btnDoNullingClc()));
    connect(ui->btnCloseIncasaciya,SIGNAL(clicked()),SLOT(btnCloseClc()));
    connect(ui->btnChangePass,SIGNAL(clicked()),SLOT(btnChangePassClc()));

    ui->btnChangePass->setVisible(false);
    ui->btnNullingCheck->setVisible(false);
    ui->btnPrinterTest->setVisible(false);
}

IncasaciyaForm::~IncasaciyaForm()
{
    delete ui;
}


void IncasaciyaForm::btnIncashClc()
{
    emit execCommand(IncashCmd::doIncash);

    QCoreApplication::processEvents();
}

void IncasaciyaForm::btnInterAdminClc()
{
    emit execCommand(IncashCmd::interAdmin);
}

void IncasaciyaForm::btnTestPrintClc()
{
    emit execCommand(IncashCmd::testPrint);
}

void IncasaciyaForm::btnDoNullingClc()
{
    emit execCommand(IncashCmd::doNullingCheck);
}

void IncasaciyaForm::btnCloseClc()
{
    emit execCommand(IncashCmd::closeThis);
}

void IncasaciyaForm::setHtmlInfoBox(QString text)
{
    ui->graphicsViewI->setHtml(text);
}

void IncasaciyaForm::btnChangePassClc()
{
    emit openDialog();
}
