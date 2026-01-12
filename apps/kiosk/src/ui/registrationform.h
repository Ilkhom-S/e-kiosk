#ifndef REGISTRATIONFORM_H
#define REGISTRATIONFORM_H

#include "createdialupconnection.h"
#include "keypud.h"

#include <QtCore/QPointer>
#include <QtCore/QTimer>

#include <QtGui/QKeyEvent>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

namespace Ui
{
	class RegistrationForm;
}

class RegistrationForm : public QDialog
{
	Q_OBJECT

public:
	enum Page
	{
		SelectTpl = 0,
		SearchDevice = 1,
		SelectConnection = 2,
		EnterAuthData = 3,
	};

	enum Status
	{
		Info = 0,
		Success = 1,
		Error = 2
	};

	enum SearchDev
	{
		search_validator = 0,
		search_printer = 1,
		search_modem = 2,
		search_watchdog = 3,
		search_coin_acceptor = 4,

		start_search = 0,
		device_found = 1,
		device_notfound = 2
	};

	explicit RegistrationForm(QWidget *parent = nullptr);
	~RegistrationForm();

	QStringList validatorList;
	QStringList coinAcceptorList;
	QStringList printerList;
	QStringList winprinterList;

	QStringList comPortList;
	QStringList connectionList;
	QStringList dialupDevice;

	void setDB(QSqlDatabase db);
	void setSearchDeviceParams(QVariantMap data);
	void deviceSearchResult(int device, int result, QString dev_name, QString dev_comment, QString dev_port);
	void deviceSearchFinished();
	void setDataListConnection(QStringList list);
	void setStatusText(int status, QString text);
	void clearStatusText();
	void setLoading(bool val);
	void setConnectionState(bool connectionOk, QString message = QString());
	void authResponse(const QString resultCode, const QString token, const QString message);

	void showMe();
	void closeMe();

private:
	Ui::RegistrationForm *ui;

	keyPud *KeyPud;

	QSqlDatabase db;

	QString tplStyleEnable;
	QString tplStyleDisable;

	bool searchClicked = false;

	void addPageData();
	void hideGroupDevice();
	bool saveDevice(int id_device, const QString &name_device, const QString &port, const QString &comment, int state);
	bool getDeviceFromDB(QVariantMap &devices);
	bool isValidConnectionType();
	bool isValidAuthInput();

	void sleep(const int msec);

	void showKeyPud();

private:
	CreateDialupConnection *createDialupConnection;

private slots:
	void showValidatorData(bool show);
	void showCoinAcceptorData(bool show);
	void showPrinterData(bool show);
	void showWatchdogData(bool show);
	void showModemData(bool show);

	void searchValidatorToggle(bool value);
	void searchCointAcceptorToggle(bool value);
	void searchPrinterToggle(bool value);
	void searchWatchdogToggle(bool value);
	void searchModemToggle(bool value);

	void validatorTest();
	void validatorSave();

	void coinAcceptorTest();
	void coinAcceptorSave();

	void printerTest();
	void printerSave();

	void watchdogTest();
	void watchdogSave();

	void modemTest();
	void modemSave();

	void tplCheck();
	void tplTJKSet();
	void tplUZBSet();

	//    void closeValidator();
	void deviceSearchStart();

	void btnSearchStartClc();

	void showDialUpParams(bool show);
	void createNewConnection();
	void connectionSelect(QString connection);

	void checkAuthInput(QString text);

	void sendCharacter(QChar character);

	void secretPassView(bool show);
	void secretPassConfirmView(bool show);

	void btnBackClicked();
	void btnNextClicked();
	void closeConfirm();

signals:
	void emitStartSearchDevices(QVariantMap data);
	void emitDeviceTest(int device, QString name, QString port, QString comment);
	void emitCreateNewConnection(QVariantMap data);
	void emitTpl(QString tpl, bool test);
	void emitStartToConnect(QString connectionName);
	void emitSaveDevice(int deviceId, QString deviceName, QString port, QString comment, int state);
	void emitSendAuthRequest(QString login, QString otp);
	void emitRegistrationData(QVariantMap data);
	void emitToLog(int sts, QString name, QString text);
};

#endif // REGISTRATIONFORM_H
