#include "SendReceipt.h"

SendReceipt::SendReceipt(QObject *parent) : SendRequest(parent) {
    senderName = "SEND_RECEIPT";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void SendReceipt::resendRequest() {
    if (countAllRep < 3) {
        // Повторная отправка
        QTimer::singleShot(20000 * countAllRep, this, SLOT(sendRequestRepeet()));
    } else {
        emit emitSendReceiptResult("", "", "");
    }
}

void SendReceipt::sendRequestRepeet() {
    countAllRep++;
    sendRequest(requestXml, 15000);
}

void SendReceipt::setDataNote(const QDomNode &dom_Element) {
    getData = false;
    resultCode = "";

    // Парсим данные
    parcerNote(dom_Element);

    if (resultCode != "") {
        // Обнуляем счетчик
        this->countAllRep = 0;

        // Тут отправляем сигнал с балансом
        emit emitSendReceiptResult(resultCode, this->trn, this->status);
    }
}

void SendReceipt::parcerNote(const QDomNode &dom_Element) {
    // Необходимо отпарсить документ
    QDomNode dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {
            QDomElement dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            if (strTag == "resultCode") {
                resultCode = dom_Element.text();
            }

            if (strTag == "receipt") {
                trn = dom_Element.attribute("tran_id", "");
                status = dom_Element.attribute("resultCode", "");
            }
        }

        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void SendReceipt::sendReceiptRequest(QString trn, QString notify) {
    QString header_xml = getHeaderRequest(Request::Type::SendReceipt);

    QString footer_xml = getFooterRequest();

    QString xml = QString(header_xml +
                          "<receipts count=\"1\">"
                          "<receipt tranID=\"%1\" notify=\"%2\"/>" +
                          "</receipts>" + footer_xml)
                      .arg(trn)
                      .arg(notify);

    requestXml = xml;

    sendRequest(xml, 15000);
}
