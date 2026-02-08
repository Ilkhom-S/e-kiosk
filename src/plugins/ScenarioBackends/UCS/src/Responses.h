/* @file Реализация классов-ответов от EFTPOS. */

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include "Ucs.h"

namespace Ucs {
//---------------------------------------------------------------------------
class BaseResponse {
public:
    char m_Class;
    char m_Code;
    QString m_TerminalID;
    QByteArray m_Data;
    /// Статус объекта API в момент получения ответа
    APIState::Enum m_APIState;

public:
    BaseResponse(const BaseResponse &aResponse)
        : m_Class(aResponse.m_Class), m_Code(aResponse.m_Code), m_TerminalID(aResponse.m_TerminalID),
          m_Data(aResponse.m_Data) {}
    virtual ~BaseResponse() {}
    static QSharedPointer<BaseResponse> createResponse(QByteArray &aResponseBuffer);

    virtual bool isValid() { return false; }

protected:
    BaseResponse() { m_Class = m_Code = 0; }
    BaseResponse(const QByteArray &aResponseBuffer);
};

typedef QSharedPointer<BaseResponse> BaseResponsePtr;

//---------------------------------------------------------------------------
class ErrorResponse : public BaseResponse {
public:
    ErrorResponse(const BaseResponse &aResponse);
    virtual bool isValid() { return true; }

    QString getError() const;
    QString getErrorMessage() const;
};

//---------------------------------------------------------------------------
class ConsoleResponse : public BaseResponse {
public:
    ConsoleResponse(const BaseResponse &aResponse);
    virtual bool isValid() { return true; }

    QString getMessage() const;

private:
    QString m_Message;
};

//---------------------------------------------------------------------------
class LoginResponse : public BaseResponse {
public:
    LoginResponse(const BaseResponse &aResponse);
    virtual bool isValid() { return true; }

    // 0	EFTPOS устройство полностью готово к обработке транзакций по картам
    // 1	EFTPOS устройству требуется инкассация
    // 2	В EFTPOS устройстве закончилась бумага для печати
    QString getStatusCode() const;
    QString getTerminalID() const;
    bool needEncashment() const;

private:
    QString m_StatusCode;
};

//---------------------------------------------------------------------------
class PrintLineResponse : public BaseResponse {
public:
    PrintLineResponse(const BaseResponse &aResponse);
    virtual bool isValid() { return true; }

    bool isLast() const;
    QString getText() const;
};

//---------------------------------------------------------------------------
class GetStateResponse : public BaseResponse {
public:
    GetStateResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }

    bool isLast() const;
    int state() const;
    QString getName() const;
};

//---------------------------------------------------------------------------
class InitialResponse : public BaseResponse {
public:
    InitialResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class BreakResponse : public BaseResponse {
public:
    BreakResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }

    bool isComplete() const;
};

//---------------------------------------------------------------------------
class PINRequiredResponse : public BaseResponse {
public:
    PINRequiredResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class OnlineRequiredResponse : public BaseResponse {
public:
    OnlineRequiredResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class AuthResponse : public BaseResponse {
public:
    Operation::Enum m_Operation;
    quint32 m_TransactionSum;
    quint32 m_Currency;
    QDateTime m_Stamp;
    QString m_Merchant;
    QString m_RRN;
    QString m_Response;
    QString m_Confirmation;
    QString m_CardNumber;
    QString m_CardLabel;
    QString m_Message;

public:
    AuthResponse(const BaseResponse &aResponse);
    virtual bool isValid() { return true; }

    bool isOK() const;
    QString toString() const;
};

//---------------------------------------------------------------------------
class EncashmentResponse : public BaseResponse {
public:
    EncashmentResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class HoldResponse : public BaseResponse {
public:
    HoldResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class MessageResponse : public BaseResponse {
public:
    MessageResponse(const BaseResponse &aResponse);
    virtual bool isValid() { return true; }

    bool needEncashment() const;

private:
    QString m_StatusCode;
    QString m_CurrentTransactionCount;
    QString m_TimeUpload;
};

} // namespace Ucs

//---------------------------------------------------------------------------
