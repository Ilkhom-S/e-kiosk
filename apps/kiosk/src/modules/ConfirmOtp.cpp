// Project
#include "ConfirmOtp.h"

ConfirmOtp::ConfirmOtp(QObject *parent) : SendRequest(parent) {
    senderName = "SEND_OTP";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_DomElement(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void ConfirmOtp::resendRequest() {
    emit emit_ConfirmOtpResult("");
}

void ConfirmOtp::setDataNote(const QDomNode &domElement) {
    resultCode = "";

    // Парсим данные
    parcerNote(domElement);

    if (resultCode != "") {
        // Тут отправляем сигнал с балансом
        emit emit_ConfirmOtpResult(resultCode);
        return;
    }

    emit emit_ConfirmOtpResult("");
    return;
}

void ConfirmOtp::parcerNote(const QDomNode &domElement) {
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

void ConfirmOtp::confirmOtpRequest(QString otpId, QString otpValue) {
    QString header_xml = getHeaderRequest(Request::Type::ConfirmOtp);

    QString footer_xml = getFooterRequest();

    QString xml =
        QString(header_xml + "<otp_id>%1</otp_id>\n" + "<otp_value>%2</otp_value>\n" + footer_xml).arg(otpId, otpValue);

    sendRequest(xml, 20000);
}
