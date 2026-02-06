/* @file Параметры для диалоговых окон; регистрируются в QML */

#pragma once

#include <QtCore/QObject>

namespace SDK {
namespace GUI {

//------------------------------------------------------------------------------
/// Константы для параметров диалоговых окон.
namespace CMessageBox {
const QString SceneName = "MessageBox";
const QString Popup = "popup";
const QString Icon = "icon";
const QString Button = "button";
const QString ButtonType = "button_type";
const QString ButtonText = "button_text";
const QString TextMessage = "text_message";
const QString TextMessageExt = "text_message_ext";
const QString TextAppendMode = "text_append_mode";
const QString Scaled = "scaled";
const QString Image = "image";
} // namespace CMessageBox

//------------------------------------------------------------------------------
/// Параметры для диалоговых окон.
class MessageBoxParams : public QObject {
    Q_OBJECT

    Q_ENUMS(Enum);
    Q_PROPERTY(QString Icon READ getIcon CONSTANT)
    Q_PROPERTY(QString Text READ getText CONSTANT)
    Q_PROPERTY(QString Button READ getButton CONSTANT)

public:
    /// Перечисление для типов кнопок и иконок.
    enum Enum { NoButton, OK, Cancel, NoIcon, Info, Warning, Critical, Wait, Question, Text };

public:
    /// Возвращает имя свойства иконки.
    QString getIcon() const { return CMessageBox::Icon; }
    /// Возвращает имя свойства текста.
    QString getText() const { return CMessageBox::TextMessage; }
    /// Возвращает имя свойства кнопки.
    QString getButton() const { return CMessageBox::Button; }
};

} // namespace GUI
} // namespace SDK

Q_DECLARE_METATYPE(SDK::GUI::MessageBoxParams::Enum)

//---------------------------------------------------------------------------
