/* @file Окно визарда. */

#include "WizardFrame.h"

#include <QtWidgets/QCheckBox>

#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"
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
char *ContextProperty = "cyberContext";
} // namespace CWizardFrame

//----------------------------------------------------------------------------
WizardFrame::WizardFrame(ServiceMenuBackend *aBackend, QWidget *aParent)
    : QWidget(aParent), m_Backend(aBackend), m_CurrentPage(nullptr) {
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
WizardFrame::~WizardFrame() = default;

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
    wPage->layout()->addWidget(aPage);
    aPage->show();
    aPage->activate();

    m_CurrentContext = aContext;
    m_CurrentPage = aPage;

    m_Backend->toLog(QString("Show page %1").arg(aContext));
}

//----------------------------------------------------------------------------
void WizardFrame::hidePage(const QString &aContext, WizardPageBase *aPage) {
    if (aPage) {
        aPage->deactivate();
        aPage->hide();
        wPage->layout()->removeWidget(aPage);
    }

    TPageMap::iterator page = m_Pages.find(aContext);

    if (page != m_Pages.end()) {
        for (auto &control : page->controls) {
            switch (control.control) {
            case BackButton:
                control.enabled = btnBack->isEnabled();
                break;

            case ForwardButton:
                control.enabled = btnForward->isEnabled();
                break;
            }
        }
    }

    btnBack->hide();
    btnForward->hide();

    m_SignalMapper.removeMappings(btnBack);
    m_SignalMapper.removeMappings(btnForward);
}

//----------------------------------------------------------------------------
void WizardFrame::setupDecoration(const QString &aStage,
                                  const QString &aStageName,
                                  const QString &aStageHowto) {
    lbStage->setText(aStage);
    lbStageName->setText(aStageName);
    lbStageHowto->setText(aStageHowto);

    TPageMap::iterator page = m_Pages.find(m_CurrentContext);

    if (page != m_Pages.end()) {
        page->stage = aStage;
        page->stageName = aStageName;
        page->stageHowto = aStageHowto;
    }
}

//----------------------------------------------------------------------------
void WizardFrame::setupControl(Control aControl,
                               bool aEnabled,
                               const QString &aTitle,
                               const QString &aContext,
                               bool aCanCache) {
    switch (aControl) {
    case BackButton:
        btnBack->show();
        btnBack->setEnabled(aEnabled);
        btnBack->setText(aTitle);
        btnBack->setProperty(CWizardFrame::ContextProperty, aContext);
        m_SignalMapper.setMapping(btnBack, aContext);
        break;

    case ForwardButton:
        btnForward->show();
        btnForward->setEnabled(aEnabled);
        btnForward->setText(aTitle);
        btnForward->setProperty(CWizardFrame::ContextProperty, aContext);
        m_SignalMapper.setMapping(btnForward, aContext);
        break;

    case ExitButton:
        btnExit->setVisible(aEnabled);
        break;
    }

    if (aCanCache) {
        TPageMap::iterator page = m_Pages.find(m_CurrentContext);

        if (page != m_Pages.end()) {
            CacheItem::ControlItem control;
            control.control = aControl;
            control.context = aContext;
            control.title = aTitle;
            control.enabled = aEnabled;

            page->controls << control;
        }
    }
}

//----------------------------------------------------------------------------
void WizardFrame::onPageEvent(const QString &aContext, bool aFlag) {
    if (aContext == "#can_proceed") {
        // Сигнализируем о возможности двигаться дальше
        btnForward->setEnabled(aFlag);
    } else if (aContext == "#main_form") {
        // Показ кнопок навигации
        wButtonBar->setVisible(aFlag);
    } else if (aFlag) {
        // Иначе переход по заданному контексту
        TPageMap::iterator page = m_Pages.find(aContext);

        if (page != m_Pages.end()) {
            hidePage(m_CurrentContext, m_CurrentPage);
            showPage(aContext, page->page);

            foreach (CacheItem::ControlItem control, page->controls) {
                setupControl(
                    control.control, control.enabled, control.title, control.context, false);
            }

            setupDecoration(page->stage, page->stageName, page->stageHowto);
        } else {
            emit changePage(aContext);
        }
    }
}

//----------------------------------------------------------------------------
void WizardFrame::onControlEvent(const QString &aContext) {
    onPageEvent(aContext, true);
}

//---------------------------------------------------------------------------
QString WizardFrame::stageIndex(const QString &aContext) {
#ifdef TC_USE_TOKEN
    bool hasRuToken = true;
#else
    bool hasRuToken = false;
#endif

    int index = 0;

    if (aContext == CWizardContext::StartPage) {
        index = 0;
    } else if (aContext == CWizardContext::SetupHardware) {
        index = 1;
    } else if (aContext == CWizardContext::SetupNetwork ||
               aContext == CWizardContext::SetupDialup ||
               aContext == CWizardContext::SetupUnmanaged) {
        index = 2;
    } else if (aContext == CWizardContext::SetupToken) {
        index = 3;
    } else if (aContext == CWizardContext::SetupKeys) {
        index = hasRuToken ? 4 : 3;
    } else {
        index = hasRuToken ? 5 : 4;
    }

    return QString("%1/%2").arg(index).arg(hasRuToken ? 5 : 4);
}

//---------------------------------------------------------------------------
void WizardFrame::onChangePage(const QString &aContext) {
#ifdef TC_USE_TOKEN
    bool hasToken = true;
#else
    bool hasToken = false;
#endif

    if (aContext == CWizardContext::StartPage) {
        auto *wwp = new WelcomeWizardPage(m_Backend, this);

        setPage(aContext, wwp);
        setupDecoration("", "", "");

        connectAllAbstractButtons(wwp);
    } else if (aContext == CWizardContext::SetupHardware) {
        auto *hwp = new HardwareWizardPage(m_Backend, this);

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
        auto *nwp = new NetworkWizardPage(m_Backend, this);

        setPage(aContext, nwp);
        setupDecoration(
            stageIndex(aContext), tr("#network_setup_stage"), tr("#network_setup_howto"));
        setupControl(
            WizardFrame::BackButton, true, tr("#to_hardware_setup"), CWizardContext::SetupHardware);

        connectAllAbstractButtons(nwp);
    } else if (aContext == CWizardContext::SetupDialup) {
        auto *dwp = new DialupWizardPage(m_Backend, this);

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
        auto *uwp = new UnmanagedWizardPage(m_Backend, this);

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
        auto *rwp = new TokenWizardPage(m_Backend, this);

        setPage(aContext, rwp);
        setupDecoration(stageIndex(aContext), tr("#token_setup_stage"), tr("#token_setup_howto"));
        setupControl(
            WizardFrame::BackButton, true, tr("#to_network_setup"), CWizardContext::SetupNetwork);
        setupControl(
            WizardFrame::ForwardButton, true, tr("#to_keys_setup"), CWizardContext::SetupKeys);

        connectAllAbstractButtons(rwp);
    } else if (aContext == CWizardContext::SetupKeys) {
        auto *kwp = new KeysWizardPage(m_Backend, this);

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
        auto *swp = new SaveSettingsWizardPage(m_Backend, this);

        setPage(aContext, swp);
        setupDecoration(
            stageIndex(aContext), tr("#save_settings_stage"), tr("#save_settings_howto"));

        connectAllAbstractButtons(swp);
    }

    commitPageChanges();
}

//----------------------------------------------------------------------------
void WizardFrame::onExit() {
    if (GUI::MessageBox::question(tr("#question_exit")) != 0) {
        QVariantMap parameters;
        parameters["signal"] = "exit";

        // Завершаем сценарий.
        m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, parameters);

        // Останавливаем ПО.
        m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::StopSoftware);

        m_Backend->toLog("Bye-bye.");
    }
}

//------------------------------------------------------------------------
void WizardFrame::connectAllAbstractButtons(QWidget *aParentWidget) {
    foreach (QAbstractButton *b, aParentWidget->findChildren<QAbstractButton *>()) {
        connect(b, SIGNAL(clicked()), this, SLOT(onAbstractButtonClicked()));
    }
}

//------------------------------------------------------------------------
void WizardFrame::onAbstractButtonClicked() {
    auto *button = qobject_cast<QAbstractButton *>(sender());

    QString message(QString("Button clicked: %1").arg(button->text()));

    auto *checkBox = qobject_cast<QCheckBox *>(sender());
    if (checkBox) {
        checkBox->isChecked() ? message += " (checked)" : message += " (unchecked)";
    }

    message += ".";

    m_Backend->toLog(message);
}

//----------------------------------------------------------------------------
