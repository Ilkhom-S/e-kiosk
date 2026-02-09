#include "ConfirmOtp.h"

Confirm_Otp::Confirm_Otp(QObject *parent) : SendRequest(parent) {
    senderName = "SEND_OTP";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void Confirm_Otp::resendRequest() {
    emit emit_Confirm_OtpResult("");
}

void Confirm_Otp::setDataNote(const QDomNode &domElement) {
    resultCode = "";

    // Парсим данные
    parcerNote(domElement);

    if (resultCode != "") {
        // Тут отправляем сигнал с балансом
        emit emit_Confirm_OtpResult(resultCode);
        return;
    }

    emit emit_Confirm_OtpResult("");
    return;
}

void Confirm_Otp::parcerNote(const QDomNode &domElement) {
    // Необходимо отпарсить документ
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull()) {
        if (domNode.isElement()) {

            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            if (strTag == "resultCode") {
                resultCode = domElement.text();
            }
        }

        parcerNote(domNode);
        domNode = domNode.nextSibling();
    }
}

void Confirm_Otp::confirm_OtpRequest(QString otpId, QString otpValue) {
    QString headerXml = getHeaderRequest(Request::Type::Confirm_Otp);

    QString footerXml = getFooterRequest();

    QString xml =
        QString(headerXml + "<otp_id>%1</otp_id>\n" + "<otp_value>%2</otp_value>\n" + footerXml)
            .arg(otpId, otpValue);

    sendRequest(xml, 20000);
}
