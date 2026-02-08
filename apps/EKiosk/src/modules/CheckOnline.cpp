#include "CheckOnline.h"

#include <QtCore/QJsonDocument>

CheckOnline::CheckOnline(QObject *parent) : SendRequest(parent) {
    senderName = "CHECK_ONLINE";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDom_Node)), this, SLOT(setDataNote(QDom_Node)));
}

void CheckOnline::resendRequest() {
    emit emit_CheckOnlineResult("", "", "", QVariantList());
}

void CheckOnline::setDataNote(const QDom_Node &dom_Element) {
    getData = false;
    resultCode = "";
    items.clear();

    // Парсим данные
    parcerNote(dom_Element);

    if (resultCode != "") {
        // Тут отправляем сигнал с балансом
        emit emit_CheckOnlineResult(resultCode, status, message, items);
        return;
    }

    emit emit_CheckOnlineResult("", "", "", QVariantList());
    return;
}

void CheckOnline::parcerNote(const QDom_Node &dom_Element) {
    // Необходимо отпарсить документ
    QDom_Node dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {

            QDom_Element dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            if (strTag == "resultCode") {
                resultCode = dom_Element.text();
            }

            if (strTag == "status") {
                status = dom_Element.text();
            }

            if (strTag == "message") {
                message = dom_Element.text();
            }

            if (strTag == "item") {
                QVariantMap item;
                item["label"] = dom_Element.attribute("label");
                item["value"] = dom_Element.attribute("value");

                items.append(item);
            }
        }

        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void CheckOnline::sendCheckOnlineRequest(
    QString trn, QString prvId, QString account, double amount, QVariantMap param) {
    QString header_xml = getHeaderRequest(Request::Type::CheckOnline);

    QString footer_xml = getFooterRequest();

    QString json = QJsonDocument::from_Variant(param).toJson(QJsonDocument::Compact);
    QString param_Str = param.isEmpty() ? "" : QString("<param>%1</param>\n").arg(json);
    QString amountStr = amount > 0 ? QString("<amount>%1</amount>\n").arg(amount) : "";

    QString xml = QString(header_xml + "<trn>%1</trn>\n" + "<prv_id>%2</prv_id>\n" +
                          "<account>%3</account>\n" + amountStr + param_Str + footer_xml)
                      .arg(trn, prvId, account);

    sendRequest(xml, 30000);
}
