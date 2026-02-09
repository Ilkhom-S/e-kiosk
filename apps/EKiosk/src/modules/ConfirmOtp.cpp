#include "ConfirmOtp.h"

Confirm_Otp::Confirm_Otp(QObject *parent) : SendRequest(parent) {
    senderName = "SEND_OTP";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void Confirm_Otp::resendRequest() {
    emit emit_Confirm_OtpResult("");
}

void Confirm_Otp::setDataNote(const QDomNode &dom_Element) {
    resultCode = "";

    // Парсим данные
    parcerNote(dom_Element);

    if (resultCode != "") {
        // Тут отправляем сигнал с балансом
        emit emit_Confirm_OtpResult(resultCode);
        return;
    }

    emit emit_Confirm_OtpResult("");
    return;
}

void Confirm_Otp::parcerNote(const QDomNode &dom_Element) {
    // Необходимо отпарсить документ
    QDomNode dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {

            QDomElement dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            if (strTag == "resultCode") {
                resultCode = dom_Element.text();
            }
        }

        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void Confirm_Otp::confirm_OtpRequest(QString otpId, QString otpValue) {
    QString header_xml = getHeaderRequest(Request::Type::Confirm_Otp);

    QString footer_xml = getFooterRequest();

    QString xml =
        QString(header_xml + "<otp_id>%1</otp_id>\n" + "<otp_value>%2</otp_value>\n" + footer_xml)
            .arg(otpId, otpValue);

    sendRequest(xml, 20000);
}
