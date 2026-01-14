#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtGui/QMovie>
#include <QtWidgets/QDialog>
#include <Common/QtHeadersEnd.h>

// Project
#include "adminbutton.h"
#include "createdialupconnection.h"
#include "keypud.h"
#include "selectcategorylogview.h"

namespace AdminLisTitle {
    enum ListTitle {
        lSettingsMain = 0,
        lSettingsDevice = 1,
        lSettingsConnection = 2,
        lSettingsPrinter = 3,
        lSettingsOther = 4,
        lSettingsLogInfo = 5
    };
} // namespace AdminLisTitle

namespace AdminCommand {
    enum AdminCmd {
        // Проверить баланс
        aCmdGetBalance = 0,
        // Количество новых платежей
        aCmdGetNewOperation = 1,
        // Список имеющихся инкасаций
        aCmdListAllIncash = 2,
        // Показать содержимое инкасации
        aCmdHtmlIncash = 3,
        // Сделать инкасацию
        aCmdExecIncashmant = 4,
        // Сделать инкасацию по дате
        aCmdExecDateIncash = 5,
        // Показать состояние асо
        aCmdShowAsoStatus = 6,
        // Показать клавиатуру
        aCmdShowKeyPud = 7,
        // Показать експлорер
        aCmdShowExplorer = 8,
        // Скрыть експлорер
        aCmdHideExplorer = 9,
        // Информация о купюроприемнике
        aCmdValidatorInform = 10,
        // Информация о принтере
        aCmdPrinterInform = 11,
        // Информация о модеме
        aCmdModemInform = 12,
        // Перезагрузить купюроприемник
        aCmdRestartValidator = 13,
        // Распечатать пробный чек
        aCmdPrintTestCheck = 14,
        // Перезагрузить модем
        aCmdRestartModem = 15,
        // Параметры поиска
        aCmdSearchParamRef = 16,
        // Информация действия с устройствами
        aCmdInfrmationPanel = 17,
        // Перезагрузить программу
        aCmdRestartApp = 18,
        // Перезагрузить АСО
        aCmdRestartASO = 19,
        // Выключить АСО
        aCmdShutDounASO = 20,
        // Сохранить параметры устройств
        aCmdSaveDeviceParam = 21,
        // Сохранить параметры устройств c перезагрузкой
        aCmdSaveDeviceParamR = 22,
        // Информация о сим карте
        aCmdSimInfoData = 23,
        // Список соединений
        aCmdRasConnlist = 24,
        // Перезагрузка ПО интервал
        aCmdErrorRasReb = 25,
        // Проверка соединения
        aCmdCheckConnect = 26,
        // Информация на странице соединения
        aCmdConnectInfo = 27,
        // Проверка данных SIM крты
        aCmdGetSimInfo = 28,
        // Создание соединения
        aCmdRasConnCreate = 29,
        // Проверяем активное рас соединение
        aCmdGetActiveDialup = 30,
        // Перезагружаем соединение
        aCmdRestartDialupCon = 31,
        // Параметры модема
        aCmdModemInfData = 32,
        // Параметры печати
        aCmdPrinterInfData = 33,
        // Параметры sms оповещений
        aCmdSmsSending = 34,
        // Сохранить параметры соединения
        aCmdSaveConnParam = 35,
        // Праметры win принтера
        aCmdWinPrinterParam = 36,
        // Праметры счетчика чека
        aCmdCounterCheckInf = 37,
        // Праметры счетчика чека сколько накрутило
        aCmdCounterCheckVal = 38,
        // Сохранение параметров печати
        aCmdSavePrinterParam = 40,
        // Параметры авторизации терминала
        aCmdAvtorizationTrmP = 41,
        // Остальные настройки
        aCmdOtherSettings = 42,
        // Получаем конфигурацию
        aCmdGetServices = 43,
        // Информация на странице регистрации
        aCmdInfoGetServices = 44,
        // Информация на странице регистрации
        aCmdSaveOtherSetting = 45,
        // Сохраняем параметры авторизации терминала
        aCmdSaveTrmNumSett = 46,
        // Сохраняем параметры авторизации пользователя
        aCmdSaveUserAvtoriza = 47,
        // Параметры сторожевика
        aCmdWDInform = 48,
        aCmdShowDesktop = 49,
        aCmdCoinAcceptorInf = 50
    };
} // namespace AdminCommand

namespace Ui {
    class AdminDialog;
}

class AdminDialog : public QDialog {
    Q_OBJECT

  public:
    explicit AdminDialog(QWidget *parent = 0);
    ~AdminDialog();

    void openThis();

    QVariantMap data;

    QString dateCollectParam;
    QString titleDataIncashment;
    QStringList dialupDevice;

    QVariantMap settings;

    QString printerName;
    QString printerPort;
    QString printerPortSpeed;
    QString printerComment;

    bool unlockNominalDuplicate();

    void showMsgDialog(QString title, QString text);

    void setLangList(QStringList langList);

    void authButtonSet(bool enable);

  public slots:
    void closeThis();
    void setDataToAdmin(int cmd, QVariantMap data);

  protected:
    void closeEvent(QCloseEvent *event);
    keyPud *KeyPud;

  private:
    Ui::AdminDialog *ui;
    SelectCategoryLogView *selectCategoryLogView;
    CreateDialupConnection *createDialupConnection;

    AdminButton *adminButtons;

    int Debuger;
    QStringList lstAdminListTitle;
    //    QStringList logLst;
    QTimer *closeTimer;
    QMovie *mveAnimateGB;
    QVariantMap deviceParam;
    QStringList connListInfData;

    void hideObjects();

  private slots:
    void printerNameChanged(int index);
    void getLogDataFromFile(QStringList &logLst, QString &all);
    void steckerClicked(int stk);
    // Проверка баланса
    void checkBalance();

    void getCollectDate(QString date);

    // Сделать инкасацию
    void doCollectExec();

    // Сделать инкасацию по дате
    void doCollectDateExec();

    void hideExplorer();

    void showExplorer();

    void showKeyPud();

    void restartValidator();

    void restartModem();

    void printTestCheck();

    void restartApp();
    void restartASO();
    void saveDeviceParam();
    void shutDounASO();
    void checkConnection();
    void getModemDataInfo();
    void saveTrmAutorizationData();
    void openSelectCategory();
    void go_to_up_log();
    void go_to_doun_log();
    void openLogInfoDate();
    void go_to_end_text_edit();
    void searchWithKeyParam();
    void SelectOptionsForSearch(bool SelectValidatorJam, bool SelectMoneyOut, bool SelectERROR, bool SelectPayDaemon,
                                bool SelectStatusAso, bool SelectStatusPrinter, bool SelectStatusValidator,
                                bool SelectConnectionState, bool SelectUpdater);
    void createNewConnection();
    void getDialupParam(QVariantMap val);
    void getActiveRasCon();
    void cmdRestartNet();
    void saveConnectionParam();
    void savePrinterParam();
    void getServices();
    void saveOtherSettings();
    void saveUserAutorizationData();
    void sendCharacter(QChar character);

  signals:
    void emit_execToMain(AdminCommand::AdminCmd cmd);
    void emit_unlockOpenAdminSts();
};

