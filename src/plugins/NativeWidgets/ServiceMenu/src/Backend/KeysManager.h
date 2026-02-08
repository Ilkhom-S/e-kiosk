/* @file Менеджер для работы с ключами */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include <Crypt/ICryptEngine.h>
#include <KeysUtils/KeysUtils.h>

#include "IConfigManager.h"

namespace SDK {
namespace PaymentProcessor {
class ICore;
class ICryptService;
class TerminalSettings;
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
class KeysManager : public QObject, public IConfigManager {
    Q_OBJECT

public:
    KeysManager(SDK::PaymentProcessor::ICore *aCore);
    ~KeysManager();

public:
    /// Ключи создались?
    virtual bool isConfigurationChanged() const;

    /// Делаем текущую конфигурацию начальной
    virtual void resetConfiguration();

public:
    /// Получение информации о eToken
    CCrypt::TokenStatus tokenStatus() const;

    /// Форматирование ключа eToken
    bool formatToken();

public:
    /// Возвращает загруженные номера пар ключей
    QList<int> getLoadedKeys() const;

    /// Проверяет принадлежит ли Код Оператора нулевой паре ключей
    bool isDefaultKeyOP(const QString &aOP);

    /// Генерирует и регистрирует ключ на сервере.
    bool generateKey(QVariantMap &aKeysParam);

    /// Сохраняет сгенерированный ключ.
    bool saveKey();

    /// Возвращает разрешение на создание ненулевых пар ключей
    bool allowAnyKeyPair() const;

    /// Параметры, получаемые от ICryptService в случае успешной регистрации ключей
    QString getSD() const;
    QString getAP() const;
    QString getOP() const;

private:
    static QString errorToString(EKeysUtilsError::Enum aCode);

private:
    SDK::PaymentProcessor::ICore *m_Core;
    SDK::PaymentProcessor::ICryptService *m_CryptService;
    SDK::PaymentProcessor::TerminalSettings *m_TerminalSettings;

    QString m_SD;
    QString m_AP;
    QString m_OP;

    bool m_IsGenerated;
};

//---------------------------------------------------------------------------
