#include "adminbutton.h"
#include "ui_adminbutton.h"

AdminButton::AdminButton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminButton)
{
    ui->setupUi(this);
}

AdminButton::~AdminButton()
{
    delete ui;
}

void AdminButton::on_btnExplorer_clicked()
{
    emit explorerCliked();
}

void AdminButton::on_btnKeyboard_clicked()
{
    emit keyboardClicked();
}

void AdminButton::on_btnClose_clicked()
{
    emit closeClicked();
}
