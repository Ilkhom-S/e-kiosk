/* @file Менеджер для работы с ключами */

#include "KeysManager.h"

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <Crypt/ICryptEngine.h>

#include "GUI/ServiceTags.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
KeysManager::KeysManager(SDK::PaymentProcessor::ICore *aCore)
    : m_Core(aCore), m_IsGenerated(false) {
    m_CryptService = m_Core->getCryptService();
    m_TerminalSettings = static_cast<PPSDK::TerminalSettings *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
}

//------------------------------------------------------------------------
KeysManager::~KeysManager() {}

//------------------------------------------------------------------------
bool KeysManager::generateKey(QVariantMap &aKeysParam) {
    QString login(aKeysParam[CServiceTags::Login].toString());
    QString password(aKeysParam[CServiceTags::Password].toString());
    QString description(aKeysParam[CServiceTags::KeyPairDescription].toString());
    QString keyPairNumber(aKeysParam[CServiceTags::KeyPairNumber].toString());
    QString url(m_TerminalSettings->getKeygenURL());

    if (url.isEmpty()) {
        aKeysParam[CServiceTags::Error] = "#url_is_empty";
        m_IsGenerated = false;
    }

    EKeysUtilsError::Enum result = static_cast<EKeysUtilsError::Enum>(m_CryptService->generateKey(
        keyPairNumber.toInt(), login, password, url, m_SD, m_AP, m_OP, description));

    aKeysParam[CServiceTags::Error] = errorToString(result);

    m_IsGenerated = result == EKeysUtilsError::Ok;

    return m_IsGenerated;
}

//------------------------------------------------------------------------
bool KeysManager::formatToken() {
    auto status = tokenStatus();

    if (status.available) {
        if (!status.initialized) {
            return m_CryptService->getCryptEngine()->initializeToken(CCrypt::ETypeEngine::RuToken);
        }

        return true;
    }

    return false;
}

//------------------------------------------------------------------------
QList<int> KeysManager::getLoadedKeys() const {
    return m_CryptService->getLoadedKeys();
}

//------------------------------------------------------------------------
bool KeysManager::isDefaultKeyOP(const QString &aOP) {
    PPSDK::ICryptService::SKeyInfo key = m_CryptService->getKeyInfo(0);

    return key.isValid() && key.op == aOP;
}

//------------------------------------------------------------------------
CCrypt::TokenStatus KeysManager::tokenStatus() const {
    return m_CryptService->getCryptEngine()->getTokenStatus(CCrypt::ETypeEngine::RuToken);
}

//------------------------------------------------------------------------
bool KeysManager::saveKey() {
    return m_CryptService->saveKey();
}

//------------------------------------------------------------------------
bool KeysManager::allowAnyKeyPair() const {
    return m_TerminalSettings->getServiceMenuSettings().allowAnyKeyPair;
}

//------------------------------------------------------------------------
bool KeysManager::isConfigurationChanged() const {
    return m_IsGenerated;
}

//------------------------------------------------------------------------
void KeysManager::resetConfiguration() {
    m_IsGenerated = false;
}

//------------------------------------------------------------------------
QString KeysManager::getSD() const {
    return m_SD;
}

//------------------------------------------------------------------------
QString KeysManager::getAP() const {
    return m_AP;
}

//------------------------------------------------------------------------
QString KeysManager::getOP() const {
    return m_OP;
}

//------------------------------------------------------------------------
QString KeysManager::errorToString(EKeysUtilsError::Enum aCode) const {
    switch (aCode) {
    case EKeysUtilsError::Ok:
        return tr("#ok");
    case EKeysUtilsError::NetworkError:
        return tr("#network_error");
    case EKeysUtilsError::WrongPassword:
        return tr("#wrong_login_or_password");
    case EKeysUtilsError::WrongServerAnswer:
        return tr("#wrong_server_answer");
    case EKeysUtilsError::WrongLocalTime:
        return tr("#wrong_local_time");
    case EKeysUtilsError::UnknownServerError:
        return tr("#unknown_server_error");
    case EKeysUtilsError::KeyPairCreateError:
        return tr("#key_pair_create_error");
    case EKeysUtilsError::KeyExportError:
        return tr("#key_export_error");
    }

    return tr("#unknown_error");
}

//------------------------------------------------------------------------
