/* @file Менеджер для работы с сетью */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

#include <SDK/PaymentProcessor/Connection/Connection.h>

#include "IConfigManager.h"

namespace SDK {
namespace PaymentProcessor {
class ICore;
class INetworkService;
class TerminalSettings;
class Directory;
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
class NetworkManager : public QObject, public IConfigManager {
    Q_OBJECT

public:
    NetworkManager(SDK::PaymentProcessor::ICore *aCore);
    ~NetworkManager();

public:
    /// Настройки соединения изменились?
    virtual bool isConfigurationChanged() const;

    /// Делаем текущую конфигурацию начальной
    virtual void resetConfiguration();

public:
    /// Устанавливает соединение.
    bool openConnection(bool aWait = false);

    /// Разрывает соединение.
    bool closeConnection();

    /// Проверяет установленно ли соединение.
    bool isConnected(bool aUseCache = false) const;

    /// Возвращает параметры активного соединения.
    SDK::PaymentProcessor::SConnection getConnection() const;

    /// Сохраняет настройки соединения в памяти
    void setConnection(const SDK::PaymentProcessor::SConnection &aConnection);

    /// Тестирует соединение: устанавливает, проверяет доступность ресурса aHost, разрывает.
    bool testConnection(QString &aErrorMessage);

    /// Поиск всех установленных в системе модемов.
    QList<QPair<QString, QString>> getModems() const;

    /// Поиск всех установленных в системе сетевых интерфейсов.
    QStringList getInterfaces() const;

    /// Список всех соединений в системе.
    QStringList getRemoteConnections() const;

    /// Список всех локальных соединений в системе.
    QStringList getLocalConnections() const;

    /// Получить имена шаблонов соединений
    QStringList getConnectionTemplates() const;

    /// Создать dialup соединение
    bool createDialupConnection(const SDK::PaymentProcessor::SConnection &aConnection,
                                const QString &aNetworkDevice);

    /// Удалить dialup соединение
    bool removeDialupConnection(const SDK::PaymentProcessor::SConnection &aConnection);

    void getNetworkInfo(QVariantMap &aResult) const;

private:
    SDK::PaymentProcessor::ICore *m_Core;
    SDK::PaymentProcessor::INetworkService *m_NetworkService;
    SDK::PaymentProcessor::TerminalSettings *m_TerminalSettings;
    SDK::PaymentProcessor::Directory *m_Directory;

    SDK::PaymentProcessor::SConnection m_InitialConnection;
    SDK::PaymentProcessor::SConnection m_SelectedConnection;
};

//---------------------------------------------------------------------------
