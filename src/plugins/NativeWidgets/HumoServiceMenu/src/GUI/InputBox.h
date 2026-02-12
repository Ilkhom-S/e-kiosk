/* @file Виджет отображения диалога ввода данных */

#pragma once

// std
#include <QtCore/QTimer>

#include <functional>

#include "ui_InputBox.h"

//---------------------------------------------------------------------------
class InputBox : public QWidget {
    Q_OBJECT

public:
    typedef std::function<bool(const QString &)> ValidatorFunction;

public:
    InputBox(QWidget *parent, ValidatorFunction aValidator);
    ~InputBox();

public:
    void setLabelText(const QString &aText);
    void setTextValue(const QString &aValue);

    QString textValue() const;

signals:
    void accepted();

private slots:
    void onOK();
    void onCancel();
    void mySetFocus();
    void onTextChanged();

private:
    virtual void showEvent(QShowEvent *aEvent);

private:
    Ui::InputBox ui{};
    ValidatorFunction m_Validator;
};

//---------------------------------------------------------------------------
