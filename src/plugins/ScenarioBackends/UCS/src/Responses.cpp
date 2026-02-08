/* @file Реализация классов-ответов от EFTPOS Ucs. */

#include "Responses.h"

#include <QtCore/QString>
#include <QtCore/QTextCodec>

namespace {
const quint32 EncashmentThreshold = 2;
} // namespace

namespace Ucs {

BaseResponsePtr BaseResponse::createResponse(QByteArray &aResponseBuffer) {
    if (aResponseBuffer.size() < 14) {
        return QSharedPointer<BaseResponse>(nullptr);
    }

    if (aResponseBuffer.size() == aResponseBuffer.count('\0')) {
        aResponseBuffer.clear();
        return QSharedPointer<BaseResponse>(nullptr);
    }

    BaseResponse response;
    response.m_Class = aResponseBuffer[0];
    response.m_Code = aResponseBuffer[1];
    response.m_TerminalID = aResponseBuffer.mid(2, 10);

    bool ok = true;
    int dataLength = QString::from_Latin1(aResponseBuffer.mid(12, 2)).toInt(&ok, 16);
    if (ok && dataLength) {
        response.m_Data = aResponseBuffer.mid(14, dataLength);
    }

    aResponseBuffer.remove(0, 14 + dataLength);

    switch (response.m_Class) {
    case Ucs::Class::Service:
        switch (response.m_Code) {
        case Ucs::Encashment::CodeResponse:
            return QSharedPointer<BaseResponse>(new EncashmentResponse(response));
        }
        break;
    case Ucs::Class::Session:
        switch (response.m_Code) {
        case Ucs::Login::CodeResponse:
            return QSharedPointer<BaseResponse>(new LoginResponse(response));
        case Ucs::PrintLine::CodeRequest:
            return QSharedPointer<BaseResponse>(new PrintLineResponse(response));
        case Ucs::Break::CodeResponse:
            return QSharedPointer<BaseResponse>(new BreakResponse(response));
        case Ucs::Information::CodeResponse:
            return QSharedPointer<MessageResponse>(new MessageResponse(response));
        }
        break;

    case Ucs::Class::Accept:
        switch (response.m_Code) {
        case Ucs::Initial::CodeResponse:
            return QSharedPointer<BaseResponse>(new InitialResponse(response));
        case Ucs::Sale::PinRequired:
            return QSharedPointer<BaseResponse>(new PINRequiredResponse(response));
        case Ucs::Sale::OnlineRequired:
            return QSharedPointer<BaseResponse>(new OnlineRequiredResponse(response));
        case Ucs::Error::Code:
            return QSharedPointer<BaseResponse>(new ErrorResponse(response));
        case Ucs::ConsoleMessage::CodeResponse:
            return QSharedPointer<BaseResponse>(new ConsoleResponse(response));
        case Ucs::Hold::CodeResponse:
            return QSharedPointer<BaseResponse>(new HoldResponse(response));
        }
        break;

    case Ucs::Class::AuthResponse:
        switch (response.m_Code) {
        case Ucs::Auth::Response:
            return QSharedPointer<BaseResponse>(new AuthResponse(response));
        }
        break;
    }

    return QSharedPointer<BaseResponse>(new BaseResponse(response));
}

//---------------------------------------------------------------------------
ErrorResponse::ErrorResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}

//---------------------------------------------------------------------------
QString ErrorResponse::getError() const {
    if (m_Data.size() >= 2) {
        return QString::from_Latin1(m_Data.left(2));
    }

    return QString();
}

//---------------------------------------------------------------------------
QString ErrorResponse::getErrorMessage() const {
    if (m_Data.size() > 2) {
        return QString::from_Latin1(m_Data.mid(2));
    }

    return QString();
}

//---------------------------------------------------------------------------
PrintLineResponse::PrintLineResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {}

//---------------------------------------------------------------------------
bool PrintLineResponse::isLast() const {
    return m_Data.size() > 0 && m_Data[0] == '1';
}

//---------------------------------------------------------------------------
QString PrintLineResponse::getText() const {
    return (m_Data.size() > 1) ? QTextCodec::codecForName("Windows-1251")->toUnicode(m_Data.mid(1))
                              : "";
}

//---------------------------------------------------------------------------
bool GetStateResponse::isLast() const {
    return m_Data.size() > 0 && m_Data[0] == '1';
}

//---------------------------------------------------------------------------
int GetStateResponse::state() const {
    if (m_Data.size() > 2) {
        return QString::from_Latin1(m_Data.mid(1, 2)).toInt(nullptr, 16);
    }

    return 0xff;
}

//---------------------------------------------------------------------------
QString GetStateResponse::getName() const {
    return QString::from_Latin1(m_Data.mid(3));
}

//---------------------------------------------------------------------------
bool BreakResponse::isComplete() const {
    return m_Data.size() && m_Data[0] == '0';
}

//---------------------------------------------------------------------------
AuthResponse::AuthResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {
    int index = 0;

    auto readTo1B = [&](QString &aString, QTextCodec *aCodec) {
        for (; m_Data.size() > index && m_Data.at(index) != 0x1b; index++) {
            char c = m_Data.at(index);
            aString.append(aCodec ? aCodec->toUnicode(&c, 1).at(0) : c);
        }
        index++;
    };

    if (m_Data.size()) {
        m_Operation = static_cast<Ucs::Operation::Enum>(m_Data.at(index));
        index++;
        m_TransactionSum = QString::from_Latin1(m_Data.mid(index, 12)).toUInt();
        index += 12;
        m_Currency = QString::from_Latin1(m_Data.mid(index, 3)).toUInt();
        index += 3;
        m_Stamp = QDateTime::from_String(QString::from_Latin1(m_Data.mid(index, 14)), "yyyyMMddhhmmss");
        index += 14;
        m_Merchant = QString::from_Latin1(m_Data.mid(index, 15));
        index += 15;
        m_RRN = QString::from_Latin1(m_Data.mid(index, 12));
        index += 12;
        m_Response = QString::from_Latin1(m_Data.mid(index, 2));
        index += 2;

        readTo1B(m_Confirmation, nullptr);
        readTo1B(m_CardNumber, nullptr);
        readTo1B(m_CardLabel, nullptr);
        readTo1B(m_Message, QTextCodec::codecForName("windows-1251"));
    }
}

//---------------------------------------------------------------------------
bool AuthResponse::isOK() const {
    return m_Response == "00";
}

//---------------------------------------------------------------------------
QString AuthResponse::toString() const {
    return m_CardLabel + "|" + m_Message;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
LoginResponse::LoginResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {
    m_StatusCode = m_Data.size() == 2 ? QString::from_Latin1(m_Data.mid(1, 1)) : "";
}

//---------------------------------------------------------------------------
QString LoginResponse::getStatusCode() const {
    return m_StatusCode;
}

//---------------------------------------------------------------------------
QString LoginResponse::getTerminalID() const {
    return m_TerminalID;
}

//---------------------------------------------------------------------------
bool LoginResponse::needEncashment() const {
    return m_StatusCode == "1";
}

//---------------------------------------------------------------------------
ConsoleResponse::ConsoleResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {
    auto readTo1B = [](const QByteArray &aBuffer) -> QString {
        QString result;
        QTextCodec *aCodec = QTextCodec::codecForName("windows-1251");
        for (int i = 0; i < aBuffer.size() && aBuffer[i]; i++) {
            char c = aBuffer.at(i);
            result.append(aCodec->toUnicode(&c, 1).at(0));
        }

        return result;
    };

    m_Message = readTo1B(m_Data);
}

//---------------------------------------------------------------------------
QString ConsoleResponse::getMessage() const {
    return m_Message;
}

//---------------------------------------------------------------------------
MessageResponse::MessageResponse(const BaseResponse &aResponse) : BaseResponse(aResponse) {
    QByteArray response = m_Data.right(6);

    if (response.size() == 6) {
        m_CurrentTransactionCount = response.left(3);
        m_TimeUpload = response.mid(3, 2);
        m_StatusCode = response.right(1);
    }
}

//---------------------------------------------------------------------------
bool MessageResponse::needEncashment() const {
    if (m_StatusCode == "1") {
        return true;
    }

    if (!m_TimeUpload.isEmpty() && m_TimeUpload.toUInt() < ::EncashmentThreshold) {
        return true;
    }

    if (!m_CurrentTransactionCount.isEmpty() &&
        m_CurrentTransactionCount.toUInt() < ::EncashmentThreshold) {
        return true;
    }

    return false;
}

} // namespace Ucs

//---------------------------------------------------------------------------
