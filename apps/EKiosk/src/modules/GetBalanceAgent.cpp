#include "GetBalanceAgent.h"

GetBalanceAgent::GetBalanceAgent(QObject *parent) : SendRequest(parent) {
    senderName = "AGENT_BALANCE";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDom_Node)), this, SLOT(setDataNote(QDom_Node)));
}

void GetBalanceAgent::resendRequest() {
    emit emit_BalanceAgent("---", "---");
}

void GetBalanceAgent::setDataNote(const QDom_Node &dom_Element) {
    resultCode = false;
    getData = false;
    balance = "resultCode";
    overdraft = "resultCode";

    // Парсим данные
    parcerNote(dom_Element);

    if (balance != "resultCode" && overdraft != "resultCode") {
        // Тут отправляем сигнал с балансом
        emit emit_BalanceAgent(balance, overdraft);
        return;
    }

    emit emit_BalanceAgent("---", "---");
}

void GetBalanceAgent::parcerNote(const QDom_Node &dom_Element) {
    // Необходимо отпарсить документ
    QDom_Node dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {

            QDom_Element dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            // if(Debugger) qDebug() << strTag + " " + dom_Element.text();

            // Данные о дилере
            if (strTag == "balance") {
                balance = dom_Element.text();
            }

            if (strTag == "overdraft") {
                overdraft = dom_Element.text();
            }
        }
        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void GetBalanceAgent::sendDataRequest() {
    QString header_xml = getHeaderRequest(Request::Type::GetBalance);
    ;

    QString footer_xml = getFooterRequest();

    QString xml = header_xml + footer_xml;

    sendRequest(xml, 20000);
}
