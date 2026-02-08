/* @file Окно визарда. */

#pragma once

#include <QtCore/QSignalMapper>
#include <QtWidgets/QWidget>

#include "WizardPage.h"
#include "ui_WizardFrame.h"

class ServiceMenuBackend;

//----------------------------------------------------------------------------
class WizardFrame : public QWidget, protected Ui::WizardFrame {
    Q_OBJECT

public:
    enum Control { BackButton, ForwardButton, ExitButton };

public:
    WizardFrame(ServiceMenuBackend *aBackend, QWidget *aParent = 0);
    ~WizardFrame();

    void initialize();
    void shutdown();

    void setPage(const QString &aContext, WizardPageBase *aPage, bool aCanCache = true);
    void
    setupDecoration(const QString &aStage, const QString &aStageName, const QString &aStageHowto);
    void setupControl(Control aControl,
                      bool aEnabled,
                      const QString &aTitle = "",
                      const QString &aContext = "",
                      bool aCanCache = true);
    void commitPageChanges();
    void setStatus(const QString &aStatus);

signals:
    void changePage(const QString &aContext);

private slots:
    void onChangePage(const QString &aContext);
    void onPageEvent(const QString &aContext, bool aFlag);
    void onControlEvent(const QString &aContext);
    void onExit();

    void onAbstractButtonClicked();

private:
    void showPage(const QString &aContext, WizardPageBase *aPage);
    void hidePage(const QString &aContext, WizardPageBase *aPage);

    void connectAllAbstractButtons(QWidget *aParentWidget);
    QString stageIndex(const QString &aContext);

private:
    struct CacheItem {
        WizardPageBase *page;

        struct ControlItem {
            Control control;
            QString title;
            QString context;
            bool enabled;
        };

        QString stage;
        QString stageName;
        QString stageHowto;

        QList<ControlItem> controls;
    };

    typedef QMap<QString, CacheItem> TPageMap;

    TPageMap m_Pages;
    QSignalMapper m_SignalMapper;

    QString m_CurrentContext;
    WizardPageBase *m_CurrentPage;

    ServiceMenuBackend *m_Backend;
};

//----------------------------------------------------------------------------
