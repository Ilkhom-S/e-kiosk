/* @file Реализация dialup-соединения. */

#pragma once

#include "Common/ConnectionBase.h"

//--------------------------------------------------------------------------------
class DialupConnection : public ConnectionBase {
    Q_OBJECT

public:
    /// Конструктор.
    DialupConnection(const QString &aName, NetworkTaskManager *aNetwork, ILog *aLog);

    /// Деструктор.
    virtual ~DialupConnection();

    /// Возвращает тип соединения.
    virtual EConnectionTypes::Enum getType() const;

private:
    /// Устанавливает соединение используя RAS API.
    virtual void doConnect() throw(...) override;

    /// Разрывает соединение используя RAS API.
    virtual void doDisconnect() throw(...) override;

    /// Проверяет наличие установленного соединения используя RAS API.
    virtual bool doIsConnected() throw(...) override;

    /// Использует метод HTTP HEAD базового класса.
    virtual bool doCheckConnection(const CheckUrl &aHost = CheckUrl()) override;
};

//--------------------------------------------------------------------------------
