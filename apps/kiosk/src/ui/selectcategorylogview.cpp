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
    bool SelectValidatorJam = ui->chbxSelectValidatorJam->checkState();
    bool SelectMoneyOut = ui->chbxSelectMoneyOut->checkState();
    bool SelectERROR = ui->chbxSelectERROR->checkState();
    bool SelectPayDaemon = ui->chbxSelectPayDaemon->checkState();
    bool SelectStatusAso = ui->chbxSelectStatusAso->checkState();
    bool SelectStatusPrinter = ui->chbxSelectStatusPrinter->checkState();
    bool SelectStatusValidator = ui->chbxSelectStatusValidator->checkState();
    bool SelectConnectionState = ui->chbxSelectConnectionState->checkState();
    bool SelectUpdater = ui->chbxSelectUpdater->checkState();

    emit this->emit_SelectOptions(SelectValidatorJam, SelectMoneyOut, SelectERROR, SelectPayDaemon,
                                  SelectStatusAso, SelectStatusPrinter, SelectStatusValidator,
                                  SelectConnectionState, SelectUpdater);

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

SelectCategoryLogView::~SelectCategoryLogView() { delete ui; }
