#include "GetBalanceAgent.h"

GetBalanceAgent::GetBalanceAgent(QObject *parent) : SendRequest(parent) {
    senderName = "AGENT_BALANCE";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void GetBalanceAgent::resendRequest() {
    emit emit_BalanceAgent("---", "---");
}

void GetBalanceAgent::setDataNote(const QDomNode &domElement) {
    resultCode = false;
    getData = false;
    balance = "resultCode";
    overdraft = "resultCode";

    // Парсим данные
    parseNode(domElement);

    if (balance != "resultCode" && overdraft != "resultCode") {
        // Тут отправляем сигнал с балансом
        emit emit_BalanceAgent(balance, overdraft);
        return;
    }

    emit emit_BalanceAgent("---", "---");
}

void GetBalanceAgent::parseNode(const QDomNode &domElement) {
    // Необходимо отпарсить документ
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull()) {
        if (domNode.isElement()) {

            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            // if(Debugger) qDebug() << strTag + " " + domElement.text();

            // Данные о дилере
            if (strTag == "balance") {
                balance = domElement.text();
            }

            if (strTag == "overdraft") {
                overdraft = domElement.text();
            }
        }
        parseNode(domNode);
        domNode = domNode.nextSibling();
    }
}

void GetBalanceAgent::sendDataRequest() {
    QString headerXml = getHeaderRequest(Request::Type::GetBalance);
    ;

    QString footerXml = getFooterRequest();

    QString xml = headerXml + footerXml;

    sendRequest(xml, 20000);
}
