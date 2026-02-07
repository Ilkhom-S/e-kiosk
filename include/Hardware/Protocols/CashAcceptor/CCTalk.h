/* @file Протокол ccTalk. */

#pragma once

#include <Hardware/CashDevices/CCTalkData.h>
#include <Hardware/Common/ProtocolBase.h>

//--------------------------------------------------------------------------------
namespace CCCTalk {
/// Неизвестный тип протокола.
extern const char UnknownType[];
} // namespace CCCTalk

//--------------------------------------------------------------------------------
class CCTalkCAProtocol : public ProtocolBase {
public:
    CCTalkCAProtocol();

    /// Установить тип CRC.
    void setType(const QString &aType);

    /// Установить адрес slave-устройства.
    void setAddress(uchar aAddress);

    /// Выполнить команду.
    TResult processCommand(const QByteArray &aCommandData, QByteArray &aAnswerData);

protected:
    /// Проверить валидность ответа.
    bool check(QByteArray &aAnswer);

    /// Получить пакет данных из порта.
    bool getAnswer(QByteArray &aAnswer, const QByteArray &aCommandData);

    /// Вычислить контрольную сумму пакета данных.
    static uchar calcCRC8(const QByteArray &aData);
    ushort calcCRC16(const QByteArray &aData);

    /// Адрес устройства.
    uchar mAddress;

    /// тип CRC.
    QString mType;
};

//--------------------------------------------------------------------------------
