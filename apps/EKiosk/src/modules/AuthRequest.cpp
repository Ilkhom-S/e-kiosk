#include "AuthRequest.h"

AuthRequest::AuthRequest(QObject *parent) : SendRequest(parent) {
    senderName = "AUTH";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(errorResponse()));
    connect(this, SIGNAL(emit_Dom_Element(QDom_Node)), this, SLOT(setDataNote(QDom_Node)));
}

void AuthRequest::sendAuthRequest(QString login, QString otp, QString hash, QString cid) {
    this->login = login;

    cid = cid.isEmpty() ? "" : QString("<cid>%1</cid>\n").arg(cid);

    QString footer_xml = getFooterRequest();

    QString xml = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                          "<request>"
                          "<login>%1</login>\n"
                          "<otp>%2</otp>\n"
                          "<hash>%3</hash>\n"
                          "%4" +
                          footer_xml)
                      .arg(login, otp, hash, cid);

    //    if(Debugger) qDebug() << xml;
    sendRequest(xml, 20000);
}

void AuthRequest::errorResponse() {
    emit emitResult("", "", "", "");
}

void AuthRequest::setDataNote(const QDom_Node &dom_Element) {
    resultCode = "";
    token = "";
    message = "";

    // Парсим данные
    parcerNote(dom_Element);

    if (resultCode != "") {
        emit emitResult(resultCode, login, token, message);
        return;
    }

    emit emitResult("", "", "", "");
}

void AuthRequest::parcerNote(const QDom_Node &dom_Element) {
    // Необходимо отпарсить документ
    QDom_Node dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {

            QDom_Element dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            if (strTag == "resultCode") {
                resultCode = dom_Element.text();
            }

            if (strTag == "token") {
                token = dom_Element.text();
            }

            if (strTag == "message") {
                message = dom_Element.text();
            }
        }

        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}
