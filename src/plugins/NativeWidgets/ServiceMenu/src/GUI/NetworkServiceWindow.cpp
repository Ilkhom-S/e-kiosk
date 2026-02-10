/* @file Окно сетевых настроек. */

// boost

#include "NetworkServiceWindow.h"

#include <QtConcurrent/QtConcurrentRun>

#include <Common/ILog.h>

#include <SDK/PaymentProcessor/Connection/ConnectionTypes.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>

#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "MessageBox/MessageBox.h"
#include "ServiceTags.h"

NetworkServiceWindow::NetworkServiceWindow(ServiceMenuBackend *aBackend, QWidget *aParent)
    : QWidget(aParent), ServiceWindowBase(aBackend), m_Backend(aBackend), m_DialupWindow(nullptr),
      m_UnmanagedWindow(nullptr) {
    setupUi(this);

    m_TypeButtonGroup.addButton(rbDialupLink);
    m_TypeButtonGroup.addButton(rbUnmanagedLink);

    connect(&m_TypeButtonGroup,
            SIGNAL(buttonClicked(QAbstractButton *)),
            SLOT(onChangeConnectionType(QAbstractButton *)));
}

//------------------------------------------------------------------------
bool NetworkServiceWindow::activate() {
    return true;
}

//------------------------------------------------------------------------
bool NetworkServiceWindow::deactivate() {
    SDK::PaymentProcessor::SConnection connection;

    if (rbUnmanagedLink->isChecked()) {
        connection.name = "unmanaged connection";
        connection.type = EConnectionTypes::Unmanaged;
        connection.proxy = m_UnmanagedWindow->getUserSelection();
    } else if (rbDialupLink->isChecked()) {
        connection.name = m_DialupWindow->getUserSelection();
        connection.type = EConnectionTypes::Dialup;
    }

    m_Backend->getNetworkManager()->setConnection(connection);
    m_Backend->saveConfiguration();

    return true;
}

//------------------------------------------------------------------------
bool NetworkServiceWindow::initialize() {
    connect(&m_TaskWatcher, SIGNAL(finished()), SLOT(onTestFinished()));

    QVariantMap networkInfo;
    m_Backend->getNetworkManager()->getNetworkInfo(networkInfo);

    auto type =
        EConnectionTypes::Enum(networkInfo[CServiceTags::ConnectionType].toInt());

    // Инициализация окна с настройками модемного соединения
    m_DialupWindow = new DialupConnectionWindow(this);
    m_DialupWindow->initialize();
    m_DialupWindow->fillTemplateList(m_Backend->getNetworkManager()->getConnectionTemplates());
    m_DialupWindow->fillModem_List(m_Backend->getNetworkManager()->getModems());
    m_DialupWindow->fillConnectionList(m_Backend->getNetworkManager()->getRemoteConnections(),
                                       networkInfo[CServiceTags::Connection].toString());

    connect(m_DialupWindow,
            SIGNAL(createConnection(const QString &, const QString &)),
            SLOT(onCreateDialupConnection(const QString &, const QString &)));
    connect(m_DialupWindow,
            SIGNAL(testConnection(const QString &)),
            SLOT(onTestDialupConnection(const QString &)));
    connect(m_DialupWindow,
            SIGNAL(removeConnection(const QString &)),
            SLOT(onRemoveDialupConnection(const QString &)));

    swContainer->addWidget(m_DialupWindow);

    // Инициализация окна с настройками соединения по локальной сети
    QNetworkProxy proxy(
        static_cast<QNetworkProxy::ProxyType>(networkInfo[CServiceTags::ProxyType].toInt()),
        networkInfo[CServiceTags::ProxyAddress].toString(),
        static_cast<quint16>(networkInfo[CServiceTags::ProxyPort].toUInt()),
        networkInfo[CServiceTags::ProxyUser].toString(),
        networkInfo[CServiceTags::ProxyPassword].toString());

    m_UnmanagedWindow = new UnmanagedConnectionWindow(this);
    m_UnmanagedWindow->initialize(proxy);

    connect(m_UnmanagedWindow,
            SIGNAL(testConnection(QNetworkProxy)),
            SLOT(onTestUnmanagedConnection(QNetworkProxy)));

    swContainer->addWidget(m_UnmanagedWindow);

    // Контролы переключения типа соединения
    if (type == EConnectionTypes::Unmanaged) {
        rbUnmanagedLink->click();
    } else if (type == EConnectionTypes::Dialup) {
        rbDialupLink->click();
    }

    return true;
}

//------------------------------------------------------------------------
bool NetworkServiceWindow::shutdown() {
    m_TaskWatcher.waitForFinished();

    return true;
}

//------------------------------------------------------------------------
void NetworkServiceWindow::onChangeConnectionType(QAbstractButton *aButton) {
    if (aButton == rbDialupLink) {
        swContainer->setCurrentWidget(m_DialupWindow);
    } else {
        swContainer->setCurrentWidget(m_UnmanagedWindow);
    }
}

//------------------------------------------------------------------------
void NetworkServiceWindow::onCreateDialupConnection(const QString &aConnection,
                                                    const QString &aNetworkDevice) {
    SDK::PaymentProcessor::SConnection connection;

    connection.type = EConnectionTypes::Dialup;
    connection.name = aConnection;

    m_Backend->getNetworkManager()->createDialupConnection(connection, aNetworkDevice);

    QVariantMap connections;
    foreach (QString c, m_Backend->getNetworkManager()->getRemoteConnections()) {
        connections.insert(c, 0);
    }

    m_DialupWindow->fillConnectionList(connections.keys(), aConnection);
    m_DialupWindow->switchToListPage();
}

//------------------------------------------------------------------------
void NetworkServiceWindow::onTestDialupConnection(const QString &aConnection) {
    SDK::PaymentProcessor::SConnection connection;

    connection.type = EConnectionTypes::Dialup;
    connection.name = aConnection;

    GUI::MessageBox::wait(tr("#testing_connection"));

    m_Backend->getNetworkManager()->setConnection(connection);
    m_TaskWatcher.setFuture(QtConcurrent::run(boost::bind(&NetworkManager::testConnection,
                                                          m_Backend->getNetworkManager(),
                                                          boost::ref(m_ConnectionError))));
}

//------------------------------------------------------------------------
void NetworkServiceWindow::onRemoveDialupConnection(const QString &aConnection) {
    SDK::PaymentProcessor::SConnection connection;

    connection.type = EConnectionTypes::Dialup;
    connection.name = aConnection;

    m_Backend->getNetworkManager()->removeDialupConnection(connection);

    QVariantMap connections;
    foreach (QString c, m_Backend->getNetworkManager()->getRemoteConnections()) {
        connections.insert(c, 0);
    }

    m_DialupWindow->fillConnectionList(connections.keys(), aConnection);
    m_DialupWindow->switchToListPage();
}

//---------------------------------------------------------------------------
void NetworkServiceWindow::onTestUnmanagedConnection(QNetworkProxy aProxy) {
    SDK::PaymentProcessor::SConnection connection;

    connection.name = "unmanaged connection";
    connection.type = EConnectionTypes::Unmanaged;

    QNetworkProxy proxy(QNetworkProxy::NoProxy);

    proxy.setType(aProxy.type());
    proxy.setHostName(aProxy.hostName());
    proxy.setPort(aProxy.port());
    proxy.setUser(aProxy.user());
    proxy.setPassword(aProxy.password());

    connection.proxy = proxy;

    GUI::MessageBox::wait(tr("#testing_connection"));

    m_Backend->getNetworkManager()->setConnection(connection);
    m_TaskWatcher.setFuture(QtConcurrent::run(boost::bind(&NetworkManager::testConnection,
                                                          m_Backend->getNetworkManager(),
                                                          boost::ref(m_ConnectionError))));
}

//---------------------------------------------------------------------------
void NetworkServiceWindow::onTestFinished() {
    GUI::MessageBox::hide();
    GUI::MessageBox::info(m_TaskWatcher.result()
                              ? tr("#connection_test_ok")
                              : tr("#connection_test_failed") + "\n" + m_ConnectionError);
}

//------------------------------------------------------------------------
