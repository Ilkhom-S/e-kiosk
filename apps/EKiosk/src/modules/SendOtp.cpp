#include "SendOtp.h"

SendOtp::SendOtp(QObject *parent) : SendRequest(parent) {
    senderName = "SEND_OTP";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDom_Node)), this, SLOT(setDataNote(QDom_Node)));
}

void SendOtp::resendRequest() {
    emit emit_SendOtpResult("", "");
}

void SendOtp::setDataNote(const QDom_Node &dom_Element) {
    resultCode = "";
    otpId = "";

    // Парсим данные
    parcerNote(dom_Element);

    if (resultCode != "") {
        // Тут отправляем сигнал с балансом
        emit emit_SendOtpResult(resultCode, otpId);
        return;
    }

    emit emit_SendOtpResult("", "");
    return;
}

void SendOtp::parcerNote(const QDom_Node &dom_Element) {
    // Необходимо отпарсить документ
    QDom_Node dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {

            QDom_Element dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            if (strTag == "resultCode") {
                resultCode = dom_Element.text();
            }

            if (strTag == "otp_id") {
                otpId = dom_Element.text();
            }
        }

        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void SendOtp::sendOtpRequest(QString account) {
    QString header_xml = getHeaderRequest(Request::Type::SendOtp);

    QString footer_xml = getFooterRequest();

    QString xml = QString(header_xml + "<account>%1</account>\n" + footer_xml).arg(account);

    sendRequest(xml, 20000);
}
