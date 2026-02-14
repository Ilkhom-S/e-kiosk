/* @file Реализация клиента сторожевого сервиса. */

#include "WatchServiceClient.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaType>
#include <QtCore/QTimer>

#include <Common/BasicApplication.h>
#include <Common/ILog.h>

#include <WatchServiceClient/Constants.h>
#include <boost/bind/bind.hpp>
#include <utility>

IWatchServiceClient *createWatchServiceClient(const QString &aClientName,
                                              IWatchServiceClient::PingThread aThread) {
    return new WatchServiceClient(aClientName, aThread);
}

//---------------------------------------------------------------------------
WatchServiceClient::WatchServiceClient(QString aName, PingThread aThread)
    : m_Name(std::move(aName)) {
    qRegisterMetaType<WatchServiceClient::TMethod>("WatchServiceClient::TMethod");

    connect(this,
            SIGNAL(invokeMethod(WatchServiceClient::TMethod)),
            SLOT(onInvokeMethod(WatchServiceClient::TMethod)));

    moveToThread(this);

    connect(&m_PingTimer,
            SIGNAL(timeout()),
            SLOT(onPing()),
            aThread == DedicateThread ? Qt::AutoConnection : Qt::QueuedConnection);

    if (aThread == DedicateThread) {
        m_PingTimer.moveToThread(this);
    }
}

//---------------------------------------------------------------------------
WatchServiceClient::~WatchServiceClient() {
    // Явно указываем класс, чтобы избежать virtual dispatch в деструкторе
    WatchServiceClient::stop();
}

//---------------------------------------------------------------------------
bool WatchServiceClient::start() {
    if (isRunning()) {
        return false;
    }

    m_InitMutex.lock();

    QThread::start();

    m_InitCondition.wait(&m_InitMutex);
    m_InitMutex.unlock();

    return isConnected();
}

//---------------------------------------------------------------------------
void WatchServiceClient::run() {
    m_Client = QSharedPointer<IMessageQueueClient>(createMessageQueueClient());
    m_Client->subscribeOnMessageReceived(this);
    m_Client->subscribeOnDisconnected(this);

    m_InitMutex.lock();

    if (m_Client->connect(CWatchService::MessageQueue)) {
        QMetaObject::invokeMethod(&m_PingTimer, "start", Q_ARG(int, CWatchService::PingInterval));

        m_InitCondition.wakeAll();
        m_InitMutex.unlock();

        QThread::exec();

        m_Client.clear();

        QMetaObject::invokeMethod(&m_PingTimer, "stop");
    } else {
        m_Client.clear();

        m_InitCondition.wakeAll();
        m_InitMutex.unlock();
    }
}

//---------------------------------------------------------------------------
void WatchServiceClient::stop() {
    quit();
    if (!wait(3000)) {
        terminate();
    }
}

//---------------------------------------------------------------------------
bool WatchServiceClient::isConnected() const {
    if (!m_Client) {
        return false;
    }
    return m_Client->isConnected();
}

//---------------------------------------------------------------------------
void WatchServiceClient::execute(QString aCommand, QString aModule, QString aParams) {
    QString command = QString::fromLatin1("%1=%2;").arg(CWatchService::Fields::Sender).arg(m_Name);

    if (!aCommand.isEmpty()) {
        command += QString::fromLatin1("%1=%2;").arg(CWatchService::Fields::Type).arg(aCommand);
    }

    if (!aModule.isEmpty()) {
        command += QString::fromLatin1("%1=%2;").arg(CWatchService::Fields::Module).arg(aModule);
    }

    if (!aParams.isEmpty()) {
        command += QString::fromLatin1("%1=%2;").arg(CWatchService::Fields::Params).arg(aParams);
    }

    emit invokeMethod([this, capture0 = command.toUtf8()] { sendMessage(capture0); });
}

//---------------------------------------------------------------------------
void WatchServiceClient::stopService() {
    execute(CWatchService::Commands::Exit, CWatchService::Modules::WatchService);
}

//---------------------------------------------------------------------------
void WatchServiceClient::restartService(QStringList aParameters) {
    execute(CWatchService::Commands::Restart,
            CWatchService::Modules::WatchService,
            aParameters.join(" "));
}

//---------------------------------------------------------------------------
void WatchServiceClient::rebootMachine() {
    execute(CWatchService::Commands::Reboot, CWatchService::Modules::WatchService);
}

//---------------------------------------------------------------------------
void WatchServiceClient::shutdownMachine() {
    execute(CWatchService::Commands::Shutdown, CWatchService::Modules::WatchService);
}

//---------------------------------------------------------------------------
void WatchServiceClient::startModule(QString aModule, QString aParams) {
    execute(CWatchService::Commands::StartModule, aModule, aParams);
}

//---------------------------------------------------------------------------
void WatchServiceClient::closeModule(QString aModule) {
    execute(CWatchService::Commands::CloseModule, aModule);
}

//---------------------------------------------------------------------------
void WatchServiceClient::closeModules() {
    execute(CWatchService::Commands::Close);
}

//---------------------------------------------------------------------------
void WatchServiceClient::showSplashScreen() {
    execute(CWatchService::Commands::ShowSplashScreen, CWatchService::Modules::WatchService);
}

//---------------------------------------------------------------------------
void WatchServiceClient::hideSplashScreen() {
    execute(CWatchService::Commands::HideSplashScreen, CWatchService::Modules::WatchService);
}

//---------------------------------------------------------------------------
void WatchServiceClient::setState(int aType, int aStatus) {
    execute(CWatchService::Commands::SetState,
            CWatchService::Modules::WatchService,
            QString("%1_%2").arg(aType).arg(aStatus));
}

//---------------------------------------------------------------------------
void WatchServiceClient::resetState() {
    execute(CWatchService::Commands::ResetState, CWatchService::Modules::WatchService);
}

//---------------------------------------------------------------------------
bool WatchServiceClient::subscribeOnCommandReceived(QObject *aObject) {
    return aObject->connect(
               this,
               SIGNAL(onCommandReceived(
                   const QString &, const QString &, const QString &, const QStringList &)),
               SLOT(onCommandReceived(
                   const QString &, const QString &, const QString &, const QStringList &)),
               Qt::UniqueConnection) != nullptr;
}

//---------------------------------------------------------------------------
bool WatchServiceClient::subscribeOnCloseCommandReceived(QObject *aObject) {
    return connect(this,
                   SIGNAL(onCloseCommandReceived()),
                   aObject,
                   SLOT(onCloseCommandReceived()),
                   Qt::UniqueConnection) != nullptr;
}

//---------------------------------------------------------------------------
bool WatchServiceClient::subscribeOnDisconnected(QObject *aObject) {
    return connect(this,
                   SIGNAL(disconnected()),
                   aObject,
                   SLOT(onDisconnected()),
                   Qt::UniqueConnection) != nullptr;
}

//---------------------------------------------------------------------------
bool WatchServiceClient::subscribeOnModuleClosed(QObject *aObject) {
    return connect(this,
                   SIGNAL(onModuleClosed(const QString &)),
                   aObject,
                   SLOT(onModuleClosed(const QString &)),
                   Qt::UniqueConnection) != nullptr;
}

//---------------------------------------------------------------------------
void WatchServiceClient::ping() {
    execute(CWatchService::Commands::Ping, CWatchService::Modules::WatchService);
}

//---------------------------------------------------------------------------
void WatchServiceClient::sendMessage(const QByteArray &aMessage) {
    if (m_Client) {
        m_Client->sendMessage(aMessage);
    }
}

//---------------------------------------------------------------------------
void WatchServiceClient::onInvokeMethod(const WatchServiceClient::TMethod &aMethod) {
    aMethod();
}

//---------------------------------------------------------------------------
void WatchServiceClient::onPing() {
    ping();
}

//---------------------------------------------------------------------------
void WatchServiceClient::onMessageReceived(QByteArray aMessage) {
    QStringList params = QString::fromUtf8(aMessage.data(), aMessage.size()).split(";");
    if (!params.isEmpty()) {
        QString sender;
        QString type;
        QString target;
        QStringList tail;

        foreach (QString param, params) {
            if (param.indexOf(CWatchService::Fields::Sender) != -1) {
                sender = param.right(param.length() - param.indexOf("=") - 1);
            } else if (param.indexOf(CWatchService::Fields::Type) != -1) {
                type = param.right(param.length() - param.indexOf("=") - 1);
            } else if (param.indexOf(CWatchService::Fields::Target) != -1) {
                target = param.right(param.length() - param.indexOf("=") - 1);
            } else {
                tail.append(param);
            }
        }

        if (target.isEmpty() || (target == m_Name)) {
            if ((sender == CWatchService::Name) && (type == CWatchService::Commands::Close)) {
                emit onCloseCommandReceived();
            } else if ((sender == CWatchService::Name) &&
                       (type == CWatchService::Commands::CloseLogs)) {
                ILog::logRotateAll();
            } else if (type == CWatchService::Notification::ModuleClosed) {
                emit onModuleClosed(sender);
            } else {
                emit onCommandReceived(sender, target, type, tail);
            }
        }
    }
}

//---------------------------------------------------------------------------
void WatchServiceClient::onDisconnected() {
    emit disconnected();

    quit();
}

//---------------------------------------------------------------------------
