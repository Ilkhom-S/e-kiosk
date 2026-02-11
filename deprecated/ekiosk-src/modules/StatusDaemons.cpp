#include "StatusDaemons.h"

StatusDaemons::StatusDaemons(QObject *parent)
    : SendRequest(parent), demonTimer(new QTimer(this)), firstSend(true) {
    senderName = "STATUS_DAEMONS";

    connect(demonTimer, SIGNAL(timeout()), this, SIGNAL(getRequestParam()));

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));

    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
}

void StatusDaemons::setDataNote(const QDomNode &domElement) {
    gbl_overdraft = 999.999;
    gbl_balance = 999.999;
    gbl_active = "";
    cmdList.clear();

    emit emit_Loging(0, senderName, "Запрос успешно отправлен.");
    // Парсим данные
    parseNode(domElement);

    // Делаем небольшую проверку
    if (gbl_balance != 999.999 && gbl_overdraft != 999.999) {
        // Отправляем баланс на исследование
        emit emit_responseBalance(gbl_balance, gbl_overdraft, 1.111);

        if (firstSend) {
            firstSend = false;
}
    }

    if (gbl_active != "") {
        emit emit_responseIsActive(gbl_active.toInt() == 1);
    }

    if (cmdList.count() > 0) {
        emit emit_cmdToMain(cmdList);
    }
}

void StatusDaemons::parseNode(const QDomNode &domElement) {
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull()) {
        if (domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();
            // проверем респонс
            if (strTag == "resultCode") {
                QString sts = domElement.text();

                if (sts == "150" || sts == "151" || sts == "245" || sts == "11" || sts == "12" ||
                    sts == "133") {
                    emit lockUnlockAuthorization(true, sts.toInt());
                    return;
                }

                if (sts == "0") {
                    emit lockUnlockAuthorization(false, 0);
                }
            }

            if (strTag == "balance") {
                gbl_balance = domElement.text().toDouble();
            }

            if (strTag == "overdraft") {
                gbl_overdraft = domElement.text().toDouble();
            }

            if (strTag == "active") {
                gbl_active = domElement.text();
            }

            if (strTag == "cmd") {
                QVariantMap cmd;
                cmd["trn"] = domElement.attribute("trn", "");
                cmd["account"] = domElement.attribute("account", "");
                cmd["comment"] = domElement.attribute("comment", "");
                cmd["cmd"] = domElement.text();

                cmdList.append(cmd);
            }

            if (strTag == "hash") {
                // Отправляем хеш на проверку
                emit emit_hashToCheck(domElement.text());
            }

            if (strTag == "hash_conf") {
                QString com = domElement.attribute("p", "");
                // Отправляем хеш update на проверку
                emit emit_hashUpdateToCheck(domElement.text(), com);
            }
        }

        parseNode(domNode);
        domNode = domNode.nextSibling();
    }
}
void StatusDaemons::resendRequest() {
    if (count_resend < 2) {
        // Количество повторений меньше 3
        count_resend++;
        QTimer::singleShot(30000, this, SLOT(r_RequestRepeat()));
    }
}

void StatusDaemons::r_RequestRepeat() {
    emit emit_Loging(
        0,
        senderName,
        QString("Попытка N - %1 отправки статуса терминала на сервер.").arg(count_resend));

    sendRequest(requestString, 30000);
}

void StatusDaemons::startTimer(const int sec) {
    if (!demonTimer->isActive()) {
        // Запускаем таймер если остановлен
        demonTimer->start(sec * 1000);
    }
}

void StatusDaemons::sendStatusToServer(Sender::Data &aData) {

    QString headerXml = getHeaderRequest(Request::Type::SendMonitor);

    QString footerXml = getFooterRequest();

    QString action = "";

    if (aData.actionState) {
        action += "<actions>\n";
        for (int i = 0; i < aData.action.count(); i++) {
            action += "<action>" + aData.action.at(i) + "</action>\n";
}
        action += "</actions>\n";
    }

    QString techParams = "";
    QString forLogData = "";

    if (aData.firstSend) {
        auto os = aData.system_Info.value("os").toMap();
        auto cpu = aData.system_Info.value("cpu").toMap();
        auto mboard = aData.system_Info.value("mboard").toMap();
        auto ram = aData.system_Info.value("ram").toMap();
        auto disk = aData.system_Info.value("disk").toMap();
        auto systemDisk = aData.system_Info.value("systemDisk").toMap();

        auto osInfo = QString("<os version=\"%1\" csname=\"%2\" csdversion=\"%3\" "
                              "osarchitecture=\"%4\">%5</os>\n")
                          .arg(os.value("version").toString(),
                               os.value("csname").toString(),
                               os.value("csdversion").toString(),
                               os.value("osarchitecture").toString(),
                               os.value("caption").toString());

        auto cpuInfo = QString("<cpu cores=\"%1\" logicalproc=\"%2\">%3</cpu>\n")
                           .arg(cpu.value("numberofcores").toString(),
                                cpu.value("numberoflogicalprocessors").toString(),
                                cpu.value("name").toString());

        auto mboardInfo = QString("<mboard product=\"%1\" version=\"%2\" "
                                  "serialnumber=\"%3\">%4</mboard>\n")
                              .arg(mboard.value("product").toString(),
                                   mboard.value("version").toString(),
                                   mboard.value("serialnumber").toString(),
                                   mboard.value("manufacturer").toString());

        auto ram_Info = QString("<ram size=\"%1 MB\" speed=\"%2\" memorytype=\"%3\">%4</ram>\n")
                            .arg(ram.value("capacity").toString(),
                                 ram.value("speed").toString(),
                                 ram.value("memorytype").toString(),
                                 ram.value("manufacturer").toString());

        auto diskInfo = QString("<disk serialnumber=\"%1\" size=\"%2 GB\" freespace=\"%3 GB\" "
                                "local_disks=\"%4\" model=\"%5\">\n"
                                "<systemDisk caption=\"%6\" size=\"%7 GB\" freespace=\"%8 "
                                "GB\"></systemDisk>\n"
                                "</disk>\n")
                            .arg(disk.value("serialnumber").toString(),
                                 disk.value("size").toString(),
                                 disk.value("freespace").toString(),
                                 disk.value("diskCaptions").toString(),
                                 disk.value("model").toString(),
                                 systemDisk.value("caption").toString(),
                                 systemDisk.value("size").toString(),
                                 systemDisk.value("freespace").toString());

        techParams += QString("<tech_parametrs>\n"
                              "<validator port=\"" +
                              aData.validator.port + "\" serial=\"" + aData.validator.serial +
                              "\">" + aData.validator.name +
                              "</validator>\n"
                              "<coin_acceptor port=\"" +
                              aData.coinAcceptor.port + "\" serial=\"" + aData.coinAcceptor.serial +
                              "\">" + aData.coinAcceptor.name +
                              "</coin_acceptor>\n"
                              "<printer port=\"" +
                              aData.printer.port + "\" >" + aData.printer.name +
                              "</printer>\n"
                              "<modem port=\"" +
                              aData.modem.port + "\">" + aData.modem.name +
                              "</modem>\n"
                              "<sim signal=\"" +
                              aData.modem.signal + "\" bsim=\"" + aData.modem.balance +
                              "\" number=\"" + aData.modem.number + "\">" + aData.modem.provider +
                              "</sim>\n"
                              "<connection>" +
                              aData.connection + "</connection>\n" + osInfo + cpuInfo + mboardInfo +
                              ram_Info + diskInfo + "</tech_parametrs>\n");

        forLogData += QString("Версия ПО                 - %1 \n"
                              "Порт валидатора           - %2 \n"
                              "Серийный номер валидатора - %3 \n"
                              "Наименование валидатора   - %4 \n"
                              "Порт принтера             - %5 \n"
                              "Наименование принтера     - %6 \n"
                              "Порт модема               - %7 \n"
                              "Наименование модема       - %8 \n"
                              "Сигнал модема             - %9 \n"
                              "Баланс сим-карты          - %10 \n"
                              "Номер сим-карты           - %11 \n"
                              "Провайдер сим-карты       - %12 \n"
                              "Тип соединения            - %13 \n")
                          .arg(aData.fullVersion,
                               aData.validator.port,
                               aData.validator.serial,
                               aData.validator.name,
                               aData.printer.port,
                               aData.printer.name,
                               aData.modem.port,
                               aData.modem.name,
                               aData.modem.signal,
                               aData.modem.balance,
                               aData.modem.number,
                               aData.modem.provider,
                               aData.connection);
    }

    QString billStr =
        QString("<bill>%1%2</bill>\n").arg(aData.validator.billInfo, aData.coinAcceptor.coinInfo);
    QString outStr = QString("<out>%1:%2</out>\n")
                         .arg(aData.validator.moneyOutCount)
                         .arg(aData.validator.moneyOutSum);

    QString status =
        QString("<status>\n"
                "<printer>%1</printer>\n"
                "<cashcode num=\"%2\" sum=\"%3\">%4</cashcode>\n"
                "<coin_acceptor num=\"%5\" sum=\"%6\">%7</coin_acceptor>\n" +
                billStr + outStr + "<lock_status>" + QString("%1").arg(aData.lockStatus) +
                "</lock_status>\n" + action + "</status>\n")
            .arg(aData.printer.allState)
            .arg(aData.validator.billCount)
            .arg(aData.validator.billSum)
            .arg(aData.validator.state)
            .arg(aData.coinAcceptor.coinCount)
            .arg(aData.coinAcceptor.coinSum)
            .arg(aData.coinAcceptor.state);

    QString request = QString(headerXml + status + techParams + footerXml);

    forLogData += QString("Статус принтера           - %1 \n"
                          "Статус валидатора         - %2 \n"
                          "Количество купюр          - %3 \n"
                          "Общая сумма               - %4 \n"
                          "Терминал заблокирован?    - %5 \n")
                      .arg(aData.printer.allState, aData.validator.state)
                      .arg(aData.validator.billCount)
                      .arg(aData.validator.billSum)
                      .arg(aData.lockStatus ? "1" : "0");

    emit emit_Loging(
        0, senderName, QString("Получены данные о состоянии терминала.\n\n%1\n").arg(forLogData));
    emit emit_Loging(0, senderName, "Начинаем отправлять статус терминала на сервер.");

    count_resend = 0;
    requestString = request;

    sendRequest(request, 30000);
}
