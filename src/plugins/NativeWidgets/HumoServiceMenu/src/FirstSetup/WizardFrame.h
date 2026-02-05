/* @file Окно визарда. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSignalMapper>
#include <QtWidgets/QWidget>
#include "ui_WizardFrame.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "WizardPage.h"

class HumoServiceBackend;

//----------------------------------------------------------------------------
class WizardFrame : public QWidget, protected Ui::WizardFrame
{
    Q_OBJECT

  public:
    enum Control
    {
        BackButton,
        ForwardButton,
        ExitButton
    };

  public:
    WizardFrame(HumoServiceBackend *aBackend, QWidget *aParent = 0);
    ~WizardFrame();

    void initialize();
    void shutdown();

    void setPage(const QString &aContext, WizardPageBase *aPage, bool aCanCache = true);
    void setupDecoration(const QString &aStage, const QString &aStageName, const QString &aStageHowto);
    void setupControl(Control aControl, bool aEnabled, const QString &aTitle = "", const QString &aContext = "",
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

  private:
    void showPage(const QString &aContext, WizardPageBase *aPage);
    void hidePage(const QString &aContext, WizardPageBase *aPage);
    QString stageIndex(const QString &aContext) const;
    void connectAllAbstractButtons(QWidget *aParentWidget);
    void onAbstractButtonClicked();

  private:
    HumoServiceBackend *mBackend;
    QString mCurrentContext;
    WizardPageBase *mCurrentPage;

    struct CacheItem
    {
        WizardPageBase *page;
    };

    QMap<QString, CacheItem> mPages;
    QSignalMapper mSignalMapper;
};

//----------------------------------------------------------------------------