// Project
#include "StatusDaemons.h"

StatusDaemons::StatusDaemons(QObject *parent) : SendRequest(parent)
{
    senderName = "STATUS_DAEMONS";

    demonTimer = new QTimer(this);
    connect(demonTimer, SIGNAL(timeout()), this, SIGNAL(getRequestParam()));

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(resendRequest()));

    connect(this, SIGNAL(emit_DomElement(QDomNode)), this, SLOT(setDataNote(QDomNode)));

    firstSend = true;
}

void StatusDaemons::setDataNote(const QDomNode &domElement)
{
    gbl_overdraft = 999.999;
    gbl_balance = 999.999;
    gbl_active = "";
    cmdList.clear();

    emit emit_Loging(0, senderName, "Запрос успешно отправлен.");
    // Парсим данные
    parcerNote(domElement);

    // Делаем небольшую проверку
    if (gbl_balance != 999.999 && gbl_overdraft != 999.999)
    {
        // Отправляем баланс на иследование
        emit emit_responseBalance(gbl_balance, gbl_overdraft, 1.111);

        if (firstSend)
            firstSend = false;
    }

    if (gbl_active != "")
    {
        emit emit_responseIsActive(gbl_active.toInt() == 1);
    }

    if (cmdList.count() > 0)
    {
        emit emit_cmdToMain(cmdList);
    }
}

void StatusDaemons::parcerNote(const QDomNode &domElement)
{
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull())
    {
        if (domNode.isElement())
        {
            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();
            // проверям респонс
            if (strTag == "resultCode")
            {
                QString sts = domElement.text();

                if (sts == "150" || sts == "151" || sts == "245" || sts == "11" || sts == "12" || sts == "133")
                {
                    emit lockUnlockAvtorization(true, sts.toInt());
                    return;
                }

                if (sts == "0")
                {
                    emit lockUnlockAvtorization(false, 0);
                }
            }

            if (strTag == "balance")
            {
                gbl_balance = domElement.text().toDouble();
            }

            if (strTag == "overdraft")
            {
                gbl_overdraft = domElement.text().toDouble();
            }

            if (strTag == "active")
            {
                gbl_active = domElement.text();
            }

            if (strTag == "cmd")
            {
                QVariantMap cmd;
                cmd["trn"] = domElement.attribute("trn", "");
                cmd["account"] = domElement.attribute("account", "");
                cmd["comment"] = domElement.attribute("comment", "");
                cmd["cmd"] = domElement.text();

                cmdList.append(cmd);
            }

            if (strTag == "hash")
            {
                // Отправляем хеш на проверку
                emit emit_hashToCheck(domElement.text());
            }

            if (strTag == "hash_conf")
            {
                QString com = domElement.attribute("p", "");
                // Отправляем хеш update на проверку
                emit emit_hashUpdateToCheck(domElement.text(), com);
            }
        }

        parcerNote(domNode);
        domNode = domNode.nextSibling();
    }
}
void StatusDaemons::resendRequest()
{
    if (count_resend < 2)
    {
        // Количество повторений меньше 3
        count_resend++;
        QTimer::singleShot(30000, this, SLOT(r_RequestRepeet()));
    }
}

void StatusDaemons::r_RequestRepeet()
{
    emit emit_Loging(0, senderName, QString("Попытка N - %1 отправки статуса терминала на сервер.").arg(count_resend));

    sendRequest(requestString, 30000);
}

void StatusDaemons::startTimer(const int sec)
{
    if (!demonTimer->isActive())
    {
        // Запускаем таймер если остановлен
        demonTimer->start(sec * 1000);
    }
}

void StatusDaemons::sendStatusToServer(Sender::Data &a_Data)
{

    QString header_xml = getHeaderRequest(Request::Type::SendMonitor);

    QString footer_xml = getFooterRequest();

    QString action = "";

    if (a_Data.actionState)
    {
        action += "<actions>\n";
        for (int i = 0; i < a_Data.action.count(); i++)
            action += "<action>" + a_Data.action.at(i) + "</action>\n";
        action += "</actions>\n";
    }

    QString techParames = "";
    QString forLogData = "";

    if (a_Data.firstSend)
    {
        auto os = a_Data.systemInfo.value("os").toMap();
        auto cpu = a_Data.systemInfo.value("cpu").toMap();
        auto mboard = a_Data.systemInfo.value("mboard").toMap();
        auto ram = a_Data.systemInfo.value("ram").toMap();
        auto disk = a_Data.systemInfo.value("disk").toMap();
        auto system_disk = a_Data.systemInfo.value("system_disk").toMap();

        auto osInfo =
            QString("<os version=\"%1\" csname=\"%2\" csdversion=\"%3\" "
                    "osarchitecture=\"%4\">%5</os>\n")
                .arg(os.value("version").toString(), os.value("csname").toString(), os.value("csdversion").toString(),
                     os.value("osarchitecture").toString(), os.value("caption").toString());

        auto cpuInfo = QString("<cpu cores=\"%1\" logicalproc=\"%2\">%3</cpu>\n")
                           .arg(cpu.value("numberofcores").toString(),
                                cpu.value("numberoflogicalprocessors").toString(), cpu.value("name").toString());

        auto mboardInfo = QString("<mboard product=\"%1\" version=\"%2\" "
                                  "serialnumber=\"%3\">%4</mboard>\n")
                              .arg(mboard.value("product").toString(), mboard.value("version").toString(),
                                   mboard.value("serialnumber").toString(), mboard.value("manufacturer").toString());

        auto ramInfo = QString("<ram size=\"%1 MB\" speed=\"%2\" memorytype=\"%3\">%4</ram>\n")
                           .arg(ram.value("capacity").toString(), ram.value("speed").toString(),
                                ram.value("memorytype").toString(), ram.value("manufacturer").toString());

        auto diskInfo = QString("<disk serialnumber=\"%1\" size=\"%2 GB\" freespace=\"%3 GB\" "
                                "local_disks=\"%4\" model=\"%5\">\n"
                                "<system_disk caption=\"%6\" size=\"%7 GB\" freespace=\"%8 "
                                "GB\"></system_disk>\n"
                                "</disk>\n")
                            .arg(disk.value("serialnumber").toString(), disk.value("size").toString(),
                                 disk.value("freespace").toString(), disk.value("diskCaptions").toString(),
                                 disk.value("model").toString(), system_disk.value("caption").toString(),
                                 system_disk.value("size").toString(), system_disk.value("freespace").toString());

        techParames += QString(
            "<tech_parametrs>\n"
            "<validator port=\"" +
            a_Data.validator.port + "\" serial=\"" + a_Data.validator.serial + "\">" + a_Data.validator.name +
            "</validator>\n"
            "<coin_acceptor port=\"" +
            a_Data.coinAcceptor.port + "\" serial=\"" + a_Data.coinAcceptor.serial + "\">" + a_Data.coinAcceptor.name +
            "</coin_acceptor>\n"
            "<printer port=\"" +
            a_Data.printer.port + "\" >" + a_Data.printer.name +
            "</printer>\n"
            "<modem port=\"" +
            a_Data.modem.port + "\">" + a_Data.modem.name +
            "</modem>\n"
            "<sim signal=\"" +
            a_Data.modem.signal + "\" bsim=\"" + a_Data.modem.balance + "\" number=\"" + a_Data.modem.number + "\">" +
            a_Data.modem.provider +
            "</sim>\n"
            "<connection>" +
            a_Data.connection + "</connection>\n" + osInfo + cpuInfo + mboardInfo + ramInfo + diskInfo +
            "</tech_parametrs>\n");

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
                          .arg(a_Data.fullVersion, a_Data.validator.port, a_Data.validator.serial,
                               a_Data.validator.name, a_Data.printer.port, a_Data.printer.name, a_Data.modem.port,
                               a_Data.modem.name, a_Data.modem.signal, a_Data.modem.balance, a_Data.modem.number,
                               a_Data.modem.provider, a_Data.connection);
    }

    QString bill_str = QString("<bill>%1%2</bill>\n").arg(a_Data.validator.billInfo, a_Data.coinAcceptor.coinInfo);
    QString out_str =
        QString("<out>%1:%2</out>\n").arg(a_Data.validator.moneyOutCount).arg(a_Data.validator.moneyOutSum);

    QString status = QString("<status>\n"
                             "<printer>%1</printer>\n"
                             "<cashcode num=\"%2\" sum=\"%3\">%4</cashcode>\n"
                             "<coin_acceptor num=\"%5\" sum=\"%6\">%7</coin_acceptor>\n" +
                             bill_str + out_str + "<lock_status>" + QString("%1").arg(a_Data.lockStatus) +
                             "</lock_status>\n" + action + "</status>\n")
                         .arg(a_Data.printer.allState)
                         .arg(a_Data.validator.billCount)
                         .arg(a_Data.validator.billSum)
                         .arg(a_Data.validator.state)
                         .arg(a_Data.coinAcceptor.coinCount)
                         .arg(a_Data.coinAcceptor.coinSum)
                         .arg(a_Data.coinAcceptor.state);

    QString request = QString(header_xml + status + techParames + footer_xml);

    forLogData += QString("Статус принтера           - %1 \n"
                          "Статус валидатора         - %2 \n"
                          "Количество купюр          - %3 \n"
                          "Общая сумма               - %4 \n"
                          "Терминал заблокирован?    - %5 \n")
                      .arg(a_Data.printer.allState, a_Data.validator.state)
                      .arg(a_Data.validator.billCount)
                      .arg(a_Data.validator.billSum)
                      .arg(a_Data.lockStatus ? "1" : "0");

    emit emit_Loging(0, senderName, QString("Получены данные о состоянии терминала.\n\n%1\n").arg(forLogData));
    emit emit_Loging(0, senderName, "Начинаем отправлять статус терминала на сервер.");

    count_resend = 0;
    requestString = request;

    sendRequest(request, 30000);
}
