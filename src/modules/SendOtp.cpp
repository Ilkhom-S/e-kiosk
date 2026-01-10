#include "SendOtp.h"

SendOtp::SendOtp(QObject *parent) : SendRequest(parent)
{
    senderName = "SEND_OTP";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_DomElement(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void SendOtp::resendRequest()
{
    emit emit_SendOtpResult("", "");
}

void SendOtp::setDataNote(const QDomNode& domElement)
{
    resultCode = "";
    otpId = "";

    //Парсим данные
    parcerNote(domElement);

    if (resultCode != "") {
        //Тут отправляем сигнал с балансом
        emit emit_SendOtpResult(resultCode, otpId);
        return;
    }

    emit emit_SendOtpResult("","");
    return;
}

void SendOtp::parcerNote(const QDomNode& domElement)
{
    //Необходимо отпарсить документ
    QDomNode domNode = domElement.firstChild();

    while(!domNode.isNull()) {
        if (domNode.isElement()) {

            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            if (strTag == "resultCode") {
                resultCode = domElement.text();
            }

            if (strTag == "otp_id") {
                otpId = domElement.text();
            }
        }

        parcerNote(domNode);
        domNode = domNode.nextSibling();
    }
}

void SendOtp::sendOtpRequest(QString account)
{
    QString header_xml = getHeaderRequest(Request::Type::SendOtp);

    QString footer_xml = getFooterRequest();

    QString xml = QString(  header_xml
                          + "<account>%1</account>\n"
                          + footer_xml).arg(account);

    sendRequest(xml, 20000);
}
