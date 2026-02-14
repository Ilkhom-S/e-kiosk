/* @file Виджет виртуальной клавиатуры */

#pragma once

#include <QtCore/QMetaType>

#include "ui_KeyboardWindow.h"

//------------------------------------------------------------------------
class VirtualButton {
public:
    VirtualButton() : m_Key(Qt::Key_unknown), m_ShiftKey(Qt::Key_unknown) {}
    VirtualButton(Qt::Key aKey, const QString &aText, Qt::Key aShiftKey, const QString &aShiftText)
        : m_Key(aKey), m_Text(aText), m_ShiftKey(aShiftKey), m_ShiftText(aShiftText) {}

public:
    Qt::Key getKey(bool aShifted) { return aShifted ? m_ShiftKey : m_Key; }
    QString getText(bool aShifted) { return aShifted ? m_ShiftText : m_Text; }

private:
    Qt::Key m_Key;
    QString m_Text;

    Qt::Key m_ShiftKey;
    QString m_ShiftText;
};

//------------------------------------------------------------------------
Q_DECLARE_METATYPE(VirtualButton);

//------------------------------------------------------------------------
class KeyboardWindow : public QWidget {
    Q_OBJECT

public:
    KeyboardWindow(QWidget *parent = 0);
    ~KeyboardWindow();

public:
    void initialize();
    void shutdown();

private slots:
    void onButtonClicked();
    void onShiftClicked() {
        m_Shifted = ui.KEY_SHIFT->isChecked();
        updateKeys();
    }
    void onLanguageClicked() {
        m_AltMode = !m_AltMode;
        updateKeys();
    }

private:
    void updateKeys();
    virtual void mousePressEvent(QMouseEvent *aEvent);

private:
    Ui::KeyboardWindow ui{};

private:
    typedef QMap<QString, VirtualButton> TKeyMap;
    TKeyMap m_KeyMap;
    TKeyMap m_AltKeyMap;

    bool m_Shifted;
    bool m_AltMode;
};

//------------------------------------------------------------------------
