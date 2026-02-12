/* @file Менеджер для работы с сетью */

#include "NetworkManager.h"

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <Connection/IConnection.h>

#include "GUI/MessageBox/MessageBox.h"
#include "GUI/ServiceTags.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
NetworkManager::NetworkManager(SDK::PaymentProcessor::ICore *aCore)
    : m_Core(aCore), m_NetworkService(m_Core->getNetworkService()),
      m_SelectedConnection(m_InitialConnection) {

    m_TerminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
    m_Directory = dynamic_cast<PPSDK::Directory *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::Directory));

    m_InitialConnection = m_NetworkService->getConnection();
}

//---------------------------------------------------------------------------
NetworkManager::~NetworkManager() = default;

//---------------------------------------------------------------------------
bool NetworkManager::isConfigurationChanged() const {
    return !(m_SelectedConnection == m_InitialConnection);
}

//------------------------------------------------------------------------
void NetworkManager::resetConfiguration() {
    m_InitialConnection = m_SelectedConnection;
}

//---------------------------------------------------------------------------
bool NetworkManager::openConnection(bool aWait) {
    return m_NetworkService->openConnection(aWait);
}

//---------------------------------------------------------------------------
bool NetworkManager::closeConnection() {
    return m_NetworkService->closeConnection();
}

//---------------------------------------------------------------------------
bool NetworkManager::isConnected(bool aUseCache) const {
    return m_NetworkService->isConnected(aUseCache);
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::SConnection NetworkManager::getConnection() const {
    return m_NetworkService->getConnection();
}

//---------------------------------------------------------------------------
void NetworkManager::setConnection(const SDK::PaymentProcessor::SConnection &aConnection) {
    m_SelectedConnection = aConnection;
    m_NetworkService->setConnection(aConnection);
    m_TerminalSettings->setConnection(aConnection);
}

//---------------------------------------------------------------------------
bool NetworkManager::testConnection(QString &aErrorMessage) {
    bool result = m_NetworkService->testConnection();
    aErrorMessage = m_NetworkService->getLastConnectionError().split(":").last();
    return result;
}

//---------------------------------------------------------------------------
QList<QPair<QString, QString>> NetworkManager::getModems() const {
    QList<QPair<QString, QString>> result;

    try {
        foreach (auto modem, IConnection::getModems()) {
            result << QPair<QString, QString>(modem, IConnection::getModem_Info(modem));
        }
    } catch (const NetworkError &) {
        GUI::MessageBox::critical(tr("#get_modems_error"));
    }

    return result;
}

//---------------------------------------------------------------------------
QStringList NetworkManager::getInterfaces() const {
    try {
        return IConnection::getInterfaces();
    } catch (const NetworkError &) {
        GUI::MessageBox::critical(tr("#get_interface_error"));
    }

    return {};
}

//---------------------------------------------------------------------------
QStringList NetworkManager::getRemoteConnections() const {
    try {
        return IConnection::getRemoteConnections();
    } catch (const NetworkError &) {
        GUI::MessageBox::critical(tr("#get_remote_connections_error"));
    }

    return {};
}

//---------------------------------------------------------------------------
QStringList NetworkManager::getLocalConnections() const {
    try {
        return IConnection::getLocalConnections();
    } catch (const NetworkError &) {
        GUI::MessageBox::critical(tr("#get_local_connections_error"));
    }

    return {};
}

//---------------------------------------------------------------------------
QStringList NetworkManager::getConnectionTemplates() const {
    QStringList nameList;

    foreach (SDK::PaymentProcessor::SConnectionTemplate dialupTemplate,
             m_Directory->getConnectionTemplates()) {
        nameList << dialupTemplate.name;
    }

    return nameList;
}

//---------------------------------------------------------------------------
bool NetworkManager::createDialupConnection(const SDK::PaymentProcessor::SConnection &aConnection,
                                            const QString &aNetworkDevice) {
    foreach (PPSDK::SConnectionTemplate connection, m_Directory->getConnectionTemplates()) {
        if (connection.name == aConnection.name) {
            try {
                IConnection::createDialupConnection(connection.name,
                                                    connection.phone,
                                                    connection.login,
                                                    connection.password,
                                                    aNetworkDevice);
                return true;
            } catch (const NetworkError &e) {
                GUI::MessageBox::critical(e.getSeverity() == ESeverity::Critical
                                              ? tr("#phone_entry_call_failed")
                                              : tr("#phone_entry_already_exist\n"));
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------
bool NetworkManager::removeDialupConnection(const SDK::PaymentProcessor::SConnection &aConnection) {
    m_NetworkService->closeConnection();

    foreach (PPSDK::SConnectionTemplate connection, m_Directory->getConnectionTemplates()) {
        if (connection.name == aConnection.name) {
            try {
                IConnection::removeDialupConnection(connection.name);
                return true;
            } catch (const NetworkError &e) {
                GUI::MessageBox::critical(e.getSeverity() == ESeverity::Critical
                                              ? tr("#phone_entry_call_failed")
                                              : tr("#phone_entry_not_exist\n"));
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------
void NetworkManager::getNetworkInfo(QVariantMap &aResult) const {
    aResult[CServiceTags::Connection] = m_NetworkService->getConnection().name;
    aResult[CServiceTags::ConnectionType] = m_NetworkService->getConnection().type;
    aResult[CServiceTags::CheckInterval] = m_NetworkService->getConnection().checkInterval;
    aResult[CServiceTags::ConnectionStatus] = m_NetworkService->isConnected();

    QNetworkProxy proxy = m_NetworkService->getConnection().proxy;

    aResult[CServiceTags::ProxyType] = proxy.type();

    if (proxy.type() != QNetworkProxy::NoProxy) {
        aResult[CServiceTags::ProxyAddress] = proxy.hostName();
        aResult[CServiceTags::ProxyPort] = proxy.port();
        aResult[CServiceTags::ProxyUser] = proxy.user();
        aResult[CServiceTags::ProxyPassword] = proxy.password();
    }

    aResult.insert(dynamic_cast<PPSDK::IService *>(m_NetworkService)->getParameters());
}

//---------------------------------------------------------------------------
