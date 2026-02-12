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
KeyboardWindow::~KeyboardWindow() = default;

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
    m_KeyMap["KEY_B"] = VirtualButton(Qt::Key_A, "b", Qt::Key_A, "B");
    m_KeyMap["KEY_C"] = VirtualButton(Qt::Key_A, "c", Qt::Key_A, "C");
    m_KeyMap["KEY_D"] = VirtualButton(Qt::Key_A, "d", Qt::Key_A, "D");
    m_KeyMap["KEY_E"] = VirtualButton(Qt::Key_A, "e", Qt::Key_A, "E");
    m_KeyMap["KEY_F"] = VirtualButton(Qt::Key_A, "f", Qt::Key_A, "F");
    m_KeyMap["KEY_G"] = VirtualButton(Qt::Key_A, "g", Qt::Key_A, "G");
    m_KeyMap["KEY_H"] = VirtualButton(Qt::Key_A, "h", Qt::Key_A, "H");
    m_KeyMap["KEY_I"] = VirtualButton(Qt::Key_A, "i", Qt::Key_A, "I");
    m_KeyMap["KEY_J"] = VirtualButton(Qt::Key_A, "j", Qt::Key_A, "J");
    m_KeyMap["KEY_K"] = VirtualButton(Qt::Key_A, "k", Qt::Key_A, "K");
    m_KeyMap["KEY_L"] = VirtualButton(Qt::Key_A, "l", Qt::Key_A, "L");
    m_KeyMap["KEY_M"] = VirtualButton(Qt::Key_A, "m", Qt::Key_A, "M");
    m_KeyMap["KEY_N"] = VirtualButton(Qt::Key_A, "n", Qt::Key_A, "N");
    m_KeyMap["KEY_O"] = VirtualButton(Qt::Key_A, "o", Qt::Key_A, "O");
    m_KeyMap["KEY_P"] = VirtualButton(Qt::Key_A, "p", Qt::Key_A, "P");
    m_KeyMap["KEY_Q"] = VirtualButton(Qt::Key_A, "q", Qt::Key_A, "Q");
    m_KeyMap["KEY_R"] = VirtualButton(Qt::Key_A, "r", Qt::Key_A, "R");
    m_KeyMap["KEY_S"] = VirtualButton(Qt::Key_A, "s", Qt::Key_A, "S");
    m_KeyMap["KEY_T"] = VirtualButton(Qt::Key_A, "t", Qt::Key_A, "T");
    m_KeyMap["KEY_U"] = VirtualButton(Qt::Key_A, "u", Qt::Key_A, "U");
    m_KeyMap["KEY_V"] = VirtualButton(Qt::Key_A, "v", Qt::Key_A, "V");
    m_KeyMap["KEY_W"] = VirtualButton(Qt::Key_A, "w", Qt::Key_A, "W");
    m_KeyMap["KEY_X"] = VirtualButton(Qt::Key_A, "x", Qt::Key_A, "X");
    m_KeyMap["KEY_Y"] = VirtualButton(Qt::Key_A, "y", Qt::Key_A, "Y");
    m_KeyMap["KEY_Z"] = VirtualButton(Qt::Key_A, "z", Qt::Key_A, "Z");

    // rus
    m_AltKeyMap["KEY_BACKSPACE"] = VirtualButton(Qt::Key_Backspace, "", Qt::Key_Backspace, "");
    m_AltKeyMap["KEY_ENTER"] = VirtualButton(Qt::Key_Enter, "", Qt::Key_Enter, "");
    m_AltKeyMap["KEY_SHIFT"] = VirtualButton(Qt::Key_Shift, "", Qt::Key_Shift, "");
    m_AltKeyMap["KEY_SPACE"] = VirtualButton(Qt::Key_Space, " ", Qt::Key_Space, " ");
    m_AltKeyMap["KEY_SEMICOLON"] = VirtualButton(
        Qt::Key_Semicolon, QString::fromUtf8("ж"), Qt::Key_Colon, QString::fromUtf8("Ж"));
    m_AltKeyMap["KEY_APOSTROPHE"] = VirtualButton(
        Qt::Key_Apostrophe, QString::fromUtf8("э"), Qt::Key_QuoteDbl, QString::fromUtf8("Э"));
    m_AltKeyMap["KEY_MINUS"] = VirtualButton(Qt::Key_Minus, "-", Qt::Key_Underscore, "_");
    m_AltKeyMap["KEY_EQUAL"] = VirtualButton(Qt::Key_Equal, "=", Qt::Key_Plus, "+");
    m_AltKeyMap["KEY_COMMA"] =
        VirtualButton(Qt::Key_Comma, QString::fromUtf8("б"), Qt::Key_Less, QString::fromUtf8("Б"));
    m_AltKeyMap["KEY_PERIOD"] = VirtualButton(
        Qt::Key_Period, QString::fromUtf8("ю"), Qt::Key_Greater, QString::fromUtf8("Ю"));
    m_AltKeyMap["KEY_SLASH"] = VirtualButton(Qt::Key_Slash, "/", Qt::Key_Question, "?");
    m_AltKeyMap["KEY_BRACKETLEFT"] = VirtualButton(
        Qt::Key_BracketLeft, QString::fromUtf8("х"), Qt::Key_BraceLeft, QString::fromUtf8("Х"));
    m_AltKeyMap["KEY_BRACKETRIGHT"] = VirtualButton(
        Qt::Key_BracketRight, QString::fromUtf8("ъ"), Qt::Key_BraceRight, QString::fromUtf8("Ъ"));
    m_AltKeyMap["KEY_BACKSLASH"] = VirtualButton(Qt::Key_Backslash, "\\", Qt::Key_Bar, "|");
    m_AltKeyMap["KEY_1"] = VirtualButton(Qt::Key_1, "1", Qt::Key_Exclam, "!");
    m_AltKeyMap["KEY_2"] = VirtualButton(Qt::Key_2, "2", Qt::Key_At, "@");
    m_AltKeyMap["KEY_3"] = VirtualButton(Qt::Key_3, "3", Qt::Key_NumberSign, "#");
    m_AltKeyMap["KEY_4"] = VirtualButton(Qt::Key_4, "4", Qt::Key_Dollar, "$");
    m_AltKeyMap["KEY_5"] = VirtualButton(Qt::Key_5, "5", Qt::Key_Percent, "%");
    m_AltKeyMap["KEY_6"] = VirtualButton(Qt::Key_6, "6", Qt::Key_AsciiCircum, "^");
    m_AltKeyMap["KEY_7"] = VirtualButton(Qt::Key_7, "7", Qt::Key_Ampersand, "&");
    m_AltKeyMap["KEY_8"] = VirtualButton(Qt::Key_8, "8", Qt::Key_Asterisk, "*");
    m_AltKeyMap["KEY_9"] = VirtualButton(Qt::Key_9, "9", Qt::Key_ParenLeft, "(");
    m_AltKeyMap["KEY_0"] = VirtualButton(Qt::Key_0, "0", Qt::Key_ParenRight, ")");
    m_AltKeyMap["KEY_A"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("ф"), Qt::Key_A, QString::fromUtf8("Ф"));
    m_AltKeyMap["KEY_B"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("и"), Qt::Key_A, QString::fromUtf8("И"));
    m_AltKeyMap["KEY_C"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("с"), Qt::Key_A, QString::fromUtf8("С"));
    m_AltKeyMap["KEY_D"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("в"), Qt::Key_A, QString::fromUtf8("В"));
    m_AltKeyMap["KEY_E"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("у"), Qt::Key_A, QString::fromUtf8("У"));
    m_AltKeyMap["KEY_F"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("а"), Qt::Key_A, QString::fromUtf8("А"));
    m_AltKeyMap["KEY_G"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("п"), Qt::Key_A, QString::fromUtf8("П"));
    m_AltKeyMap["KEY_H"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("р"), Qt::Key_A, QString::fromUtf8("Р"));
    m_AltKeyMap["KEY_I"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("ш"), Qt::Key_A, QString::fromUtf8("Ш"));
    m_AltKeyMap["KEY_J"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("о"), Qt::Key_A, QString::fromUtf8("О"));
    m_AltKeyMap["KEY_K"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("л"), Qt::Key_A, QString::fromUtf8("Л"));
    m_AltKeyMap["KEY_L"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("д"), Qt::Key_A, QString::fromUtf8("Д"));
    m_AltKeyMap["KEY_M"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("ь"), Qt::Key_A, QString::fromUtf8("Ь"));
    m_AltKeyMap["KEY_N"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("т"), Qt::Key_A, QString::fromUtf8("Т"));
    m_AltKeyMap["KEY_O"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("щ"), Qt::Key_A, QString::fromUtf8("Щ"));
    m_AltKeyMap["KEY_P"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("з"), Qt::Key_A, QString::fromUtf8("З"));
    m_AltKeyMap["KEY_Q"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("й"), Qt::Key_A, QString::fromUtf8("Й"));
    m_AltKeyMap["KEY_R"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("к"), Qt::Key_A, QString::fromUtf8("К"));
    m_AltKeyMap["KEY_S"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("ы"), Qt::Key_A, QString::fromUtf8("Ы"));
    m_AltKeyMap["KEY_T"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("е"), Qt::Key_A, QString::fromUtf8("Е"));
    m_AltKeyMap["KEY_U"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("г"), Qt::Key_A, QString::fromUtf8("Г"));
    m_AltKeyMap["KEY_V"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("м"), Qt::Key_A, QString::fromUtf8("М"));
    m_AltKeyMap["KEY_W"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("ц"), Qt::Key_A, QString::fromUtf8("Ц"));
    m_AltKeyMap["KEY_X"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("ч"), Qt::Key_A, QString::fromUtf8("Ч"));
    m_AltKeyMap["KEY_Y"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("н"), Qt::Key_A, QString::fromUtf8("Н"));
    m_AltKeyMap["KEY_Z"] =
        VirtualButton(Qt::Key_A, QString::fromUtf8("я"), Qt::Key_A, QString::fromUtf8("Я"));

    updateKeys();
}

//---------------------------------------------------------------------------
void KeyboardWindow::shutdown() {}

//---------------------------------------------------------------------------
void KeyboardWindow::onButtonClicked() {
    if (sender()->objectName() == "KEY_ENTER") {
        hide();
    }

    VirtualButton button =
        m_AltMode ? m_AltKeyMap[sender()->objectName()] : m_KeyMap[sender()->objectName()];

    QKeyEvent keyPress(QEvent::KeyPress,
                       button.getKey(m_Shifted),
                       m_Shifted ? Qt::ShiftModifier : Qt::NoModifier,
                       button.getText(m_Shifted));
    QApplication::sendEvent(QApplication::focusWidget(), &keyPress);
}

//---------------------------------------------------------------------------
void KeyboardWindow::updateKeys() {
    QList<QToolButton *> padButtons = this->findChildren<QToolButton *>();
    foreach (QToolButton *button, padButtons) {
        if (button->objectName() == "KEY_LANG") {
            button->setText(m_AltMode ? "LAT" : "RUS");
        } else if (button->objectName() == "KEY_7" && m_Shifted) {
            button->setText("&&");
        } else {
            button->setText(
                (m_AltMode ? m_AltKeyMap[button->objectName()] : m_KeyMap[button->objectName()])
                    .getText(m_Shifted));
        }
    }
}

//---------------------------------------------------------------------------
void KeyboardWindow::mousePressEvent(QMouseEvent * /*aEvent*/) {
    // Блокируем дальнейшее прохождение кликов
}

//---------------------------------------------------------------------------
