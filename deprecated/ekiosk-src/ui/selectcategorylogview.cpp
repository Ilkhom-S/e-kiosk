#include "selectcategorylogview.h"

#include "ui_selectcategorylogview.h"

SelectCategoryLogView::SelectCategoryLogView(QWidget *parent)
    : QDialog(parent), ui(new Ui::SelectCategoryLogView) {
    ui->setupUi(this);

    connect(ui->btnSelectDataAllS, SIGNAL(clicked()), SLOT(selectAll()));

    connect(ui->btnShowSelectData, SIGNAL(clicked()), SLOT(showLikeThis()));

    connect(ui->btnDeSelectDataAllS, SIGNAL(clicked()), SLOT(deSelectAll()));
}

void SelectCategoryLogView::deSelectAll() {
    ui->chbxSelectValidatorJam->setChecked(false);
    ui->chbxSelectMoneyOut->setChecked(false);
    ui->chbxSelectERROR->setChecked(false);
    ui->chbxSelectPayDaemon->setChecked(false);
    ui->chbxSelectStatusAso->setChecked(false);
    ui->chbxSelectStatusPrinter->setChecked(false);
    ui->chbxSelectStatusValidator->setChecked(false);
    ui->chbxSelectConnectionState->setChecked(false);
    ui->chbxSelectUpdater->setChecked(false);
}

void SelectCategoryLogView::showLikeThis() {
    bool selectValidatorJam = ui->chbxSelectValidatorJam->checkState() != 0U;
    bool selectMoneyOut = ui->chbxSelectMoneyOut->checkState() != 0U;
    bool selectError = ui->chbxSelectERROR->checkState() != 0U;
    bool selectPayDaemon = ui->chbxSelectPayDaemon->checkState() != 0U;
    bool selectStatusAso = ui->chbxSelectStatusAso->checkState() != 0U;
    bool selectStatusPrinter = ui->chbxSelectStatusPrinter->checkState() != 0U;
    bool selectStatusValidator = ui->chbxSelectStatusValidator->checkState() != 0U;
    bool selectConnectionState = ui->chbxSelectConnectionState->checkState() != 0U;
    bool selectUpdater = ui->chbxSelectUpdater->checkState() != 0U;

    emit this->emit_SelectOptions(selectValidatorJam,
                                  selectMoneyOut,
                                  selectError,
                                  selectPayDaemon,
                                  selectStatusAso,
                                  selectStatusPrinter,
                                  selectStatusValidator,
                                  selectConnectionState,
                                  selectUpdater);

    this->close();
}

void SelectCategoryLogView::selectAll() {
    ui->chbxSelectValidatorJam->setChecked(true);
    ui->chbxSelectMoneyOut->setChecked(true);
    ui->chbxSelectERROR->setChecked(true);
    ui->chbxSelectPayDaemon->setChecked(true);
    ui->chbxSelectStatusAso->setChecked(true);
    ui->chbxSelectStatusPrinter->setChecked(true);
    ui->chbxSelectStatusValidator->setChecked(true);
    ui->chbxSelectConnectionState->setChecked(true);
    ui->chbxSelectUpdater->setChecked(true);
}

SelectCategoryLogView::~SelectCategoryLogView() {
    delete ui;
}
