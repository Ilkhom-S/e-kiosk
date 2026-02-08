/* @file Окно с локальным соединением. */

#include "UnmanagedWizardPage.h"

#include <QtConcurrent/QtConcurrentRun>

#include <SDK/PaymentProcessor/Connection/Connection.h>
#include <SDK/PaymentProcessor/Connection/ConnectionTypes.h>

#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "GUI/MessageBox/MessageBox.h"
#include "GUI/ServiceTags.h"
#include "GUI/UnmanagedConnectionWindow.h"

UnmanagedWizardPage::UnmanagedWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent), m_ConnectionWindow(new UnmanagedConnectionWindow) {
    m_ConnectionWindow->setParent(this);
    setLayout(new QHBoxLayout(this));
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(m_ConnectionWindow);

    connect(m_ConnectionWindow,
            SIGNAL(testConnection(QNetworkProxy)),
            SLOT(onTestConnection(QNetworkProxy)));
    connect(m_ConnectionWindow, SIGNAL(userSelectionChanged()), SLOT(onUserSelectionChanged()));
    connect(&m_TaskWatcher, SIGNAL(finished()), SLOT(onTestFinished()));
}

//------------------------------------------------------------------------
bool UnmanagedWizardPage::initialize() {
    m_ConnectionSettings[CServiceTags::ConnectionType] = EConnectionTypes::Unmanaged;
    m_ConnectionSettings[CServiceTags::Connection] = "unmanaged connection";

    QVariantMap result;
    m_Backend->getNetworkManager()->getNetworkInfo(result);

    // Заполняем хосты для пинга и временные интервалы
    foreach (const QString &param, result.keys()) {
        if (param.contains(CServiceTags::CheckHost) || (param == CServiceTags::CheckInterval)) {
            m_StaticParameters.insert(param, result[param]);
        }
    }

    QNetworkProxy proxy(QNetworkProxy::NoProxy);

    if (result.contains(CServiceTags::ProxyType)) {
        proxy.setType(QNetworkProxy::ProxyType(result[CServiceTags::ProxyType].toInt()));
        proxy.setHostName(result[CServiceTags::ProxyAddress].toString());
        proxy.setPort(static_cast<quint16>(result[CServiceTags::ProxyPort].toUInt()));
        proxy.setUser(result[CServiceTags::ProxyUser].toString());
        proxy.setPassword(result[CServiceTags::ProxyPassword].toString());
    }

    m_ConnectionWindow->initialize(proxy);

    emit pageEvent("#can_proceed", false);

    return true;
}

//------------------------------------------------------------------------
bool UnmanagedWizardPage::shutdown() {
    m_TaskWatcher.waitForFinished();

    return true;
}

//------------------------------------------------------------------------
bool UnmanagedWizardPage::activate() {
    return true;
}

//------------------------------------------------------------------------
bool UnmanagedWizardPage::deactivate() {
    SDK::PaymentProcessor::SConnection connection;

    connection.name = "unmanaged connection";
    connection.type = EConnectionTypes::Unmanaged;

    QNetworkProxy proxy(QNetworkProxy::NoProxy);

    if (m_ConnectionSettings.contains(CServiceTags::ProxyType)) {
        proxy.setType(
            QNetworkProxy::ProxyType(m_ConnectionSettings[CServiceTags::ProxyType].toInt()));
        proxy.setHostName(m_ConnectionSettings[CServiceTags::ProxyAddress].toString());
        proxy.setPort(static_cast<quint16>(m_ConnectionSettings[CServiceTags::ProxyPort].toUInt()));
        proxy.setUser(m_ConnectionSettings[CServiceTags::ProxyUser].toString());
        proxy.setPassword(m_ConnectionSettings[CServiceTags::ProxyPassword].toString());
    }

    connection.proxy = proxy;

    m_Backend->getNetworkManager()->setConnection(connection);

    return true;
}

//---------------------------------------------------------------------------
void UnmanagedWizardPage::onUserSelectionChanged() {
    emit pageEvent("#can_proceed", false);
}

//---------------------------------------------------------------------------
void UnmanagedWizardPage::onTestConnection(QNetworkProxy aProxy) {
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

    m_Backend->getNetworkManager()->setConnection(connection);

    GUI::MessageBox::wait(tr("#testing_connection"));

    m_TaskWatcher.setFuture(QtConcurrent::run([this]() {
        QString errorMessage;
        return m_Backend->getNetworkManager()->testConnection(errorMessage);
    }));
}

//---------------------------------------------------------------------------
void UnmanagedWizardPage::onTestFinished() {
    GUI::MessageBox::hide();
    GUI::MessageBox::info(m_TaskWatcher.result() ? tr("#connection_test_ok")
                                                 : tr("#connection_test_failed"));

    // сохраняем в m_ConnectionSettings хорошие настройки прокси
    if (m_TaskWatcher.result()) {
        auto proxy = m_Backend->getNetworkManager()->getConnection().proxy;

        m_ConnectionSettings[CServiceTags::ProxyType] = proxy.type();
        m_ConnectionSettings[CServiceTags::ProxyAddress] = proxy.hostName();
        m_ConnectionSettings[CServiceTags::ProxyPort] = proxy.port();
        m_ConnectionSettings[CServiceTags::ProxyUser] = proxy.user();
        m_ConnectionSettings[CServiceTags::ProxyPassword] = proxy.password();
    }

    emit pageEvent("#can_proceed", m_TaskWatcher.result());
}

//------------------------------------------------------------------------
