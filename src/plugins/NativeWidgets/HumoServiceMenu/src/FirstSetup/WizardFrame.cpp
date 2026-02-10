/* @file Окно визарда. */

#include "WizardFrame.h"

#include <QtWidgets/QCheckBox>

#include "Backend/HumoServiceBackend.h"
#include "DialupWizardPage.h"
#include "GUI/MessageBox/MessageBox.h"
#include "HardwareWizardPage.h"
#include "KeysWizardPage.h"
#include "NetworkWizardPage.h"
#include "SaveSettingsWizardPage.h"
#include "TokenWizardPage.h"
#include "UnmanagedWizardPage.h"
#include "WelcomeWizardPage.h"
#include "WizardContext.h"

namespace CWizardFrame {
char *ContextProperty = "humoContext";
} // namespace CWizardFrame

//----------------------------------------------------------------------------
WizardFrame::WizardFrame(HumoServiceBackend *aBackend, QWidget *aParent)
    : QWidget(aParent), m_Backend(aBackend), m_CurrentPage(0) {
    setupUi(this);
    wPage->setLayout(new QGridLayout);

    m_SignalMapper.connect(btnBack, SIGNAL(clicked()), SLOT(map()));
    m_SignalMapper.connect(btnForward, SIGNAL(clicked()), SLOT(map()));

    connect(this, SIGNAL(changePage(const QString &)), this, SLOT(onChangePage(const QString &)));
    connect(
        &m_SignalMapper, SIGNAL(mapped(const QString &)), SLOT(onControlEvent(const QString &)));
    connect(btnExit, SIGNAL(clicked()), SLOT(onExit()));

    m_Backend->toLog("Welcome to the First Setup Wizard!");
}

//----------------------------------------------------------------------------
WizardFrame::~WizardFrame() {}

//----------------------------------------------------------------------------
void WizardFrame::initialize() {
    emit changePage(CWizardContext::StartPage);
}

//----------------------------------------------------------------------------
void WizardFrame::shutdown() {
    hidePage(m_CurrentContext, m_CurrentPage);

    foreach (CacheItem item, m_Pages.values()) {
        item.page->deactivate();
    }
}

//----------------------------------------------------------------------------
void WizardFrame::setStatus(const QString &aStatus) {
    lbStatus->setText(aStatus);
}

//----------------------------------------------------------------------------
void WizardFrame::setPage(const QString &aContext, WizardPageBase *aPage, bool aCanCache) {
    if (aCanCache) {
        CacheItem item;
        item.page = aPage;
        m_Pages[aContext] = item;
    }

    connect(
        aPage, SIGNAL(pageEvent(const QString &, bool)), SLOT(onPageEvent(const QString &, bool)));
    hidePage(m_CurrentContext, m_CurrentPage);
    showPage(aContext, aPage);

    m_Backend->saveConfiguration();
}

//----------------------------------------------------------------------------
void WizardFrame::commitPageChanges() {
    if (m_CurrentPage) {
        m_CurrentPage->initialize();
    }
}

//----------------------------------------------------------------------------
void WizardFrame::showPage(const QString &aContext, WizardPageBase *aPage) {
    m_CurrentContext = aContext;
    m_CurrentPage = aPage;

    if (m_CurrentPage) {
        m_CurrentPage->activate();
        wPage->layout()->addWidget(m_CurrentPage);
        m_CurrentPage->setProperty(CWizardFrame::ContextProperty, aContext);
        m_CurrentPage->show();
    }
}

//----------------------------------------------------------------------------
void WizardFrame::hidePage(const QString &aContext, WizardPageBase *aPage) {
    Q_UNUSED(aContext)

    if (aPage) {
        aPage->hide();
        wPage->layout()->removeWidget(aPage);
    }
}

//----------------------------------------------------------------------------
void WizardFrame::setupDecoration(const QString &aStage,
                                  const QString &aStageName,
                                  const QString &aStageHowto) {
    lbStage->setText(aStage);
    lbStageName->setText(aStageName);
    lbStageHowto->setText(aStageHowto);
}

//----------------------------------------------------------------------------
void WizardFrame::setupControl(Control aControl,
                               bool aEnabled,
                               const QString &aTitle,
                               const QString &aContext,
                               bool aCanCache) {
    QPushButton *button = 0;

    switch (aControl) {
    case BackButton:
        button = btnBack;
        break;
    case ForwardButton:
        button = btnForward;
        break;
    case ExitButton:
        button = btnExit;
        break;
    }

    if (button) {
        button->setEnabled(aEnabled);
        button->setText(aTitle);
        button->setVisible(aEnabled);

        if (aCanCache) {
            m_SignalMapper.setMapping(button, aContext);
        }
    }
}

//----------------------------------------------------------------------------
QString WizardFrame::stageIndex(const QString &aContext) const {
    if (aContext == CWizardContext::SetupHardware) {
        return "1";
    } if (aContext == CWizardContext::SetupNetwork) {
        return "2";
    } else if (aContext == CWizardContext::SetupDialup) {
        return "3";
    } else if (aContext == CWizardContext::SetupUnmanaged) {
        return "3";
    } else if (aContext == CWizardContext::SetupToken) {
        return "4";
    } else if (aContext == CWizardContext::SetupKeys) {
        return "5";
    } else if (aContext == CWizardContext::SaveSettings) {
        return "6";
    }

    return "";
}

//----------------------------------------------------------------------------
void WizardFrame::onChangePage(const QString &aContext) {
#ifdef TC_USE_TOKEN
    bool hasToken = true;
#else
    bool hasToken = false;
#endif

    if (aContext == CWizardContext::StartPage) {
        WelcomeWizardPage *wwp = new WelcomeWizardPage(m_Backend, this);

        setPage(aContext, wwp);
        setupDecoration("", "", "");

        connectAllAbstractButtons(wwp);
    } else if (aContext == CWizardContext::SetupHardware) {
        HardwareWizardPage *hwp = new HardwareWizardPage(m_Backend, this);

        setPage(aContext, hwp);
        setupDecoration(
            stageIndex(aContext), tr("#hardware_setup_stage"), tr("#hardware_setup_howto"));
        setupControl(
            WizardFrame::BackButton, true, tr("#to_start_page"), CWizardContext::StartPage);
        setupControl(WizardFrame::ForwardButton,
                     true,
                     tr("#to_network_setup"),
                     CWizardContext::SetupNetwork);

        connectAllAbstractButtons(hwp);
    } else if (aContext == CWizardContext::SetupNetwork) {
        NetworkWizardPage *nwp = new NetworkWizardPage(m_Backend, this);

        setPage(aContext, nwp);
        setupDecoration(
            stageIndex(aContext), tr("#network_setup_stage"), tr("#network_setup_howto"));
        setupControl(
            WizardFrame::BackButton, true, tr("#to_hardware_setup"), CWizardContext::SetupHardware);

        connectAllAbstractButtons(nwp);
    } else if (aContext == CWizardContext::SetupDialup) {
        DialupWizardPage *dwp = new DialupWizardPage(m_Backend, this);

        setPage(aContext, dwp);
        setupDecoration(stageIndex(aContext), tr("#dialup_setup_stage"), tr("#dialup_setup_howto"));
        setupControl(
            WizardFrame::BackButton, true, tr("#to_network_setup"), CWizardContext::SetupNetwork);

        if (hasToken) {
            setupControl(WizardFrame::ForwardButton,
                         true,
                         tr("#to_token_setup"),
                         CWizardContext::SetupToken);
        } else {
            setupControl(
                WizardFrame::ForwardButton, true, tr("#to_keys_setup"), CWizardContext::SetupKeys);
        }

        connectAllAbstractButtons(dwp);
    } else if (aContext == CWizardContext::SetupUnmanaged) {
        UnmanagedWizardPage *uwp = new UnmanagedWizardPage(m_Backend, this);

        setPage(aContext, uwp);
        setupDecoration(
            stageIndex(aContext), tr("#unmanaged_setup_stage"), tr("#unmanaged_setup_howto"));
        setupControl(
            WizardFrame::BackButton, true, tr("#to_network_setup"), CWizardContext::SetupNetwork);

        if (hasToken) {
            setupControl(WizardFrame::ForwardButton,
                         true,
                         tr("#to_token_setup"),
                         CWizardContext::SetupToken);
        } else {
            setupControl(
                WizardFrame::ForwardButton, true, tr("#to_keys_setup"), CWizardContext::SetupKeys);
        }

        connectAllAbstractButtons(uwp);
    } else if (aContext == CWizardContext::SetupToken) {
        TokenWizardPage *rwp = new TokenWizardPage(m_Backend, this);

        setPage(aContext, rwp);
        setupDecoration(stageIndex(aContext), tr("#token_setup_stage"), tr("#token_setup_howto"));
        setupControl(
            WizardFrame::BackButton, true, tr("#to_network_setup"), CWizardContext::SetupNetwork);
        setupControl(
            WizardFrame::ForwardButton, true, tr("#to_keys_setup"), CWizardContext::SetupKeys);

        connectAllAbstractButtons(rwp);
    } else if (aContext == CWizardContext::SetupKeys) {
        KeysWizardPage *kwp = new KeysWizardPage(m_Backend, this);

        setPage(aContext, kwp);
        setupDecoration(stageIndex(aContext), tr("#keys_setup_stage"), tr("#keys_setup_howto"));

        if (hasToken) {
            setupControl(
                WizardFrame::BackButton, true, tr("#to_token_setup"), CWizardContext::SetupToken);
        } else {
            setupControl(WizardFrame::BackButton,
                         true,
                         tr("#to_network_setup"),
                         CWizardContext::SetupNetwork);
        }

        setupControl(WizardFrame::ForwardButton,
                     true,
                     tr("#to_save_settings"),
                     CWizardContext::SaveSettings);

        connectAllAbstractButtons(kwp);
    } else if (aContext == CWizardContext::SaveSettings) {
        SaveSettingsWizardPage *swp = new SaveSettingsWizardPage(m_Backend, this);

        setPage(aContext, swp);
        setupDecoration(
            stageIndex(aContext), tr("#save_settings_stage"), tr("#save_settings_howto"));

        connectAllAbstractButtons(swp);
    }

    commitPageChanges();
}

//----------------------------------------------------------------------------
void WizardFrame::onPageEvent(const QString &aContext, bool aFlag) {
    Q_UNUSED(aFlag)

    emit changePage(aContext);
}

//----------------------------------------------------------------------------
void WizardFrame::onControlEvent(const QString &aContext) {
    emit changePage(aContext);
}

//----------------------------------------------------------------------------
void WizardFrame::onExit() {
    if (GUI::MessageBox::question(tr("#question_exit"))) {
        QVariantMap parameters;
        parameters["signal"] = "exit";

        // Завершаем сценарий.
        m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, parameters);

        // Останавливаем ПО.
        m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::StopSoftware);

        m_Backend->toLog("Bye-bye.");
    }
}

//----------------------------------------------------------------------------
void WizardFrame::connectAllAbstractButtons(QWidget *aParentWidget) {
    foreach (QAbstractButton *b, aParentWidget->findChildren<QAbstractButton *>()) {
        connect(b, SIGNAL(clicked()), this, SLOT(onAbstractButtonClicked()));
    }
}

//----------------------------------------------------------------------------
void WizardFrame::onAbstractButtonClicked() {
    QAbstractButton *button = qobject_cast<QAbstractButton *>(sender());

    QString message(QString("Button clicked: %1").arg(button->text()));

    QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
    if (checkBox) {
        checkBox->isChecked() ? message += " (checked)" : message += " (unchecked)";
    }

    message += ".";

    m_Backend->toLog(message);
}

//----------------------------------------------------------------------------