/* @file Интерфейс модема. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QList>

#include <SDK/Drivers/IDevice.h>

namespace SDK {
namespace Driver {

namespace GSM {
/// Описание структуры полученного SMS сообщения
struct SSMS {
    QString from;   /// Отправитель
    QDateTime date; /// Время получения
    QString text;   /// Содержимое SMS
};
} // namespace GSM

//--------------------------------------------------------------------------------
class IModem : public IDevice {
public:
    /// Сброс.
    virtual bool reset() = 0;

    /// Устанавливает строку инициализации.
    virtual bool setInitString(const QString &aInitString) = 0;

    /// Получение оператора.
    virtual bool getOperator(QString &aOperator) = 0;

    /// Получение качество сигнала.
    virtual bool getSignalQuality(int &aSignalQuality) = 0;

    /// Получение информации о модеме (sim карте)
    virtual bool getInfo(QString &aInfo) = 0;

    /// Выполнить USSD-запрос.
    virtual bool processUSSD(const QString &aMessage, QString &aReply) = 0;

    /// Послать SMS.
    virtual bool sendMessage(const QString &aPhone, const QString &aMessage) = 0;

    /// Получить все SMS
    /// Сообщения.
    typedef QList<GSM::SSMS> TMessages;
    virtual bool takeMessages(TMessages &aMessages) = 0;

protected:
    virtual ~IModem() {}
};

} // namespace Driver
} // namespace SDK

//--------------------------------------------------------------------------------
