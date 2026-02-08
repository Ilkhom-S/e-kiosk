/* @file Реализация классов-ответов от EFTPOS. */

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include "Uniteller.h"

namespace Uniteller {
//---------------------------------------------------------------------------
class BaseResponse {
public:
    char m_Class;
    char m_Code;
    QString m_TerminalID;
    QByteArray m_Data;

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

typedef QSharedPointer<ErrorResponse> ErrorResponsePtr;

//---------------------------------------------------------------------------
class LoginResponse : public BaseResponse {
public:
    LoginResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }
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
class DeviceEventResponse : public BaseResponse {
public:
    DeviceEventResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}
    virtual bool isValid() { return true; }

    DeviceEvent::Enum event() const;
    char keyCode() const;
    KeyCode::Enum key() const;
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
    quint32 m_TransactionSumm;
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

} // namespace Uniteller

//---------------------------------------------------------------------------
