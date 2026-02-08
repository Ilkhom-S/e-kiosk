/* @file Виджет виртуальной клавиатуры */

#include "KeyboardWindow.h"

#include <QtGui/QKeyEvent>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>

#include <SDK/PaymentProcessor/Core/IGUIService.h>

KeyboardWindow::KeyboardWindow(QWidget *parent)
    : QWidget(parent), m_Shifted(false), m_AltMode(false) {
    ui.setupUi(this);

    connect(ui.KEY_SHIFT, SIGNAL(clicked()), this, SLOT(onShiftClicked()));
    connect(ui.KEY_LANG, SIGNAL(clicked()), this, SLOT(onLanguageClicked()));

    QList<QToolButton *> padButtons = this->findChildren<QToolButton *>();
    foreach (QToolButton *button, padButtons) {
        connect(button, SIGNAL(clicked()), SLOT(onButtonClicked()));
    }
}

//---------------------------------------------------------------------------
KeyboardWindow::~KeyboardWindow() {}

//---------------------------------------------------------------------------
void KeyboardWindow::initialize() {
    // lat
    m_KeyMap["KEY_BACKSPACE"] = VirtualButton(Qt::Key_Backspace, "", Qt::Key_Backspace, "");
    m_KeyMap["KEY_ENTER"] = VirtualButton(Qt::Key_Enter, "", Qt::Key_Enter, "");
    m_KeyMap["KEY_SHIFT"] = VirtualButton(Qt::Key_Shift, "", Qt::Key_Shift, "");
    m_KeyMap["KEY_SPACE"] = VirtualButton(Qt::Key_Space, " ", Qt::Key_Space, " ");
    m_KeyMap["KEY_SEMICOLON"] = VirtualButton(Qt::Key_Semicolon, ";", Qt::Key_Colon, ":");
    m_KeyMap["KEY_APOSTROPHE"] = VirtualButton(Qt::Key_Apostrophe, "'", Qt::Key_QuoteDbl, "\"");
    m_KeyMap["KEY_MINUS"] = VirtualButton(Qt::Key_Minus, "-", Qt::Key_Underscore, "_");
    m_KeyMap["KEY_EQUAL"] = VirtualButton(Qt::Key_Equal, "=", Qt::Key_Plus, "+");
    m_KeyMap["KEY_COMMA"] = VirtualButton(Qt::Key_Comma, ",", Qt::Key_Less, "<");
    m_KeyMap["KEY_PERIOD"] = VirtualButton(Qt::Key_Period, ".", Qt::Key_Greater, ">");
    m_KeyMap["KEY_SLASH"] = VirtualButton(Qt::Key_Slash, "/", Qt::Key_Question, "?");
    m_KeyMap["KEY_BRACKETLEFT"] = VirtualButton(Qt::Key_BracketLeft, "[", Qt::Key_BraceLeft, "{");
    m_KeyMap["KEY_BRACKETRIGHT"] =
        VirtualButton(Qt::Key_BracketRight, "]", Qt::Key_BraceRight, "}");
    m_KeyMap["KEY_BACKSLASH"] = VirtualButton(Qt::Key_Backslash, "\\", Qt::Key_Bar, "|");
    m_KeyMap["KEY_1"] = VirtualButton(Qt::Key_1, "1", Qt::Key_Exclam, "!");
    m_KeyMap["KEY_2"] = VirtualButton(Qt::Key_2, "2", Qt::Key_At, "@");
    m_KeyMap["KEY_3"] = VirtualButton(Qt::Key_3, "3", Qt::Key_NumberSign, "#");
    m_KeyMap["KEY_4"] = VirtualButton(Qt::Key_4, "4", Qt::Key_Dollar, "$");
    m_KeyMap["KEY_5"] = VirtualButton(Qt::Key_5, "5", Qt::Key_Percent, "%");
    m_KeyMap["KEY_6"] = VirtualButton(Qt::Key_6, "6", Qt::Key_AsciiCircum, "^");
    m_KeyMap["KEY_7"] = VirtualButton(Qt::Key_7, "7", Qt::Key_Ampersand, "&");
    m_KeyMap["KEY_8"] = VirtualButton(Qt::Key_8, "8", Qt::Key_Asterisk, "*");
    m_KeyMap["KEY_9"] = VirtualButton(Qt::Key_9, "9", Qt::Key_ParenLeft, "(");
    m_KeyMap["KEY_0"] = VirtualButton(Qt::Key_0, "0", Qt::Key_ParenRight, ")");
    m_KeyMap["KEY_A"] = VirtualButton(Qt::Key_A, "a", Qt::Key_A, "A");
    m_KeyMap["KEY_B"] = VirtualButton(Qt::Key_B, "b", Qt::Key_B, "B");
    m_KeyMap["KEY_C"] = VirtualButton(Qt::Key_C, "c", Qt::Key_C, "C");
    m_KeyMap["KEY_D"] = VirtualButton(Qt::Key_D, "d", Qt::Key_D, "D");
    m_KeyMap["KEY_E"] = VirtualButton(Qt::Key_E, "e", Qt::Key_E, "E");
    m_KeyMap["KEY_F"] = VirtualButton(Qt::Key_F, "f", Qt::Key_F, "F");
    m_KeyMap["KEY_G"] = VirtualButton(Qt::Key_G, "g", Qt::Key_G, "G");
    m_KeyMap["KEY_H"] = VirtualButton(Qt::Key_H, "h", Qt::Key_H, "H");
    m_KeyMap["KEY_I"] = VirtualButton(Qt::Key_I, "i", Qt::Key_I, "I");
    m_KeyMap["KEY_J"] = VirtualButton(Qt::Key_J, "j", Qt::Key_J, "J");
    m_KeyMap["KEY_K"] = VirtualButton(Qt::Key_K, "k", Qt::Key_K, "K");
    m_KeyMap["KEY_L"] = VirtualButton(Qt::Key_L, "l", Qt::Key_L, "L");
    m_KeyMap["KEY_M"] = VirtualButton(Qt::Key_M, "m", Qt::Key_M, "M");
    m_KeyMap["KEY_N"] = VirtualButton(Qt::Key_N, "n", Qt::Key_N, "N");
    m_KeyMap["KEY_O"] = VirtualButton(Qt::Key_O, "o", Qt::Key_O, "O");
    m_KeyMap["KEY_P"] = VirtualButton(Qt::Key_P, "p", Qt::Key_P, "P");
    m_KeyMap["KEY_Q"] = VirtualButton(Qt::Key_Q, "q", Qt::Key_Q, "Q");
    m_KeyMap["KEY_R"] = VirtualButton(Qt::Key_R, "r", Qt::Key_R, "R");
    m_KeyMap["KEY_S"] = VirtualButton(Qt::Key_S, "s", Qt::Key_S, "S");
    m_KeyMap["KEY_T"] = VirtualButton(Qt::Key_T, "t", Qt::Key_T, "T");
    m_KeyMap["KEY_U"] = VirtualButton(Qt::Key_U, "u", Qt::Key_U, "U");
    m_KeyMap["KEY_V"] = VirtualButton(Qt::Key_V, "v", Qt::Key_V, "V");
    m_KeyMap["KEY_W"] = VirtualButton(Qt::Key_W, "w", Qt::Key_W, "W");
    m_KeyMap["KEY_X"] = VirtualButton(Qt::Key_X, "x", Qt::Key_X, "X");
    m_KeyMap["KEY_Y"] = VirtualButton(Qt::Key_Y, "y", Qt::Key_Y, "Y");
    m_KeyMap["KEY_Z"] = VirtualButton(Qt::Key_Z, "z", Qt::Key_Z, "Z");

    // rus
    m_AltKeyMap["KEY_BACKSPACE"] = VirtualButton(Qt::Key_Backspace, "", Qt::Key_Backspace, "");
    m_AltKeyMap["KEY_ENTER"] = VirtualButton(Qt::Key_Enter, "", Qt::Key_Enter, "");
    m_AltKeyMap["KEY_SHIFT"] = VirtualButton(Qt::Key_Shift, "", Qt::Key_Shift, "");
    m_AltKeyMap["KEY_SPACE"] = VirtualButton(Qt::Key_Space, " ", Qt::Key_Space, " ");
    m_AltKeyMap["KEY_SEMICOLON"] = VirtualButton(
        Qt::Key_Semicolon, QString::from_Utf8("ж"), Qt::Key_Colon, QString::from_Utf8("Ж"));
    m_AltKeyMap["KEY_APOSTROPHE"] = VirtualButton(
        Qt::Key_Apostrophe, QString::from_Utf8("э"), Qt::Key_QuoteDbl, QString::from_Utf8("Э"));
    m_AltKeyMap["KEY_MINUS"] = VirtualButton(
        Qt::Key_Minus, QString::from_Utf8("х"), Qt::Key_Underscore, QString::from_Utf8("Х"));
    m_AltKeyMap["KEY_EQUAL"] = VirtualButton(
        Qt::Key_Equal, QString::from_Utf8("ъ"), Qt::Key_Plus, QString::from_Utf8("Ъ"));
    m_AltKeyMap["KEY_COMMA"] = VirtualButton(
        Qt::Key_Comma, QString::from_Utf8("б"), Qt::Key_Less, QString::from_Utf8("Б"));
    m_AltKeyMap["KEY_PERIOD"] = VirtualButton(
        Qt::Key_Period, QString::from_Utf8("ю"), Qt::Key_Greater, QString::from_Utf8("Ю"));
    m_AltKeyMap["KEY_SLASH"] = VirtualButton(
        Qt::Key_Slash, QString::from_Utf8("."), Qt::Key_Question, QString::from_Utf8(","));
    m_AltKeyMap["KEY_BRACKETLEFT"] = VirtualButton(
        Qt::Key_BracketLeft, QString::from_Utf8("ш"), Qt::Key_BraceLeft, QString::from_Utf8("Ш"));
    m_AltKeyMap["KEY_BRACKETRIGHT"] = VirtualButton(
        Qt::Key_BracketRight, QString::from_Utf8("щ"), Qt::Key_BraceRight, QString::from_Utf8("Щ"));
    m_AltKeyMap["KEY_BACKSLASH"] = VirtualButton(
        Qt::Key_Backslash, QString::from_Utf8("з"), Qt::Key_Bar, QString::from_Utf8("З"));
    m_AltKeyMap["KEY_1"] = VirtualButton(Qt::Key_1, "1", Qt::Key_Exclam, "!");
    m_AltKeyMap["KEY_2"] = VirtualButton(Qt::Key_2, "2", Qt::Key_At, "\"");
    m_AltKeyMap["KEY_3"] =
        VirtualButton(Qt::Key_3, "3", Qt::Key_NumberSign, QString::from_Utf8("№"));
    m_AltKeyMap["KEY_4"] = VirtualButton(Qt::Key_4, "4", Qt::Key_Dollar, QString::from_Utf8(";"));
    m_AltKeyMap["KEY_5"] = VirtualButton(Qt::Key_5, "5", Qt::Key_Percent, "%");
    m_AltKeyMap["KEY_6"] = VirtualButton(Qt::Key_6, "6", Qt::Key_AsciiCircum, ":");
    m_AltKeyMap["KEY_7"] = VirtualButton(Qt::Key_7, "7", Qt::Key_Ampersand, "?");
    m_AltKeyMap["KEY_8"] = VirtualButton(Qt::Key_8, "8", Qt::Key_Asterisk, "*");
    m_AltKeyMap["KEY_9"] = VirtualButton(Qt::Key_9, "9", Qt::Key_ParenLeft, "(");
    m_AltKeyMap["KEY_0"] = VirtualButton(Qt::Key_0, "0", Qt::Key_ParenRight, ")");
    m_AltKeyMap["KEY_A"] =
        VirtualButton(Qt::Key_A, QString::from_Utf8("ф"), Qt::Key_A, QString::from_Utf8("Ф"));
    m_AltKeyMap["KEY_B"] =
        VirtualButton(Qt::Key_B, QString::from_Utf8("и"), Qt::Key_B, QString::from_Utf8("И"));
    m_AltKeyMap["KEY_C"] =
        VirtualButton(Qt::Key_C, QString::from_Utf8("с"), Qt::Key_C, QString::from_Utf8("С"));
    m_AltKeyMap["KEY_D"] =
        VirtualButton(Qt::Key_D, QString::from_Utf8("в"), Qt::Key_D, QString::from_Utf8("В"));
    m_AltKeyMap["KEY_E"] =
        VirtualButton(Qt::Key_E, QString::from_Utf8("у"), Qt::Key_E, QString::from_Utf8("У"));
    m_AltKeyMap["KEY_F"] =
        VirtualButton(Qt::Key_F, QString::from_Utf8("а"), Qt::Key_F, QString::from_Utf8("А"));
    m_AltKeyMap["KEY_G"] =
        VirtualButton(Qt::Key_G, QString::from_Utf8("п"), Qt::Key_G, QString::from_Utf8("П"));
    m_AltKeyMap["KEY_H"] =
        VirtualButton(Qt::Key_H, QString::from_Utf8("р"), Qt::Key_H, QString::from_Utf8("Р"));
    m_AltKeyMap["KEY_I"] =
        VirtualButton(Qt::Key_I, QString::from_Utf8("ш"), Qt::Key_I, QString::from_Utf8("Ш"));
    m_AltKeyMap["KEY_J"] =
        VirtualButton(Qt::Key_J, QString::from_Utf8("о"), Qt::Key_J, QString::from_Utf8("О"));
    m_AltKeyMap["KEY_K"] =
        VirtualButton(Qt::Key_K, QString::from_Utf8("л"), Qt::Key_K, QString::from_Utf8("Л"));
    m_AltKeyMap["KEY_L"] =
        VirtualButton(Qt::Key_L, QString::from_Utf8("д"), Qt::Key_L, QString::from_Utf8("Д"));
    m_AltKeyMap["KEY_M"] =
        VirtualButton(Qt::Key_M, QString::from_Utf8("ь"), Qt::Key_M, QString::from_Utf8("Ь"));
    m_AltKeyMap["KEY_N"] =
        VirtualButton(Qt::Key_N, QString::from_Utf8("т"), Qt::Key_N, QString::from_Utf8("Т"));
    m_AltKeyMap["KEY_O"] =
        VirtualButton(Qt::Key_O, QString::from_Utf8("щ"), Qt::Key_O, QString::from_Utf8("Щ"));
    m_AltKeyMap["KEY_P"] =
        VirtualButton(Qt::Key_P, QString::from_Utf8("з"), Qt::Key_P, QString::from_Utf8("З"));
    m_AltKeyMap["KEY_Q"] =
        VirtualButton(Qt::Key_Q, QString::from_Utf8("й"), Qt::Key_Q, QString::from_Utf8("Й"));
    m_AltKeyMap["KEY_R"] =
        VirtualButton(Qt::Key_R, QString::from_Utf8("к"), Qt::Key_R, QString::from_Utf8("К"));
    m_AltKeyMap["KEY_S"] =
        VirtualButton(Qt::Key_S, QString::from_Utf8("ы"), Qt::Key_S, QString::from_Utf8("Ы"));
    m_AltKeyMap["KEY_T"] =
        VirtualButton(Qt::Key_T, QString::from_Utf8("е"), Qt::Key_T, QString::from_Utf8("Е"));
    m_AltKeyMap["KEY_U"] =
        VirtualButton(Qt::Key_U, QString::from_Utf8("г"), Qt::Key_U, QString::from_Utf8("Г"));
    m_AltKeyMap["KEY_V"] =
        VirtualButton(Qt::Key_V, QString::from_Utf8("м"), Qt::Key_V, QString::from_Utf8("М"));
    m_AltKeyMap["KEY_W"] =
        VirtualButton(Qt::Key_W, QString::from_Utf8("ц"), Qt::Key_W, QString::from_Utf8("Ц"));
    m_AltKeyMap["KEY_X"] =
        VirtualButton(Qt::Key_X, QString::from_Utf8("ч"), Qt::Key_X, QString::from_Utf8("Ч"));
    m_AltKeyMap["KEY_Y"] =
        VirtualButton(Qt::Key_Y, QString::from_Utf8("н"), Qt::Key_Y, QString::from_Utf8("Н"));
    m_AltKeyMap["KEY_Z"] =
        VirtualButton(Qt::Key_Z, QString::from_Utf8("я"), Qt::Key_Z, QString::from_Utf8("Я"));

    updateKeys();
}

//---------------------------------------------------------------------------
void KeyboardWindow::shutdown() {}

//---------------------------------------------------------------------------
void KeyboardWindow::onButtonClicked() {
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    if (!button) {
        return;
    }

    QString buttonName = button->objectName();
    VirtualButton virtualButton = m_AltMode
                                      ? m_AltKeyMap.value(buttonName, m_KeyMap.value(buttonName))
                                      : m_KeyMap.value(buttonName);

    if (virtualButton.getKey(m_Shifted) != Qt::Key_unknown) {
        QKeyEvent *keyEvent =
            new QKeyEvent(QEvent::KeyPress, virtualButton.getKey(m_Shifted), Qt::NoModifier);
        QApplication::postEvent(QApplication::focusWidget(), keyEvent);
    }
}

//---------------------------------------------------------------------------
void KeyboardWindow::updateKeys() {
    QList<QToolButton *> padButtons = this->findChildren<QToolButton *>();
    foreach (QToolButton *button, padButtons) {
        QString buttonName = button->objectName();
        VirtualButton virtualButton =
            m_AltMode ? m_AltKeyMap.value(buttonName, m_KeyMap.value(buttonName))
                      : m_KeyMap.value(buttonName);

        button->setText(virtualButton.getText(m_Shifted));
    }
}

//---------------------------------------------------------------------------
void KeyboardWindow::mousePressEvent(QMouseEvent *aEvent) {
    QWidget::mousePressEvent(aEvent);
}

//---------------------------------------------------------------------------