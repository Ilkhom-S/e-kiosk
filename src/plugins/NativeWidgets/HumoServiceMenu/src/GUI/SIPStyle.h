/* @file Новый стиль QLineEdit для нужного поведения виртуальной клавиатуры */

#pragma once

#include <QtWidgets/QProxyStyle>

//------------------------------------------------------------------------
class SIPStyle : public QProxyStyle {
public:
    virtual int styleHint(StyleHint hint,
                          const QStyleOption *option = 0,
                          const QWidget *widget = 0,
                          QStyleHintReturn *returnData = 0) const {
        return hint == SH_RequestSoftwareInputPanel
                   ? RSIP_OnMouseClick
                   : QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

//------------------------------------------------------------------------
