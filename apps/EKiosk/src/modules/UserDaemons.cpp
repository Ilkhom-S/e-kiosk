#include "UserDaemons.h"

UserDaemons::UserDaemons(QObject *parent) : SendRequest(parent) {
    senderName = "USER_DAEMONS";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void UserDaemons::resendRequest() {
    emit emit_UserData("---");
}

void UserDaemons::setDataNote(const QDomNode &dom_Element) {
    resultCode = false;
    getData = false;
    balance = "";

    // Парсим данные
    parcerNote(dom_Element);

    if (balance != "") {
        // Тут отправляем сигнал с балансом
        emit this->emit_UserData(balance);
        return;
    }

    emit emit_UserData("---");
}

void UserDaemons::parcerNote(const QDomNode &dom_Element) {
    QDomNode dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {
            QDomElement dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            // Данные о дилере
            if (strTag == "info") {
                if (dom_Element.attribute("result", "") == "0") {
                    balance = dom_Element.attribute("balance", "");
                }
            }
        }

        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void UserDaemons::sendUserDataRequest(QString account, QString prvId) {
    QString header_xml = getHeaderRequest(Request::Type::GetAbonentInfo);

    QString footer_xml = getFooterRequest();

    QString xml = QString(header_xml + "<info account=\"%1\" prv=\"%2\" />\n" + footer_xml)
                      .arg(account, prvId);

    sendRequest(xml, 20000);
}
