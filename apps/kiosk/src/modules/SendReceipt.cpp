#include "SendReceipt.h"

SendReceipt::SendReceipt(QObject *parent) : SendRequest(parent)
{
    senderName = "SEND_RECEIPT";

    connect(this,SIGNAL(emit_ErrResponse()),this,SLOT(resendRequest()));
    connect(this,SIGNAL(emit_DomElement(QDomNode)),this,SLOT(setDataNote(QDomNode)));
}


void SendReceipt::resendRequest()
{
    if (countAllRep < 3) {
        //Повторная отправка
        QTimer::singleShot(20000 * countAllRep, this, SLOT(sendRequestRepeet()));
    } else {
        emit emitSendReceiptResult("","","");
    }
}

void SendReceipt::sendRequestRepeet()
{
    countAllRep ++;
    sendRequest(requestXml, 15000);
}

void SendReceipt::setDataNote(const QDomNode& domElement)
{
    getData = false;
    resultCode = "";

    //Парсим данные
    parcerNote(domElement);

    if (resultCode != "") {
        //Обнуляем счетчик
        this->countAllRep = 0;

        //Тут отправляем сигнал с балансом
        emit emitSendReceiptResult(resultCode, this->trn, this->status);
    }
}

void SendReceipt::parcerNote(const QDomNode& domElement)
{
    //Необходимо отпарсить документ
    QDomNode domNode = domElement.firstChild();

    while(!domNode.isNull())
    {
        if (domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            if (strTag == "resultCode") {
                resultCode = domElement.text();
            }

            if (strTag == "receipt") {
                trn    = domElement.attribute("tran_id","");
                status = domElement.attribute("resultCode","");
            }
        }

        parcerNote(domNode);
        domNode = domNode.nextSibling();
    }
}

void SendReceipt::sendReceiptRequest(QString trn, QString notify)
{
    QString header_xml = getHeaderRequest(Request::Type::SendReceipt);

    QString footer_xml = getFooterRequest();

    QString xml = QString(  header_xml
                          + "<receipts count=\"1\">"
                            "<receipt tranID=\"%1\" notify=\"%2\"/>"
                          + "</receipts>"
                          + footer_xml).arg(trn).arg(notify);

    requestXml = xml;

    sendRequest(xml, 15000);
}
