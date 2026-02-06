/* @file Реализация базового функционала сетевого соединения. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include <Common/ExceptionFilter.h>
#include <Common/ILogable.h>

#include "Connection/IConnection.h"

//--------------------------------------------------------------------------------
/// Константы
namespace CConnection {
/// Имя лога.
const QString LogName = "Connection";
} // namespace CConnection

//--------------------------------------------------------------------------------
/// Базовый класс соединения.
/// Управляет таймерами проверки соединения и реализует метод проверки
/// соединения по HTTP.
class ConnectionBase : public IConnection {
    Q_OBJECT

public:
    /// Конструктор.
    ConnectionBase(const QString &aName, NetworkTaskManager *aNetwork, ILog *aLog);

    /// Возвращает имя соединения.
    virtual QString getName() const;

    ///	Устанавливает период проверки соединения.
    virtual void setCheckPeriod(int aMilliseconds);

    /// Осуществить попытку поднять соединение.
    virtual void open(bool aWatch = true) noexcept(false);

    /// Осуществить попытку закрыть соединение.
    virtual void close() noexcept(false);

    /// Проверяет установленно ли соединение.
    virtual bool isConnected(bool aUseCache) noexcept(false);

    /// Физически проверяет соединение выполняя HTTP запрос.
    virtual bool checkConnection(const CheckUrl &aHost = CheckUrl()) noexcept(false);

    /// Устанавливает список хостов для проверки соединения.
    virtual void setCheckHosts(const QList<IConnection::CheckUrl> &aHosts);

protected slots:
    void onCheckTimeout();

protected:
    virtual void doConnect() noexcept(false) = 0;
    virtual void doDisconnect() noexcept(false) = 0;
    virtual bool doIsConnected() noexcept(false) = 0;
    virtual bool doCheckConnection(const CheckUrl &aHost = CheckUrl()) = 0;

    bool httpCheckMethod(const IConnection::CheckUrl &aHost);

    void toLog(LogLevel::Enum aLevel, const QString &aMessage) const;

    NetworkTaskManager *mNetwork;

    QString mName;
    bool mConnected;
    bool mWatch;
    int mPingPeriod;
    int mCheckCount;
    QTimer mCheckTimer;
    QList<CheckUrl> mCheckHosts;
    ILog *mLog;
};

//--------------------------------------------------------------------------------
