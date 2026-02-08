/* @file Реализация dialup-соединения. */

#include "DialupConnection.h"

#include <Connection/NetworkError.h>

#include "RasWrapper.h"

//--------------------------------------------------------------------------------
DialupConnection::DialupConnection(const QString &aName, NetworkTaskManager *aNetwork, ILog *aLog)
    : ConnectionBase(aName, aNetwork, aLog) {}

//--------------------------------------------------------------------------------
DialupConnection::~DialupConnection() {
    try {
        close();
    } catch (...) {
        toLog(LogLevel::Fatal,
              "DialupConnection: An exception occurred while close network connection.");
    }
}

//--------------------------------------------------------------------------------
EConnectionTypes::Enum DialupConnection::getType() const {
    return EConnectionTypes::Dialup;
}

//--------------------------------------------------------------------------------
void DialupConnection::doConnect() throw(...) {
    RasApi::PhonebookEntryName entryName;
    entryName.setName(m_Name.toStdWString());

    RasApi::PhonebookEntry entry;
    if (RasApi::GetEntryProperties(entryName, entry) == 0) {
        toLog(LogLevel::Normal,
              QString("Device name:    %1 (%2)")
                  .arg(QString::from_StdWString(entry.deviceName()))
                  .arg(QString::from_StdWString(RasApi::getAttachedTo(entry.deviceName()))));
        toLog(LogLevel::Normal,
              QString("Phone number:   %1").arg(QString::from_StdWString(entry.localPhoneNumber())));
        toLog(LogLevel::Normal, QString("*").repeated(40));
    }

    // TODO: probably need to add win2000 phonebook path workaround

    DWORD raserror = RasApi::Dial(entryName);

    if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY) {
        throw NetworkError(
            ECategory::Network,
            ESeverity::Critical,
            raserror,
            QString("RasApi: Dial failed because RPC server is busy or unavailable (%1)")
                .arg(raserror));
    } else if (raserror != ERROR_SUCCESS) {
        throw NetworkError(ECategory::Network,
                           ESeverity::Major,
                           raserror,
                           QString("RasApi: failed to dial '%1': %2 (%3)")
                               .arg(m_Name)
                               .arg(QString::from_StdWString(RasApi::EErrorCode::ToString(raserror)))
                               .arg(raserror));
    }
}

//--------------------------------------------------------------------------------
void DialupConnection::doDisconnect() throw(...) {
    DWORD raserror = RasApi::HangUp(m_Name.toStdWString());

    if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY) {
        throw NetworkError(
            ECategory::Network,
            ESeverity::Critical,
            raserror,
            QString("RasApi: HangUp failed because RPC server is busy or unavailable (%1)")
                .arg(raserror));
    } else if (raserror != ERROR_SUCCESS) {
        throw NetworkError(ECategory::Network,
                           ESeverity::Major,
                           raserror,
                           QString("RasApi: failed to dial '%1': %2 (%3)")
                               .arg(m_Name)
                               .arg(QString::from_StdWString(RasApi::EErrorCode::ToString(raserror)))
                               .arg(raserror));
    }
}

//--------------------------------------------------------------------------------
bool DialupConnection::doIsConnected() throw(...) {
    RasApi::EConnectionStatus::Enum status = RasApi::EConnectionStatus::Disconnected;

    DWORD raserror = RasApi::GetConnectionStatus(m_Name.toStdWString(), status);

    if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY) {
        throw NetworkError(
            ECategory::Network,
            ESeverity::Critical,
            raserror,
            QString(
                "RasApi: GetConnectionStatus failed because RPC server is busy or unavailable (%1)")
                .arg(raserror));
    } else if (raserror != ERROR_SUCCESS) {
        toLog(LogLevel::Error, QString("GetConnectionStatus for '%1' failed.").arg(m_Name));
        return false;
    }

    toLog(LogLevel::Debug, QString("RAS connection status: %1").arg(status));

    return status == RasApi::EConnectionStatus::Connected;
}

//--------------------------------------------------------------------------------
bool DialupConnection::doCheckConnection(const CheckUrl &aHost) {
    return httpCheckMethod(aHost);
}

//----------------------------------------------------------------------------
