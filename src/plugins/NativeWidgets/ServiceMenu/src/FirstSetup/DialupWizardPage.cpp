/* @file Окно с модемным соединением. */

// boost

#include "DialupWizardPage.h"

#include <QtConcurrent/QtConcurrentRun>

#include <SDK/PaymentProcessor/Connection/ConnectionTypes.h>

#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>

#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "GUI/DialupConnectionWindow.h"
#include "GUI/MessageBox/MessageBox.h"
#include "GUI/ServiceTags.h"

DialupWizardPage::DialupWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent), m_ConnectionWindow(new DialupConnectionWindow(this)) {
    
    m_ConnectionWindow->setParent(this);
    setLayout(new QHBoxLayout(this));
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(m_ConnectionWindow);

    connect(m_ConnectionWindow,
            SIGNAL(createConnection(const QString &, const QString &)),
            SLOT(onCreateConnection(const QString &, const QString &)));
    connect(m_ConnectionWindow,
            SIGNAL(testConnection(const QString &)),
            SLOT(onTestConnection(const QString &)));
    connect(m_ConnectionWindow,
            SIGNAL(removeConnection(const QString &)),
            SLOT(onRemoveConnection(const QString &)));
    connect(m_ConnectionWindow,
            SIGNAL(userSelectionChanged(const QString &)),
            SLOT(onSelectionChanged(const QString &)));

    connect(&m_TaskWatcher, SIGNAL(finished()), SLOT(onTestFinished()));
}

//------------------------------------------------------------------------
bool DialupWizardPage::initialize() {
    m_ConnectionWindow->initialize();
    m_ConnectionWindow->fillTemplateList(m_Backend->getNetworkManager()->getConnectionTemplates());
    m_ConnectionWindow->fillModem_List(m_Backend->getNetworkManager()->getModems());

    QVariantMap networkInfo;
    m_Backend->getNetworkManager()->getNetworkInfo(networkInfo);
    m_ConnectionWindow->fillConnectionList(m_Backend->getNetworkManager()->getRemoteConnections(),
                                           networkInfo[CServiceTags::Connection].toString());

    emit pageEvent("#can_proceed", false);

    return true;
}

//------------------------------------------------------------------------
bool DialupWizardPage::shutdown() {
    m_TaskWatcher.waitForFinished();

    return true;
}

//------------------------------------------------------------------------
bool DialupWizardPage::activate() {
    return true;
}

//------------------------------------------------------------------------
bool DialupWizardPage::deactivate() {
    SDK::PaymentProcessor::SConnection connection;

    connection.type = EConnectionTypes::Dialup;
    connection.name = m_ConnectionWindow->getUserSelection();

    m_Backend->getNetworkManager()->setConnection(connection);

    return true;
}

//------------------------------------------------------------------------
void DialupWizardPage::onSelectionChanged(const QString & /*aSelectedConnection*/) {
    emit pageEvent("#can_proceed", false);
}

//------------------------------------------------------------------------
void DialupWizardPage::onCreateConnection(const QString &aConnection,
                                          const QString &aNetworkDevice) {
    SDK::PaymentProcessor::SConnection connection;

    connection.type = EConnectionTypes::Dialup;
    connection.name = aConnection;

    if (m_Backend->getNetworkManager()->createDialupConnection(connection, aNetworkDevice)) {
        m_ConnectionWindow->fillConnectionList(
            m_Backend->getNetworkManager()->getRemoteConnections(), aConnection);
        m_ConnectionWindow->switchToListPage();
    } else {
        GUI::MessageBox::warning(tr("#failed_create_connection"));
    }
}

//------------------------------------------------------------------------
void DialupWizardPage::onTestConnection(const QString &aConnection) {
    SDK::PaymentProcessor::SConnection connection;

    connection.type = EConnectionTypes::Dialup;
    connection.name = aConnection;
    m_Backend->getNetworkManager()->setConnection(connection);

    GUI::MessageBox::wait(tr("#testing_connection"));

    m_TaskWatcher.setFuture(QtConcurrent::run(boost::bind(&NetworkManager::testConnection,
                                                          m_Backend->getNetworkManager(),
                                                          boost::ref(m_ConnectionError))));
}

//------------------------------------------------------------------------
void DialupWizardPage::onRemoveConnection(const QString &aConnection) {
    SDK::PaymentProcessor::SConnection connection;

    connection.type = EConnectionTypes::Dialup;
    connection.name = aConnection;

    if (m_Backend->getNetworkManager()->removeDialupConnection(connection)) {
        m_ConnectionWindow->fillConnectionList(
            m_Backend->getNetworkManager()->getRemoteConnections(), aConnection);
        m_ConnectionWindow->switchToListPage();
    } else {
        GUI::MessageBox::warning(tr("#failed_remove_connection"));
    }
}

//---------------------------------------------------------------------------
void DialupWizardPage::onTestFinished() {
    GUI::MessageBox::hide();
    GUI::MessageBox::info(m_TaskWatcher.result()
                              ? tr("#connection_test_ok")
                              : tr("#connection_test_failed") + "\n" + m_ConnectionError);

    emit pageEvent("#can_proceed", m_TaskWatcher.result());
}

//------------------------------------------------------------------------
