// Project
#include "loadinggprsform.h"
#include "ui_loadinggprsform.h"

LoadingGprsForm::LoadingGprsForm(QWidget *parent) : QWidget(parent), ui(new Ui::LoadingGprsForm) {
    ui->setupUi(this);

    ui->loading_gprs_info->setVisible(false);
}

LoadingGprsForm::~LoadingGprsForm() {
    delete ui;
}

void LoadingGprsForm::setAboutCompany(QString text) {
    // ui->loading_about_company->setText(text);
}

void LoadingGprsForm::setCopirightText(QString text, QString version) {
    ui->loading_copyrigh_info->setText(text);
}

void LoadingGprsForm::setSimInfo(QString text) {
    ui->loading_sim_info->setText(text);
}

void LoadingGprsForm::setGprsInfo(QString text) {
    ui->loading_gprs_info->setText(text);
}

void LoadingGprsForm::setGprsComment(QString text) {
    ui->lblcommentInfoGprs->setText(text);
}

void LoadingGprsForm::setLogo(QString path) {
    if (path.isEmpty()) {
        return;
    }

    ui->loading_devices_logo->setStyleSheet(QString("image: url(%1);").arg(path));
}
