/* @file Виджет отображения диалога ввода данных */

#include "InputBox.h"

#include "SIPStyle.h"

//---------------------------------------------------------------------------
InputBox::InputBox(QWidget *parent, ValidatorFunction aValidator)
    : QWidget(parent), m_Validator(aValidator) {
    ui.setupUi(this);

    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(onOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(onCancel()));

    ui.btnOK->setEnabled(false);

    ui.lineEdit->setStyle(new SIPStyle);
    connect(ui.lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onTextChanged()));
}

//---------------------------------------------------------------------------
InputBox::~InputBox() = default;

//---------------------------------------------------------------------------
void InputBox::setLabelText(const QString &aText) {
    ui.label->setText(aText);
}

//---------------------------------------------------------------------------
void InputBox::setTextValue(const QString &aValue) {
    ui.lineEdit->setText(aValue);
}

//---------------------------------------------------------------------------
void InputBox::onOK() {
    close();

    emit accepted();
}

//---------------------------------------------------------------------------
void InputBox::onCancel() {
    close();
}

//---------------------------------------------------------------------------
QString InputBox::textValue() const {
    return ui.lineEdit->text();
}

//---------------------------------------------------------------------------
void InputBox::mySetFocus() {
    ui.lineEdit->setFocus();
    qApp->sendEvent(ui.lineEdit, new QEvent(QEvent::RequestSoftwareInputPanel));
}

//---------------------------------------------------------------------------
void InputBox::showEvent(QShowEvent *aEvent) {
    QWidget::showEvent(aEvent);

    QTimer::singleShot(100, this, SLOT(mySetFocus()));
}

//---------------------------------------------------------------------------
void InputBox::onTextChanged() {
    ui.btnOK->setEnabled(m_Validator(ui.lineEdit->text()));
}

//---------------------------------------------------------------------------
