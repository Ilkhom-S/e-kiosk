// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QJsonDocument>
#include <Common/QtHeadersEnd.h>

// Project
#include "CheckOnline.h"

CheckOnline::CheckOnline(QObject *parent) : SendRequest(parent)
{
    senderName = "CHECK_ONLINE";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_DomElement(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void CheckOnline::resendRequest()
{
    emit emit_CheckOnlineResult("", "", "", QVariantList());
}

void CheckOnline::setDataNote(const QDomNode &domElement)
{
    getData = false;
    resultCode = "";
    items.clear();

    // Парсим данные
    parcerNote(domElement);

    if (resultCode != "")
    {
        // Тут отправляем сигнал с балансом
        emit emit_CheckOnlineResult(resultCode, status, message, items);
        return;
    }

    emit emit_CheckOnlineResult("", "", "", QVariantList());
    return;
}

void CheckOnline::parcerNote(const QDomNode &domElement)
{
    // Необходимо отпарсить документ
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull())
    {
        if (domNode.isElement())
        {

            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            if (strTag == "resultCode")
            {
                resultCode = domElement.text();
            }

            if (strTag == "status")
            {
                status = domElement.text();
            }

            if (strTag == "message")
            {
                message = domElement.text();
            }

            if (strTag == "item")
            {
                QVariantMap item;
                item["label"] = domElement.attribute("label");
                item["value"] = domElement.attribute("value");

                items.append(item);
            }
        }

        parcerNote(domNode);
        domNode = domNode.nextSibling();
    }
}

void CheckOnline::sendCheckOnlineRequest(QString trn, QString prvId, QString account, double amount, QVariantMap param)
{
    QString header_xml = getHeaderRequest(Request::Type::CheckOnline);

    QString footer_xml = getFooterRequest();

    QString json = QJsonDocument::fromVariant(param).toJson(QJsonDocument::Compact);
    QString paramStr = param.isEmpty() ? "" : QString("<param>%1</param>\n").arg(json);
    QString amountStr = amount > 0 ? QString("<amount>%1</amount>\n").arg(amount) : "";

    QString xml = QString(header_xml + "<trn>%1</trn>\n" + "<prv_id>%2</prv_id>\n" + "<account>%3</account>\n" +
                          amountStr + paramStr + footer_xml)
                      .arg(trn, prvId, account);

    sendRequest(xml, 30000);
}
