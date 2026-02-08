/* Реализация протокола с EFTPOS 3.0 компании UCS. */

#pragma once

#include <QtCore/QFutureWatcher>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>

#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Payment/Amount.h>
#include <SDK/PaymentProcessor/Scripting/IBackendScenarioObject.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include "DatabaseUtils.h"
#include "Ucs.h"

namespace PPSDK = SDK::PaymentProcessor;

class NetworkTaskManager;

//---------------------------------------------------------------------------
namespace Ucs {
extern const char LogName[];
extern const char ScriptObjectName[];
extern const char EncashmentTask[];

typedef void *(*EftpCreate)(char *szConfigPath);
typedef void (*EftpDestroy)(void *pvSelf);

typedef int (*FUN_IDLE)(void *pvData);
typedef int (*EftpDo)(
    void *pvSelf, char *pchInBuffer, char *pchOutBuffer, FUN_IDLE pfIdle, void *pvData);
} // namespace Ucs

//---------------------------------------------------------------------------
namespace Ucs {
class BaseResponse;
typedef QSharedPointer<BaseResponse> BaseResponsePtr;

class AuthResponse;
class UscEncashTask;

//---------------------------------------------------------------------------
/// API для работы с сервисом
class API : public SDK::PaymentProcessor::Scripting::IBackendScenarioObject, public ILogable {
    Q_OBJECT

    API(SDK::PaymentProcessor::ICore *aCore, ILog *aLog);

public:
    struct TResponse {
        int result;
        APIState::Enum state;
        QByteArray response;

        TResponse::TResponse() : result(-1), state(APIState::None) {}
        explicit TResponse::TResponse(int aResult,
                                      APIState::Enum aState,
                                      const QByteArray &aResponse)
            : result(aResult), state(aState), response(aResponse) {}
    };

public:
    static QSharedPointer<API> getInstance(SDK::PaymentProcessor::ICore *aCore, ILog *aLog);

public:
    virtual QString getName() const { return Ucs::ScriptObjectName; }

    bool setupRuntime(const QString &aRuntimePath);

    bool isReady() const;

public slots:
    /// Начать процедуру оплаты, включая атворизацию, инкассацию и тд
    bool enable(PPSDK::TPaymentAmount aAmount);

    /// Отсоединиться
    void disable();

    void eftpCleanup();

    /// EFTPOS запросы
    void login();

    void sale(double aAmount);
    void breakSale();
    void status();

    bool encashmentOK() { return !m_NeedEncashment; }
    void encashment(bool aOnDemand = true);
    /// Возвращает последнюю ошибку
    bool isOK() { return m_LastError.isEmpty(); }

    /// Слот для задачи планировщика
    void onEncashTaskFinished(const QString &aName, bool aComplete);

signals:
    void hold();
    void ready();
    void readyToCard();
    void error(const QString &aMessage);
    void breakComplete();
    void encashmentComplete();
    void pinRequired();
    void onlineRequired();
    void message(const QString &aMessage);
    void saleComplete(double aAmount,
                      int aCurrency,
                      const QString &aRRN,
                      const QString &aConfirmationCode);
    void doComplete(bool aLastLine);

private slots:
    TResponse send(const QByteArray &aRequest, bool aWaitOperationComplete = true);
    void onResponseFinished();
    void onReceiptPrinted(int aJobIndex, bool aError);

private:
    void doEncashment(bool aOnDemand, bool aSkipPrintReceipt = false);
    QByteArray makeRequest(char aClass, char aCode, const QByteArray &aData = QByteArray());
    QString translateErrorMessage(const QString &aError, const QString &aMessage) const;

    void printAllEncashments();

    void killOldUCSProcess();

private:
    /// Обработчики ответов сервера
    bool isErrorResponse(BaseResponsePtr aResponse);
    bool isLoginResponse(BaseResponsePtr aResponse);
    bool isPrintLineResponse(BaseResponsePtr aResponse);
    bool isConsoleResponse(BaseResponsePtr aResponse);
    bool isEnchashmentResponse(BaseResponsePtr aResponse);
    bool isBreakResponse(BaseResponsePtr aResponse);
    bool isInitialResponse(BaseResponsePtr aResponse);
    bool isPINRequiredResponse(BaseResponsePtr aResponse);
    bool isOnlineRequiredResponse(BaseResponsePtr aResponse);
    bool isAuthResponse(BaseResponsePtr aResponse);
    bool isHoldResponse(BaseResponsePtr aResponse);
    bool isMessageResponse(BaseResponsePtr aResponse);

private:
    void printReceipt();
    void saveEncashmentReport();

private:
    SDK::PaymentProcessor::ICore *m_Core;
    SDK::PaymentProcessor::TerminalSettings *m_TerminalSettings{};
    QString m_TerminalID;
    QDateTime m_LoggedIn{};
    bool m_LastLineReceived;

    QStringList m_CurrentReceipt{};

    bool isLoggedIn() const;
    bool isLoggedInExpired() const;

private:
    QString m_LastError;

protected:
    int m_TimerEncashID;
    void timerEvent(QTimerEvent *aEvent);

private:
    bool m_RuntimeInit;
    EftpCreate m_EftpCreate;
    EftpDestroy m_EftpDestroy;
    EftpDo m_EftpDo;
    void *m_PySelf; // Управляющий объект библиотеки

    QFutureWatcher<TResponse> m_ResponseWatcher{};
    APIState::Enum m_TerminalState;
    SDK::PaymentProcessor::TPaymentAmount m_MaxAmount;
    bool m_NeedEncashment;
    bool m_NeedPrintAllEncashmentReports;
    UcsDB::DatabaseUtils m_Database;
    QSharedPointer<Ucs::AuthResponse> m_AuthResponse{};
    QMap<int, UcsDB::Encashment> m_EncashmentInPrint{};

private:
    UscEncashTask *m_EncashmentTask;
};

} // namespace Ucs
