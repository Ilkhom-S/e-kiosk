#include "searchdevicesform.h"

#include <QMovie>
#include <QStyleFactory>

#include "ui_searchdevicesform.h"

SearchDevicesForm::SearchDevicesForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::SearchDevicesForm) {
    ui->setupUi(this);

    //    ui->loading_validator_info->setVisible(false);
    ui->loading_coin_acceptor_info->setVisible(false);
    ui->loading_coin_acceptor_search->setVisible(false);
    //    ui->loading_printer_info->setVisible(false);
    //    ui->loading_modem_info->setVisible(false);

    ui->loading_wd_info->setVisible(false);

    movie = new QMovie("assets/images/progress.gif");
    srchMovie = new QMovie(":/assets/icons/search.gif");

    ui->loading_validator_search->setVisible(false);
    ui->loading_validator_state->setVisible(false);
    ui->loading_validator_info->setVisible(false);

    ui->loading_coin_acceptor_search->setVisible(false);
    ui->loading_coin_acceptor_state->setVisible(false);
    ui->loading_coin_acceptor_info->setVisible(false);

    ui->loading_printer_search->setVisible(false);
    ui->loading_printer_state->setVisible(false);
    ui->loading_printer_info->setVisible(false);

    ui->loading_modem_search->setVisible(false);
    ui->loading_modem_state->setVisible(false);
    ui->loading_modem_info->setVisible(false);

    if (movie->isValid()) {
        ui->lblProgress->setMovie(movie);
        movie->start();
    }
}

SearchDevicesForm::~SearchDevicesForm() { delete ui; }

void SearchDevicesForm::setAboutCompany(QString text) {
    // ui->loading_about_company->setText(text);
}

void SearchDevicesForm::setValidatorSearchText(int state, QString text) {
    if (state == 0) {
        ui->loading_validator_search->setVisible(true);
        ui->loading_validator_search->setMovie(srchMovie);
        srchMovie->start();
    } else if (state == 1) {
        ui->loading_validator_search->setVisible(false);
        ui->loading_validator_state->setVisible(true);
        ui->loading_validator_state->setPixmap(QPixmap(":/assets/icons/search_ok.png"));
    } else if (state == 2) {
        ui->loading_validator_search->setVisible(false);
        ui->loading_validator_state->setVisible(true);
        ui->loading_validator_state->setPixmap(QPixmap(":/assets/icons/search_no.png"));
    }

    if (!ui->loading_validator_info->isVisible()) {
        ui->loading_validator_info->setVisible(true);
    }

    ui->loading_validator_info->setText(text);
}

void SearchDevicesForm::setCoinAcceptorSearchText(int state, QString text) {
    if (state == 0) {
        ui->loading_coin_acceptor_search->setVisible(true);
        ui->loading_coin_acceptor_search->setMovie(srchMovie);
        srchMovie->start();
    } else if (state == 1) {
        ui->loading_coin_acceptor_search->setVisible(false);
        ui->loading_coin_acceptor_state->setVisible(true);
        ui->loading_coin_acceptor_state->setPixmap(QPixmap(":/assets/icons/search_ok.png"));
    } else if (state == 2) {
        ui->loading_coin_acceptor_search->setVisible(false);
        ui->loading_coin_acceptor_state->setVisible(true);
        ui->loading_coin_acceptor_state->setPixmap(QPixmap(":/assets/icons/search_no.png"));
    }

    if (!ui->loading_coin_acceptor_info->isVisible()) {
        ui->loading_coin_acceptor_info->setVisible(true);
    }

    ui->loading_coin_acceptor_info->setText(text);
}

void SearchDevicesForm::setWDSearchText(int state, QString text) {
    Q_UNUSED(state)
    Q_UNUSED(text)
    //    if(!ui->loading_wd_info->isVisible()) ui->loading_wd_info->setVisible(true);
    //    ui->loading_wd_info->setText(text);
}

void SearchDevicesForm::setPrinterSearchText(int state, QString text) {
    if (state == 0) {
        ui->loading_printer_search->setVisible(true);
        ui->loading_printer_search->setMovie(srchMovie);
        srchMovie->start();
    } else if (state == 1) {
        ui->loading_printer_search->setVisible(false);
        ui->loading_printer_state->setVisible(true);
        ui->loading_printer_state->setPixmap(QPixmap(":/assets/icons/search_ok.png"));
    } else if (state == 2) {
        ui->loading_printer_search->setVisible(false);
        ui->loading_printer_state->setVisible(true);
        ui->loading_printer_state->setPixmap(QPixmap(":/assets/icons/search_no.png"));
    }

    if (!ui->loading_printer_info->isVisible()) {
        ui->loading_printer_info->setVisible(true);
    }

    ui->loading_printer_info->setText(text);
}

void SearchDevicesForm::setModemSearchText(int state, QString text) {
    if (state == 0) {
        ui->loading_modem_search->setVisible(true);
        ui->loading_modem_search->setMovie(srchMovie);
        srchMovie->start();
    } else if (state == 1) {
        ui->loading_modem_search->setVisible(false);
        ui->loading_modem_state->setVisible(true);
        ui->loading_modem_state->setPixmap(QPixmap(":/assets/icons/search_ok.png"));
    } else if (state == 2) {
        ui->loading_modem_search->setVisible(false);
        ui->loading_modem_state->setVisible(true);
        ui->loading_modem_state->setPixmap(QPixmap(":/assets/icons/search_no.png"));
    }

    if (!ui->loading_modem_info->isVisible()) {
        ui->loading_modem_info->setVisible(true);
    }

    ui->loading_modem_info->setText(text);
}

void SearchDevicesForm::setLogo(QString path) {
    if (path.isEmpty()) {
        return;
    }

    ui->loading_devices_logo->setStyleSheet(QString("image: url(%1);").arg(path));
}

void SearchDevicesForm::setCopirightText(QString text, QString version) {
    Q_UNUSED(version)

    ui->loading_copyrigh_info->setText(text);
}
