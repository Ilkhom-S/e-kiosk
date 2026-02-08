#include "CommandConfirm.h"

CommandConfirm::CommandConfirm(QObject *parent) : SendRequest(parent) {
    senderName = "COMMAND_CONFIRM";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDom_Node)), this, SLOT(setDataNote(QDom_Node)));
}

void CommandConfirm::resendRequest() {
    if (countAllRep < 20) {
        // Повторная отправка
        QTimer::singleShot(20000 * countAllRep, this, SLOT(sendRequestRepeet()));
    }
}

void CommandConfirm::sendRequestRepeet() {
    countAllRep++;
    sendRequest(requestXml, 30000);
}

void CommandConfirm::setDataNote(const QDom_Node &dom_Element) {
    resultCode = false;

    // Парсим данные
    parcerNote(dom_Element);

    if (resultCode) {
        // Обнуляем счетчик
        countAllRep = 0;
        emit emit_cmdConfirmed(trnCmd);
    }
}

void CommandConfirm::parcerNote(const QDom_Node &dom_Element) {
    QDom_Node dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {
            QDom_Element dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            if (strTag == "resultCode") {
                QString sts = dom_Element.text();

                if (sts == "0") {
                    resultCode = true;
                }
            }
        }

        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void CommandConfirm::sendCommandConfirm(QString trn, int cmd) {
    trnCmd = trn;

    QString center_xml = QString("<commands>\n"
                                 "<cmd trn=\"%1\" resultCode=\"%2\">%3</cmd>\n"
                                 "</commands>\n")
                             .arg(trn, "0", QString::number(cmd));

    QString header_xml = getHeaderRequest(Request::Type::SendCommandConfirm);

    QString footer_xml = getFooterRequest();

    QString xml = QString(header_xml + center_xml + footer_xml);

    requestXml = xml;

    sendRequest(xml, 40000);
}
