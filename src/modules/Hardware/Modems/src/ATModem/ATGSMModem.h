/* @file AT-совместимый модем. */

#pragma once

#include <QtCore/QCoreApplication>

#include "ATData.h"
#include "ATModem_Base.h"

//--------------------------------------------------------------------------------
namespace ENetworkAccessability {
enum Enum {
    NotRegistered,
    RegisteredHomeNetwork,
    SearchingOperator,
    RegistrationDenied,
    Unknown,
    RegisteredRoaming
};
} // namespace ENetworkAccessability

//--------------------------------------------------------------------------------
class ATGSMModem : public ATModem_Base {
public:
    ATGSMModem();

#pragma region IDevice interface
    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);
#pragma endregion

#pragma region IModem interface
    /// Сброс.
    virtual bool reset();

    /// Получение оператора.
    virtual bool getOperator(QString &aOperator);

    /// Получение качество сигнала.
    virtual bool getSignalQuality(int &aSignalQuality);

    /// Получение информации об устройстве
    virtual bool getInfo(QString &aInfo);

    /// Выполнить USSD-запрос.
    virtual bool processUSSD(const QString &aMessage, QString &aAnswer);

    /// Послать SMS.
    virtual bool sendMessage(const QString &aPhone, const QString &aMessage);

    /// Забрать из модема полученные SMS
    virtual bool takeMessages(TMessages &aMessages);
#pragma endregion

protected:
    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Получить состояние регистрации в сети
    static bool getNetworkAccessability(ENetworkAccessability::Enum &aNetworkAccessability);

    /// Функция ожидания доступности GSM сети
    static bool waitNetworkAccessability(int aTimeout);

    /// Декодирование ответа +CUSD команды
    bool getCUSDMessage(const QByteArray &aBuffer, QString &aMessage);

    /// Разбор имени модема
    virtual void setDeviceName(const QByteArray &aFullName);

    /// Получить инфо о SIM-карте
    static void getSIMData(const QByteArray &aCommand);

    static bool
    parseFieldInternal(const QByteArray &aBuffer, const QString &aFieldName, QString &aValue);
    static bool getSiemensCellList(QString &aValue);
    static bool getSim_COMCellList(QString &aValue);

    /// Определение диалекта модема
    AT::EModem_Dialect::Enum m_Gsm_Dialect;
};

//--------------------------------------------------------------------------------
