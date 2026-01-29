// Project
#include "UserDaemons.h"

UserDaemons::UserDaemons(QObject *parent) : SendRequest(parent)
{
    senderName = "USER_DAEMONS";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_DomElement(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void UserDaemons::resendRequest()
{
    emit emit_UserData("---");
}

void UserDaemons::setDataNote(const QDomNode &domElement)
{
    resultCode = false;
    getData = false;
    balance = "";

    // Парсим данные
    parcerNote(domElement);

    if (balance != "")
    {
        // Тут отправляем сигнал с балансом
        emit this->emit_UserData(balance);
        return;
    }

    emit emit_UserData("---");
}

void UserDaemons::parcerNote(const QDomNode &domElement)
{
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull())
    {
        if (domNode.isElement())
        {
            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            // Данные о дилере
            if (strTag == "info")
            {
                if (domElement.attribute("result", "") == "0")
                {
                    balance = domElement.attribute("balance", "");
                }
            }
        }

        parcerNote(domNode);
        domNode = domNode.nextSibling();
    }
}

void UserDaemons::sendUserDataRequest(QString account, QString prvId)
{
    QString header_xml = getHeaderRequest(Request::Type::GetAbonentInfo);

    QString footer_xml = getFooterRequest();

    QString xml = QString(header_xml + "<info account=\"%1\" prv=\"%2\" />\n" + footer_xml).arg(account, prvId);

    sendRequest(xml, 20000);
}
