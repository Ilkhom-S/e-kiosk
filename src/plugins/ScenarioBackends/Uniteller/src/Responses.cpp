/* @file Реализация классов-ответов от EFTPOS Uniteller. */

#include "Responses.h"

#include <QtCore/QString>
#include <QtCore/QTextCodec>

namespace Uniteller {
BaseResponsePtr BaseResponse::createResponse(QByteArray &aResponseBuffer) {
    if (aResponseBuffer.size() < 14) {
        return QSharedPointer<BaseResponse>(nullptr);
    }

    BaseResponse response;
    response.m_Class = aResponseBuffer[0];
    response.m_Code = aResponseBuffer[1];
    response.m_TerminalID = aResponseBuffer.mid(2, 10);

    bool ok = true;
    int dataLength = QString::fromLatin1(aResponseBuffer.mid(12, 2)).toInt(&ok, 16);
    if (ok && dataLength) {
        response.m_Data = aResponseBuffer.mid(14, dataLength);
    }

    aResponseBuffer.remove(0, 14 + dataLength);

    switch (response.m_Class) {
    case Uniteller::Class::Session:
        switch (response.m_Code) {
        case Uniteller::Login::CodeResponse:
            return QSharedPointer<BaseResponse>(new LoginResponse(response));
        case Uniteller::PrintLine::Code:
            return QSharedPointer<BaseResponse>(new PrintLineResponse(response));
        case Uniteller::Break::CodeResponse:
            return QSharedPointer<BaseResponse>(new BreakResponse(response));
        case Uniteller::Auth::DeviceEvent:
            return QSharedPointer<BaseResponse>(new DeviceEventResponse(response));
        }
        break;

    case Uniteller::Class::Accept:
        switch (response.m_Code) {
        case Uniteller::Initial::CodeResponse:
            return QSharedPointer<BaseResponse>(new InitialResponse(response));
        case Uniteller::Sell::PinReqired:
            return QSharedPointer<BaseResponse>(new PINRequiredResponse(response));
        case Uniteller::Sell::OnlineReqired:
            return QSharedPointer<BaseResponse>(new OnlineRequiredResponse(response));
        case Uniteller::Error::Code:
            return QSharedPointer<BaseResponse>(new ErrorResponse(response));
        }
        break;

    case Uniteller::Class::AuthResponse:
        switch (response.m_Code) {
        case Uniteller::Auth::Response:
            return QSharedPointer<BaseResponse>(new AuthResponse(response));
        }
        break;

    case Uniteller::Class::Diagnostic:
        switch (response.m_Code) {
        case Uniteller::State::CodeResponse:
            return QSharedPointer<BaseResponse>(new GetStateResponse(response));
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
        return QString::fromLatin1(m_Data.left(2));
    }

    return QString();
}

//---------------------------------------------------------------------------
QString ErrorResponse::getErrorMessage() const {
    if (m_Data.size() > 2) {
        return QString::fromLatin1(m_Data.mid(2));
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
        return QString::fromLatin1(m_Data.mid(1, 2)).toInt(nullptr, 16);
    }

    return 0xff;
}

//---------------------------------------------------------------------------
QString GetStateResponse::getName() const {
    return QString::fromLatin1(m_Data.mid(3));
}

//---------------------------------------------------------------------------
DeviceEvent::Enum DeviceEventResponse::event() const {
    const QByteArray eventCode = m_Data.mid(2, 2);

    if (eventCode == "PE")
        return DeviceEvent::KeyPress;
    else if (eventCode == "CI")
        return DeviceEvent::CardInserted;
    else if (eventCode == "CC")
        return DeviceEvent::CardCaptured;
    else if (eventCode == "CO")
        return DeviceEvent::CardOut;

    return DeviceEvent::Unknown;
}

//---------------------------------------------------------------------------
char DeviceEventResponse::keyCode() const {
    return (m_Data.size() >= 5) ? m_Data[4] : 0;
}

//---------------------------------------------------------------------------
KeyCode::Enum DeviceEventResponse::key() const {
    switch (keyCode()) {
    case 'T':
        return KeyCode::Timeout;
    case 'N':
        return KeyCode::Numeric;
    case 'B':
        return KeyCode::Clear;
    case 'C':
        return KeyCode::Cancel;
    case 'D':
        return KeyCode::Enter;
    default:
        return KeyCode::Unknown;
    }
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
        m_Operation = static_cast<Uniteller::Operation::Enum>(m_Data.at(index));
        index++;
        m_TransactionSumm = QString::fromLatin1(m_Data.mid(index, 12)).toUInt();
        index += 12;
        m_Currency = QString::fromLatin1(m_Data.mid(index, 3)).toUInt();
        index += 3;
        m_Stamp =
            QDateTime::from_String(QString::fromLatin1(m_Data.mid(index, 14)), "yyyyMMddhhmmss");
        index += 14;
        m_Merchant = QString::fromLatin1(m_Data.mid(index, 15));
        index += 15;
        m_RRN = QString::fromLatin1(m_Data.mid(index, 12));
        index += 12;
        m_Response = QString::fromLatin1(m_Data.mid(index, 2));
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

} // namespace Uniteller
