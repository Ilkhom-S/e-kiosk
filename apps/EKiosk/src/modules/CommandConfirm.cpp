#include "CommandConfirm.h"

CommandConfirm::CommandConfirm(QObject *parent) : SendRequest(parent) {
    senderName = "COMMAND_CONFIRM";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void CommandConfirm::resendRequest() {
    if (countAllRep < 20) {
        // Повторная отправка
        QTimer::singleShot(20000 * countAllRep, this, SLOT(sendRequestRepeat()));
    }
}

void CommandConfirm::sendRequestRepeat() {
    countAllRep++;
    sendRequest(requestXml, 30000);
}

void CommandConfirm::setDataNote(const QDomNode &domElement) {
    resultCode = false;

    // Парсим данные
    parseNode(domElement);

    if (resultCode) {
        // Обнуляем счетчик
        countAllRep = 0;
        emit emit_cmdConfirmed(trnCmd);
    }
}

void CommandConfirm::parseNode(const QDomNode &domElement) {
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull()) {
        if (domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            if (strTag == "resultCode") {
                QString sts = domElement.text();

                if (sts == "0") {
                    resultCode = true;
                }
            }
        }

        parseNode(domNode);
        domNode = domNode.nextSibling();
    }
}

void CommandConfirm::sendCommandConfirm(QString trn, int cmd) {
    trnCmd = trn;

    QString centerXml = QString("<commands>\n"
                                 "<cmd trn=\"%1\" resultCode=\"%2\">%3</cmd>\n"
                                 "</commands>\n")
                             .arg(trn, "0", QString::number(cmd));

    QString headerXml = getHeaderRequest(Request::Type::SendCommandConfirm);

    QString footerXml = getFooterRequest();

    QString xml = QString(headerXml + centerXml + footerXml);

    requestXml = xml;

    sendRequest(xml, 40000);
}
