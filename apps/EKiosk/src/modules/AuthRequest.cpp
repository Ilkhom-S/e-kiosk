#include "AuthRequest.h"

AuthRequest::AuthRequest(QObject *parent) : SendRequest(parent) {
    senderName = "AUTH";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(errorResponse()));
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void AuthRequest::sendAuthRequest(QString login, QString otp, QString hash, QString cid) {
    this->login = login;

    cid = cid.isEmpty() ? "" : QString("<cid>%1</cid>\n").arg(cid);

    QString footerXml = getFooterRequest();

    QString xml = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                          "<request>"
                          "<login>%1</login>\n"
                          "<otp>%2</otp>\n"
                          "<hash>%3</hash>\n"
                          "%4" +
                          footerXml)
                      .arg(login, otp, hash, cid);

    //    if(Debugger) qDebug() << xml;
    sendRequest(xml, 20000);
}

void AuthRequest::errorResponse() {
    emit emitResult("", "", "", "");
}

void AuthRequest::setDataNote(const QDomNode &domElement) {
    resultCode = "";
    token = "";
    message = "";

    // Парсим данные
    parseNode(domElement);

    if (resultCode != "") {
        emit emitResult(resultCode, login, token, message);
        return;
    }

    emit emitResult("", "", "", "");
}

void AuthRequest::parseNode(const QDomNode &domElement) {
    // Необходимо отпарсить документ
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull()) {
        if (domNode.isElement()) {

            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            if (strTag == "resultCode") {
                resultCode = domElement.text();
            }

            if (strTag == "token") {
                token = domElement.text();
            }

            if (strTag == "message") {
                message = domElement.text();
            }
        }

        parseNode(domNode);
        domNode = domNode.nextSibling();
    }
}
