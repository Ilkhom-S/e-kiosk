#include "SendLogInfo.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

SendLogInfo::SendLogInfo(QObject *parent) : SendRequest(parent) {
    senderName = "COMMAND_CONFIRM";

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));

    timerPic = new QTimer(this);
    timerPic->setSingleShot(true);
    connect(timerPic, SIGNAL(timeout()), SLOT(resendRequest()));

    resultCode = false;
    system_Log = "";
}

void SendLogInfo::resendRequest() {
    if (countAllRep < 20) {
        // Повторная отправка
        QTimer::singleShot(20000 * this->countAllRep, this, SLOT(sendRequestRepeet()));
    } else {
        emit emit_Loging(2,
                         "SEND_LOG_INFO",
                         QString("Невозможно отправить лог с id - %1 на сервер, "
                                 "количество повторов превысело 20")
                             .arg(nowIdTrn));
    }
}

void SendLogInfo::sendRequestRepeet() {
    countAllRep++;

    emit emit_Loging(
        0,
        "SEND_LOG_INFO",
        QString("Попытка N-%1 отправки на сервер лога с id - %2").arg(countAllRep).arg(nowIdTrn));

    sendRequest(requestXml, 30000);
}

void SendLogInfo::setDataNote(const QDomNode &domElement) {
    resultCode = false;

    // Парсим данные
    parcerNote(domElement);

    if (resultCode) {
        // Обнуляем счетчик
        countAllRep = 0;

        timerPic->stop();

        emit emit_cmdResponseCode(0);

        emit this->emit_Loging(0,
                               "SEND_LOG_INFO",
                               QString("Лог с id - %1 на сервер успешно отправлен.").arg(nowIdTrn));
    }
}

void SendLogInfo::parcerNote(const QDomNode &domElement) {
    // Необходимо отпарсить документ
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull()) {
        if (domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            // if(Debugger) qDebug() << strTag + " " + dom_Element.text();

            if (strTag == "resultCode") {
                QString sts = domElement.text();

                if (sts == "0") {
                    resultCode = true;
                }
            }
        }

        parcerNote(domNode);
        domNode = domNode.nextSibling();
    }
}

void SendLogInfo::sendLogInfoToServer(QString trn, QString date) {

    QString headerXml = getHeaderRequest(Request::Type::SendLogInfo);

    // дата создания
    QString vrmDate = QDate::fromString(date, "yyyy-MM-dd").toString("dd.MM.yyyy");
    QString striper = "";
    bool result = false;

    getCompressLogData(vrmDate, result, striper);

    QString idTrn = QDateTime::currentDateTime().toString("sszzz");
    nowIdTrn = idTrn.toInt();
    QFileInfo vrmFileInf;
    vrmFileInf.setFile("log/" + vrmDate + ".txt");

    QString dateCreate = vrmFileInf.birthTime().toString("yyyy-MM-dd HH:mm:ss");

    QString centerXml =
        QString("<messages>\n"
                "<message id=\"%1\" cmdtrn=\"%6\" datecreate=\"%2\">\n"
                "<title>Log Message %3</title>\n"
                "<attachments>\n"
                "<attachment name=\"%3.txt\">%4</attachment>\n"
                "</attachments>\n"
                "</message>\n"
                "</messages>\n")
            .arg(idTrn, dateCreate == "" ? vrmDate : dateCreate, vrmDate, striper, trn);

    QString footerXml = getFooterRequest();

    QString xml = QString(headerXml + centerXml + footerXml);

    emit emit_Loging(0,
                     "SEND_LOG_INFO",
                     QString("Начинаем отправлять лог на сервер(id - %1, дата "
                             "создания - %2, наименование - %3)")
                         .arg(idTrn, dateCreate, date + ".txt"));

    requestXml = xml;
    timerPic->start(120000);

    sendRequest(xml, 120000);
}

void SendLogInfo::sendLogValidatorToServer(QString trn, QString date, QString account) {
    QString headerXml = getHeaderRequest(Request::Type::SendLogInfo);

    QString vrmDate = date;
    QString striper = "";
    bool result = false;
    getCompressValiatorLogData(vrmDate, account, result, striper);

    QString idTrn = QDateTime::currentDateTime().toString("sszzz");
    nowIdTrn = idTrn.toInt();
    QFileInfo vrmFileInf;
    vrmFileInf.setFile("logvalidator/" + date + "/" + account + ".txt");

    QString dateCreate = vrmFileInf.birthTime().toString("yyyy-MM-dd HH:mm:ss");

    QString centerXml =
        QString("<messages>\n"
                "<message id=\"%1\" cmdtrn=\"%6\" datecreate=\"%2\">\n"
                "<title>Log Validator Message %5</title>\n"
                "<attachments>\n"
                "<attachment name=\"%3.txt\">%4</attachment>\n"
                "</attachments>\n"
                "</message>\n"
                "</messages>\n")
            .arg(idTrn, dateCreate == "" ? vrmDate : dateCreate, account, striper, date, trn);

    QString footerXml = getFooterRequest();

    QString xml = QString(headerXml + centerXml + footerXml);

    emit emit_Loging(0,
                     "SEND_LOG_VALIDATOR_INFO",
                     QString("Начинаем отправлять лог валидатора на сервер(id - "
                             "%1, дата создания - %2, наименование - %3)")
                         .arg(idTrn, dateCreate, account + ".txt"));

    requestXml = xml;
    timerPic->start(120000);

    sendRequest(xml, 120000);
}

void SendLogInfo::getCompressLogData(QString date, bool &result, QString &strip) {
    QByteArray ba;
    QFile fileInfo(QString("log/%1.txt").arg(date));

    if (!fileInfo.open(QIODevice::ReadOnly)) {
        qDebug() << "Error opened file - " << date;
        ba = QString("Лог файл не найден").toUtf8();
        result = false;
        return;
    }

    ba = fileInfo.readAll();
    result = true;

    fileInfo.close();

    QString string = "";
    string.append(ba);

    strip = QString(qCompress(ba, 9).toBase64());
}

void SendLogInfo::getCompressValiatorLogData(QString date,
                                             QString account,
                                             bool &result,
                                             QString &strip) {
    QByteArray ba;
    QFile fileInfo(QString("logvalidator/%1/%2.txt").arg(date, account));

    QString string = "";

    // если меньше 9MB
    if (fileInfo.size() < 9000000) {

        if (!fileInfo.open(QIODevice::ReadOnly)) {
            qDebug() << "Error opened file - " << date;
            ba = QString("Лог файл не найден").toUtf8();
            result = false;
            return;
        }

        ba = fileInfo.readAll();
        result = true;

        fileInfo.close();

        string.append(ba);
    } else {
        ba = "File is very big";
    }

    strip = QString(qCompress(ba, 9).toBase64());
}
