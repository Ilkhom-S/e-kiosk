#ifndef MAINPAGELOADER_H
#define MAINPAGELOADER_H

#include <QWidget>
#if defined(HAS_WEBKIT) || defined(HAS_WEBENGINE)
#if defined(HAS_WEBKIT)
#include <QtWebKitWidgets>
#elif defined(HAS_WEBENGINE)
#include <QtWebEngineWidgets>
#include <QWebChannel>
#endif
#endif
#include <QSound>
#include "QSqlDatabase"
#include <QFile>
#include <QFileInfo>

namespace Lang {
    const QString RU = "ru";
    const QString TJ = "tj";
    const QString UZ = "uz";
    const QString EN = "en";
}

namespace PageIn {
    enum page {
        Main                = 0,
        SelectGroup         = 1,
        CheckPay            = 2,
        SelectServices      = 3,
        InputNumber         = 4,
        InputSum            = 5,
        PrintDialog         = 6,
        AfterPrint          = 7,
        LockTerminal        = 8,
        Information         = 9,
    };

    enum sec {
        sMain                = 0,
        sSelectGroup         = 60000,
        sCheckPay            = 150000,
        sSelectServices      = 60000,
        sInputNumber         = 90000,
        sInputSum            = 120000,
        sPrintDialog_1       = 15000,
        sPrintDialog_2       = 8000,
        sAfterPrint          = 8000,
        sWebPage             = 180000
    };
}

namespace ConstData {

    const QString companyName = "Humo";
    const QString version     = "3.2.3";

    namespace Path {
        const QString Config   = "assets/config/";
        const QString Font     = "assets/fonts/";
        const QString Sound    = "assets/sound/";
        const QString Styles   = "assets/styles/";
        const QString Nominals = "assets/nominals/";
    }

    namespace FileName {
        const QString StyleQSS          = "style.qss";
        const QString Settings          = "settings.ini";
        const QString DBName            = "database.db";
        const QString DBFile            = "updater.db";
    }
}

namespace Sound
{
    const QString sGetOperator      = "01.wav";
    const QString sInputNumber      = "02.wav";
    const QString sInputMoney       = "03.wav";
    const QString sThanksFull       = "04.wav";
    const QString sThanks           = "05.wav";
    const QString sPrintChek        = "06.wav";
    const QString sGetCategory      = "07.wav";
}

namespace Ui {
    class MainPageLoader;
}

//enum HttpMethod {
//    GET   = 1,
//    POST  = 2,
//};

class MainPageLoader : public QWidget
{
    Q_OBJECT

public:
    explicit MainPageLoader(QWidget *parent = 0);
    ~MainPageLoader();

    QMap<int , QMap<QString , QString> > xmlServices;
    QMap<int , QMap<QString , QString> > xmlGroup;

    QVariantList services;
    QVariantList categories;

    QVariantMap terminalInfo;

    PageIn::page getStepByStepPage();

    bool moneyExistInPay();
    void loadHtmlPage(PageIn::page page);
    void gotoPage(PageIn::page page);
    void loadMainPage();
    void payToWhenBoxOpen();
    void setTerminalInfo(QString data);
    void setDbName(QSqlDatabase &dbName);
    void setTemplate(QString tpl);
    void interfaceCacheClear();
    void favoriteServicesInit();
    void langDefaultSet(QString lang);
    void inspectEnable();

    QVariantMap serviceMaxSum();
    QVariantList banners;

    QString lockReason;

    bool printerStatus;
    bool printDialogShow;
    bool connectionIsUp;

    QString originalNumber;
    QString afterMaskNumber;

private:
    // Web
#if defined(HAS_WEBKIT) || defined(HAS_WEBENGINE)
#if defined(HAS_WEBKIT)
    QWebView *webView;
#elif defined(HAS_WEBENGINE)
    QWebEngineView *webView;
#endif
#endif

private:
    Ui::MainPageLoader *ui;

    QSqlDatabase db;

    QTimer *payTimer;
    QTimer *goInputSum;

    QMap <int, QMap<QString, QString> > commissionMap;

    QVariantMap _serviceCurrent;
    QVariantMap _categoryCurrent;

    QString tpl;

    int nominalInserted;
    QString nominalDenomination;
    double nominalCash;
    double nominalAmount;

    double gblRatioPrv;
    double gblCmsSum;
    bool btnPayClck;

    QString gblNowTranzaction;

    void clearNominalData();
    void getCommissionMap();
    void getSumToFromMinusCommis(double amountFrom);
    double getMoneyToFromAll(int cmsType, double from, double value);
    double getRatioFromIn(double from, double to);

    void playSound(QString fileName);
    void playSoundRepeet(int page);
    void setPageStatus(PageIn::page page);

    QFileInfo mainWebPage;
    QFileInfo lockWebPage;

    PageIn::page currentPage;

    QString _lang;
    QString _langDefault;

    QString gblTrnOnlineCheck;
    QString gblOtpId;

    QString normalizeField(QString value);

    QVariantList favoriteList;
    QVariantList _precheckItems;

    QVariantMap fieldsData;
    QVariantMap paramPrecheck;
    QVariantMap extraInfo;
    QVariantMap extraInfoGet();

    QVariantMap orzuInfo;

    void orzuTranshCreate();

private slots:
    void btnGotoInputSumClc();

public slots:
    void populateJavaScriptWindowObject();

    void adminOpen();
    void gotoPageMain(bool disableValidator = false);
    void gotoPageInfo();
    void gotoPageInputAccount(int serviceId);
    void gotoPageInsertNominal(QString account, QString accountDesign, int serviceId);
    void gotoPageServices(int categoryId);
    void gotoPageCategories();
    void gotoPageInputFromPayment(const int serviceId = 0);
    void paymentConfirm();
    void userInfoCheck(QString account, QString idPrv);
    void langSet(QString lng);
    void playNumpadSound(QString fileName);
    void receiptPrint(bool withSound);
    void receiptSend(QString phone);
    void otpSend(QString account);
    void otpConfirm(QString otp);
    void setCheckOnlineResult(QString resultCode, QString status, QString message, QVariantList items);
    void sendReceiptResult(QString resultCode, QString trn, QString status);
    void sendOtpResult(QString resultCode, QString otpId);
    void confirmOtpResult(QString resultCode);
    void notifyRouteSet(QString value);
    void jsonResponseError(QString error, QString requestName);
    void jsonResponseSuccess(QVariantMap response, QString requestName);

    bool connectionCheck();

    void precheck(QString idPrv, QString account, double amount);
    void orzuUserDefine(QString inn);
    void orzuOtpResend();
    void orzuOtpConfirm(QString inn, QString otp);
    void orzuConditionsGet(int amount);
    bool orzuConditionIsValid(int amount, int term);
    void panCheck(QString pan);

    QString defaultLang();
    QString lang();
    QString theme();
    QVariantList precheckItems();
    QString showPrintDialog();
    QString getPrinterStatus();
    QString getLockReason();
    QString paymentId();
    QVariantMap homeData();
    QVariantList favoriteServices();
    QVariantList serviceList(const int categoryId = 0);
    QVariantList categoryList();
    QVariantMap serviceInfo(int id);
    QVariantMap serviceCurrent();
    QVariantMap categoryCurrent();

    void    setFieldData(QString key, QString val);
    QString getFieldData(QString key);

    QVariantMap orzuData();
    QVariantMap terminalData();

    QString commissionProfile();

    void inputNominal(int nominal, bool coin = false);
    void showHideReturnNominal(bool status);

signals:
    void validator_activate(bool status);
    void emit_pay_new(QVariantMap payment);
    void emit_update_pay(QVariantMap payment);
    void emit_confirm_pay(QString tranz_id, bool print);
    void emit_print_pay(QString tranz_id);
    void emit_getUserInfo(QString account, QString prvId);
    void emit_checkOnline(QString trn, QString prvId, QString account, double amount, QVariantMap extraInfo = QVariantMap());
    void emit_sendReceipt(QString trn, QString notify);
    void emit_otpSend(QString account);
    void emit_otpConfirm(QString otpId, QString otpValue);
    void emit_sendJsonRequest(QJsonObject json, QString url, QString requestName, int method, int timeout, QVariantMap header = QVariantMap());
    void emit_toLoging(int sts, QString name, QString text);
    void emit_openAvtorizationDialog();
    void emit_sendStatusValidator();
    void emit_updaterLock(bool updaterLock);
    void emit_checkStatus54();
};

#endif // MAINPAGELOADER_H
