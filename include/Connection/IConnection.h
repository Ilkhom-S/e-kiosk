/* @file Интерфейс сетевого соединения. */

#pragma once

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QPair>
#include <QtCore/QDateTime>
#include "Common/QtHeadersEnd.h"

#include <Common/ILog.h>

// SDK
#include <SDK/PaymentProcessor/Connection/ConnectionTypes.h>

// Проект
#include "NetworkError.h"

class NetworkTaskManager;

//--------------------------------------------------------------------------------
/// Интерфейс сетевого соединения.
class IConnection : public QObject
{
    Q_OBJECT

  public:
    /// Структура для проверки связи <url, response string>
    typedef QPair<QUrl, QString> CheckUrl;

  public:
    /// Создать экземпляр соединения.
    static IConnection *create(const QString &aName, EConnectionTypes::Enum aType, NetworkTaskManager *aNetwork,
                               ILog *aLog);

    /// Деструктор.
    virtual ~IConnection()
    {
    }

    /// Возвращает имя соединения.
    virtual QString getName() const = 0;

    /// Возвращает тип соединения.
    virtual EConnectionTypes::Enum getType() const = 0;

    ///	Устанавливает период проверки соединения.
    virtual void setCheckPeriod(int aMinutes) = 0;

    /// Проверяет установленно ли подключение.
    virtual bool isConnected(bool aUseCache = true) noexcept(false) = 0;

    /// Проверяет соединение с хостом.
    virtual bool checkConnection(const CheckUrl &aHost = CheckUrl()) noexcept(false) = 0;

    /// Устанавливает соединение.
    virtual void open(bool aWatch = true) noexcept(false) = 0;

    /// Закрывает соединение.
    virtual void close() noexcept(false) = 0;

    /// Устанавливает список хостов для проверки соединения.
    virtual void setCheckHosts(const QList<CheckUrl> &aHosts) = 0;

  signals:
    /// При закрытии/обрыве соединения.
    void connectionLost();

    /// Сигнал об успешной проверке соединения
    void connectionAlive();

  public:
    /// Поиск всех установленных в системе модемов.
    static QStringList getModems() noexcept(false);

    /// Получить информацию о модеме
    static QString getModemInfo(const QString &aName) noexcept(false);

    /// Поиск всех установленных в системе сетевых интерфейсов.
    static QStringList getInterfaces() noexcept(false);

    /// Список всех соединений в системе.
    static QStringList getRemoteConnections() noexcept(false);

    /// Список всех локальных соединений в системе.
    static QStringList getLocalConnections() noexcept(false);

    /// Создать dialup соединение
    static void createDialupConnection(const QString &aName, const QString &aPhone, const QString &aLogin,
                                       const QString &aPassword, const QString &aDevice) noexcept(false);

    /// Удалить dialup соединение
    static void removeDialupConnection(const QString &aName) noexcept(false);

  public:
    static ILog *mLog;
};

//--------------------------------------------------------------------------------
