#include "CollectDaemons.h"

#include <QtCore/QDebug>
#include <QtCore/QUuid>

CollectDaemons::CollectDaemons(QObject *parent) : SendRequest(parent) {

    senderName = "COLLECT_DAEMONS";

    RealRepeet = 10;
    countAllRep = 0;

    demonTimer = new QTimer(this);
    connect(demonTimer, SIGNAL(timeout()), this, SLOT(getRequestParam()));

    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(errResponse()));

    connect(this, SIGNAL(emit_Dom_Element(QDom_Node)), this, SLOT(setDataNote(QDom_Node)));

    debugger = false;
}

void CollectDaemons::setDataNote(const QDom_Node &dom_Element) {
    emit emit_Loging(0, senderName, QString("Пришел ответ инкассации от сервера. Парсим данные"));

    cIdUpdate = "";

    // Парсим данные
    parcerNote(dom_Element);

    // Делаем небольшую проверку
    if (cIdUpdate != "") {
        // Обновляем статус инкассации на sent
        if (confirm_Collection(cIdUpdate)) {
            // Обновляем статус купюр мимо с id_collect
            emit emit_Loging(
                0,
                senderName,
                QString("Инкассация с ID - %1 на сервере подтвердилась.").arg(cIdUpdate));
            updateMoneyOut(cIdUpdate, "confirmed");
        }

        countAllRep = 0;

        if (repeatCount) {
            demonTimer->start(55000);
        }
    }
}

bool CollectDaemons::getDataCollectList(QStringList &lst, int &count_i) {
    QSqlQuery selectCollect(db);

    QString strQuery;

    strQuery = QString("SELECT * FROM terminal_collect WHERE status != 'new' "
                       "ORDER BY date_create DESC;");

    if (!selectCollect.exec(strQuery)) {
        // if(Debugger) qDebug() << "Error Select moneyout;";
        // if(Debugger) qDebug() << selectCollect.lastError();
        return false;
    }
    // if(Debugger) qDebug() << strQuery;

    QSqlRecord recordCollect = selectCollect.record();
    count_i = 0;

    while (selectCollect.next()) {
        count_i++;
        lst << selectCollect.value(recordCollect.indexOf("date_create")).toString();
    }

    return true;
}

void CollectDaemons::parcerNote(const QDom_Node &dom_Element) {
    QDom_Node dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {
            QDom_Element dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            // проверям респонс
            if (strTag == "resultCode") {
                QString sts = dom_Element.text();
                if (sts == "150" || sts == "245" || sts == "11" || sts == "12" || sts == "133") {
                    emit lockUnlockAvtorization(true, sts.toInt());
                    return;
                }
            }

            if (strTag == "stacker") {
                cIdUpdate = dom_Element.attribute("cid", "");
            }
        }

        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void CollectDaemons::errResponse() {
    // Число попыток на отправку
    if (countAllRep >= RealRepeet) {
        // Тут надо остановить демон проверки соединения
        emit emit_Loging(0,
                         senderName,
                         QString("Отключаем демон( превышено число отправок инкассации - %1 )")
                             .arg(countAllRep));
        demonTimer->stop();
    } else {

        if (countAllRep <= 0) {
            countAllRep = 1;
        }

        if (!demonTimer->isActive()) {
            demonTimer->start(countAllRep * 55000);
        }
    }
}

void CollectDaemons::setNominalData(QVariantMap data) {
    nominalData = NominalData::from_Variant(data);
}

QString CollectDaemons::getHtmlInfoBox(QString &nonCollectPayment,
                                       int &moneyOutNum,
                                       double &moneyOutSum,
                                       QString data,
                                       QString &collectionId,
                                       QString &stackId,
                                       QString &trnFrom,
                                       QString &trnTo) {
    moneyOutNum = 0;
    moneyOutSum = 0;

    QString dateCreate = "";
    QString xmlDenom = "";
    QString collectionIdNext = "";

    // Парсим не инкасированные платежи
    if (data == "") {
        QString cid = getCollectionId();
        int countNonCollectPayment = getNonCollectOperationCount(cid);
        nonCollectPayment = QString::number(countNonCollectPayment);

        // Транзакция с, по,
        getTrnOperation(cid, "MIN", trnFrom);
        getTrnOperation(cid, "MAX", trnTo);

        // Вытаскиваем номиналы купюр
        parserCollectIntoOperation(cid);

        // Вытаскиваем купюры мимо
        getMoneyOut(cid, moneyOutNum, moneyOutSum);
    } else {

        nonCollectPayment = "0";

        auto collId = getCollectIdByDate(data);

        if (collId != "") {
            // Если есть какая нибудь информация
            if (getCollectionInfo(collId, collectionIdNext, stackId, dateCreate, xmlDenom)) {

                // Теперь надо отпарсить xml
                if (parsDenomilSlot(xmlDenom)) {
                    collectionId = collId;

                    // Транзакция с, по,
                    getTrnOperation(collectionId, "MIN", trnFrom);
                    getTrnOperation(collectionId, "MAX", trnTo);

                    // Купюр мимо
                    getMoneyOut(collectionId, moneyOutNum, moneyOutSum);
                } else {
                    emit emit_Loging(1, senderName, QString("Ошибка при парсинге xml collect."));
                    return "";
                }
            } else {
                emit emit_Loging(
                    1, senderName, QString("В системе отсутствует запрашиваемая инкассация."));
                return "";
            }
        } else {
            emit emit_Loging(
                1, senderName, QString("В системе отсутствует запрашиваемая инкассация."));
            return "";
        }
    }

    QString strBills = "<table width = \"100%\" style=\"font-size: 12px; border-style:solid; "
                       "border-width:1px; "
                       "border-color:#ffffff;\">"
                       "<tr bgcolor=\"#aaaaaa\" style=\"font-size: 14px; color: #ffffff; \">"
                       "<td align=\"center\" height=\"40px\">Номинал</td><td "
                       "align=\"center\">Количество</td><td "
                       "align=\"center\">Сумма</td>"
                       "</tr>";

    auto i = 0;
    for (auto &bill : nominalData.bills) {
        strBills += QString("<tr bgcolor=\"#%5\">"
                            "<td align=\"center\">%1 %2</td><td "
                            "align=\"center\">%3</td><td align=\"center\">%4</td>"
                            "</tr>")
                        .arg(bill.face)
                        .arg(nominalData.billCurrency)
                        .arg(bill.value)
                        .arg(bill.face * bill.value)
                        .arg(i % 2 == 0 ? "fff" : "eee");

        i++;
    }

    strBills += QString("<tr>"
                        "<td align=\"center\" width=\"50%\"><b>Купюры - "
                        "%1</b></td> <td colspan=\"2\" align=\"center\" "
                        "width=\"50%\"><b> Сумма - %2</b></td>"
                        "</tr>"
                        "</table>")
                    .arg(nominalData.billCount)
                    .arg(nominalData.billSum);

    QString strCoins = "";

    if (nominalData.coins.length() > 0) {
        strCoins += "<table width = \"100%\" style=\"font-size: 12px; border-style:solid; "
                    "border-width:1px; "
                    "border-color:#ffffff;\">"
                    "<tr bgcolor=\"#aaaaaa\" style=\"font-size: 14px; color: #ffffff; \">"
                    "<td align=\"center\" height=\"40px\">Монета</td><td "
                    "align=\"center\">Количество</td><td "
                    "align=\"center\">Сумма</td>"
                    "</tr>";

        i = 0;
        for (auto &coin : nominalData.coins) {
            strCoins += QString("<tr bgcolor=\"#%5\">"
                                "<td align=\"center\">%1 %2</td><td "
                                "align=\"center\">%3</td><td align=\"center\">%4</td>"
                                "</tr>")
                            .arg(coin.face)
                            .arg(nominalData.coinCurrency)
                            .arg(coin.value)
                            .arg(getCoinTxt(coin.face * coin.value), i % 2 == 0 ? "fff" : "eee");

            i++;
        }

        strCoins += QString("<tr>"
                            "<td align=\"center\" width=\"50%\"><b>Монеты - "
                            "%1</b></td> <td colspan=\"2\" "
                            "align=\"center\" width=\"50%\"><b> Сумма - %2</b></td>"
                            "</tr>"
                            "</table>")
                        .arg(nominalData.coinCount)
                        .arg(getCoinTxt(nominalData.coinSum));
    }

    QString strTotal = "";

    if (nominalData.coins.length() > 0) {
        auto totalCount = nominalData.billCount + nominalData.coinCount;
        auto totalSum = nominalData.billSum + nominalData.coinSum / nominalData.coinDivider;

        strTotal = QString("<br><td style='font-size: 14px;' colspan = \"3\" "
                           "align=\"center\"><b><font "
                           "color=\"blue\">Общее количество</font> - %1 "
                           "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <font "
                           "color=\"blue\">Общая сумма</font> - %2</b></td>")
                       .arg(totalCount)
                       .arg(totalSum);
    }

    QString html = "<table width = \"100%\" >"
                   "<tr>"
                   "<td>" +
                   strBills +
                   "</td>"
                   "<td width=\"5%\"></td>"
                   "<td>" +
                   strCoins +
                   "</td>"
                   "</tr>"
                   "<tr>" +
                   strTotal +
                   "</tr>"
                   "</table>";

    return html;
}

QString CollectDaemons::getCoinTxt(int coin) {
    QString coin_txt = QString("%1 %2").arg(coin).arg(nominalData.coinCurrency);
    if (coin >= nominalData.coinDivider)
        coin_txt =
            QString("%1 %2").arg(coin / nominalData.coinDivider).arg(nominalData.billCurrency);

    return coin_txt;
}

void CollectDaemons::getRequestParam() {
    // Количество инкассаций которые надо отправить
    repeatCount = 0;

    int collectionToSend = getCollectionCount("send");

    if (collectionToSend == 0) {
        // Останавливаем демон
        demonTimer->stop();

        emit emit_Loging(0, senderName, QString("Отключаем демон( нет новых инкассаций )"));
        return;
    }

    emit emit_Loging(0,
                     senderName,
                     QString("Количество новых инкассаций в системе - %1.").arg(collectionToSend));

    QString trnId = "";
    QString dateCreate = "";
    QString xmlDenom = "";
    QString trnFrom = "";
    QString trnTo = "";
    QString collectionIdNext = "";
    int moneyOutNum = 0;
    double moneyOutSum = 0;

    repeatCount = collectionToSend;

    auto collectionId = getCollectionId("send");

    if (collectionId == "") {
        emit emit_Loging(0, senderName, QString("В системе нет новых инкассаций."));
        demonTimer->stop();
        return;
    } else {
        emit emit_Loging(
            0,
            senderName,
            QString("В системе имеется новая инкассация под номером - %1.").arg(collectionId));
    }

    if (getCollectionInfo(collectionId, collectionIdNext, trnId, dateCreate, xmlDenom)) {
        // Теперь надо отпарсить xml
        if (parsDenomilSlot(xmlDenom)) {
            // Транзакция с, по,
            getTrnOperation(collectionId, "MIN", trnFrom);
            getTrnOperation(collectionId, "MAX", trnTo);

            // Купюр мимо
            getMoneyOut(collectionId, moneyOutNum, moneyOutSum);

            if (trnId != "") {
                // Тут надо создать XML с инкассацией
                requestXml = "";
                getXmlForSend(requestXml,
                              collectionId,
                              collectionIdNext,
                              trnId,
                              dateCreate,
                              xmlDenom,
                              moneyOutNum,
                              moneyOutSum,
                              trnFrom,
                              trnTo);

                // Тут надо отправить XML на сервер
                QTimer::singleShot(3000, this, SLOT(sendCollectRequest()));
            }
        } else {
            emit emit_Loging(1, senderName, QString("Ошибка при парсинге xml collect."));
            demonTimer->stop();
            return;
        }
    } else {
        emit emit_Loging(0, senderName, QString("В системе отсутствует запрашиваемая инкассация."));
        demonTimer->stop();
        return;
    }
}

QString CollectDaemons::getCollectIdByDate(QString date) {
    QSqlQuery selectCollect(db);

    QString strCollect = QString("SELECT collect_id FROM terminal_collect WHERE date_create = "
                                 "\"%1\" and status != 'new' LIMIT 1;")
                             .arg(date);

    if (!selectCollect.exec(strCollect)) {
        if (debugger)
            qDebug() << selectCollect.lastError().text();
        return "";
    }

    QSqlRecord recordCollect = selectCollect.record();

    QString collectionId = "";

    if (selectCollect.next()) {
        collectionId = selectCollect.value(recordCollect.indexOf("collect_id")).toString();
    }

    return collectionId;
}

QVariantMap CollectDaemons::getNominalInfo() {
    QVariantMap data;

    int moneyOutCount = 0;
    double moneyOutSum = 0;

    auto cid = getCollectionId();

    parserCollectIntoOperation(cid);
    getMoneyOut(cid, moneyOutCount, moneyOutSum);

    data["money_out_count"] = moneyOutCount;
    data["money_out_sum"] = moneyOutSum;

    data["bill_count"] = nominalData.billCount;
    data["bill_sum"] = nominalData.billSum;

    data["coin_count"] = nominalData.coinCount;
    data["coin_sum"] = nominalData.coinSum / nominalData.coinDivider;

    QString billInfo = "";
    for (auto &bill : nominalData.bills) {
        billInfo += QString("%1:%2;").arg(bill.face).arg(bill.value);
    }
    data["bills"] = billInfo;

    QString coinInfo = "";
    for (auto &coin : nominalData.coins) {
        coinInfo += QString("%1M:%2;").arg(coin.face).arg(coin.value);
    }
    data["coins"] = coinInfo;

    return data;
}

bool CollectDaemons::parserCollectIntoOperation(const QString cid) {
    for (auto &b : nominalData.bills) {
        b.value = 0;
    }

    for (auto &c : nominalData.coins) {
        c.value = 0;
    }

    QSqlQuery selectCollect(db);
    QString strCollect = QString("SELECT operation_money_denom FROM terminal_operation WHERE "
                                 "operation_collect_id = '%1';")
                             .arg(cid);

    if (!selectCollect.exec(strCollect)) {
        // if(Debugger) qDebug() << "Error getSum_Count";
        // if(Debugger) qDebug() << selectCollect.lastError();
        return false;
    }
    QSqlRecord recordCollect = selectCollect.record();

    while (selectCollect.next()) {
        QString operation_money_denom =
            selectCollect.value(recordCollect.indexOf("operation_money_denom")).toString();
        QStringList list = operation_money_denom.split(",", Qt::SkipEmptyParts);

        for (auto &val : list) {
            if (val.contains("M")) {
                int face = val.replace("M", "").toInt();

                for (auto &c : nominalData.coins) {
                    if (c.face == face) {
                        c.value++;
                        break;
                    }
                }
            } else {
                int face = val.toInt();
                for (auto &b : nominalData.bills) {
                    if (b.face == face) {
                        b.value++;
                        break;
                    }
                }
            }
        }
    }

    nominalData.calculateTotal();

    return true;
}

bool CollectDaemons::getCheckText(QString &text, bool preCheck, QString dateP, QString cid) {
    QString trnId = "";
    QString collectId = "";
    QString collectIdNext = "";
    QString dateCreate = "";
    QString xmlDenom = "";
    QString datePre = "";
    QString trnFrom = "";
    QString trnTo = "";
    int moneyOutNum = 0;
    double moneyOutSum = 0;

    //    bool remoteCollection;

    if (cid.isEmpty()) {
        //        remoteCollection = false;
        cid = getCollectionId();
    } else {
        //        remoteCollection = true;
    }

    if (!preCheck) {
        // Надо сделать инкассацию
        // Смотрим количество не инкасированных платежей
        int countNonCollectPayment = getNonCollectOperationCount(cid);

        emit emit_Loging(0, senderName, QString("Пришла команда на инкасирование платежей."));

        if (countNonCollectPayment > 0) {
            emit emit_Loging(
                0,
                senderName,
                QString("Количество не инкасированных платежей %1.").arg(countNonCollectPayment));

            // Вытаскиваем номиналы купюр
            if (parserCollectIntoOperation(cid)) {
                // Сама xml-ка с купюрами
                xmlDenom = getDenominalXml();

                // Подтверждаем инкассацию в базе
                if (sendCollection(trnId, cid, collectIdNext, dateCreate, xmlDenom)) {

                    // Надо проапдейтить купюры мимо на collect id
                    updateMoneyOut(cid, "send");

                    // Дата предыдущей инкассации
                    getDatePrevCollection(datePre, cid);

                    // Транзакция с, по,
                    getTrnOperation(cid, "MIN", trnFrom);
                    getTrnOperation(cid, "MAX", trnTo);

                    emit emit_Loging(
                        0,
                        senderName,
                        QString("Транзакции не инкассированных платежей с - %1 по - %2")
                            .arg(trnFrom, trnTo));

                    // Купюр мимо
                    getMoneyOut(cid, moneyOutNum, moneyOutSum);
                } else {
                    emit emit_Loging(2, senderName, QString("Ошибка при создании инкассации."));
                    return false;
                }
            } else {
                emit emit_Loging(2, senderName, QString("Не возможно вытащить xml из платежей."));
                return false;
            }
        } else {
            //            if (remoteCollection) {
            //                emit emit_Loging(0, senderName, QString("В системе
            //                отсутствует инкассация с cid: %1").arg(cid));

            //                QSqlQuery selectCollect(db);

            //                auto dateCreate =
            //                QDateTime::currentDateTime().toString("yyyy-MM-dd
            //                HH:mm:ss"); auto trnId      = "1" +
            //                QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz").right(15);
            //                auto cidNext    = getCollectionId();

            //                QString xmlDenom = "<collection>";

            //                for (auto &bill: nominalData.bills) {
            //                     xmlDenom += QString("<bill
            //                     face=\"%1\">%2</bill>").arg(bill.face).arg(0);
            //                }

            //                for (auto &coin: nominalData.coins) {
            //                     xmlDenom += QString("<bill
            //                     face=\"%1M\">%2</bill>").arg(coin.face).arg(0);
            //                }

            //                xmlDenom += "</collection>";

            //                QString strQuery = QString("INSERT INTO
            //                terminal_collect(collect_id, stack_id, date_create,
            //                denom, status, collect_id_next)"
            //                                           " VALUES('%1', %2 , \"%3\",
            //                                           '%4', 'send',
            //                                           '%5');").arg(cid, trnId,
            //                                           dateCreate, xmlDenom,
            //                                           cidNext);

            //                if (!selectCollect.exec(strQuery)) {
            //                    return false;
            //                }

            //            } else {
            emit emit_Loging(0, senderName, QString("В системе нет неинкассированных платежей."));
            //            }
            return false;
        }
    } else {

        if (dateP == "") {
            collectId = cid;
        } else {
            collectId = getCollectIdByDate(dateP);
        }

        if (collectId != "") {
            // Если есть какая нибудь информация

            if (getCollectionInfo(collectId, collectIdNext, trnId, dateCreate, xmlDenom)) {
                // Теперь надо отпарсить xml
                if (parsDenomilSlot(xmlDenom)) {

                    // Дата предыдущей инкассации
                    getDatePrevCollection(datePre, collectId);

                    // Транзакция с, по,
                    getTrnOperation(collectId, "MIN", trnFrom);
                    getTrnOperation(collectId, "MAX", trnTo);

                    // Купюр мимо
                    getMoneyOut(collectId, moneyOutNum, moneyOutSum);
                } else {
                    emit emit_Loging(1, senderName, QString("Ошибка при парсинге xml collect."));
                    return false;
                }
            } else {
                emit emit_Loging(
                    0, senderName, QString("В системе отсутствует запрашиваемая инкассация."));
                return false;
            }
        } else {
            emit emit_Loging(
                0, senderName, QString("В системе отсутствует запрашиваемая инкассация."));
            return false;
        }
    }

    QString bills = "[p]Статистика по купюрам:\n";
    for (auto &bill : nominalData.bills) {
        bills +=
            QString("[p]%1 %2: %3\n").arg(bill.face).arg(nominalData.billCurrency).arg(bill.value);
    }

    bills += QString("[p]Купюр: %1\n").arg(nominalData.billCount);
    bills += QString("[p][b]Сумма:[/b] %1\n").arg(nominalData.billSum);

    QString coins = "";
    if (nominalData.coins.length() > 0) {
        coins = "[p]------------------------------\n"
                "[p]Статистика по монетам:\n";

        for (auto &coin : nominalData.coins) {
            coins += QString("[p]%1 %2: %3\n")
                         .arg(coin.face)
                         .arg(nominalData.coinCurrency)
                         .arg(coin.value);
        }

        coins += QString("[p]Монеты: %1\n").arg(nominalData.coinCount);
        coins += QString("[p][b]Сумма:[/b] %1\n").arg(getCoinTxt(nominalData.coinSum));
    }

    QString total = "";
    if (nominalData.coins.length() > 0) {
        auto totalCount = nominalData.billCount + nominalData.coinCount;
        auto totalSum = nominalData.billSum + nominalData.coinSum / nominalData.coinDivider;

        total = QString("[p]Общее количество: %1 Общая сумма: %2\n").arg(totalCount).arg(totalSum);
    }

    text = QString("[p]Инкассация %1\n"
                   "[p]==============================\n"
                   "[p]Номер терминала: %2\n"
                   "[p]ID Инкассации: %3\n"
                   "[p]Дата: %4\n"
                   "[p]Дата пред: %5\n"
                   "[p]Транз.: с  %6\n"
                   "[p]        по %7\n"
                   "[p]==============================\n"
                   "%8"
                   "%9"
                   "[p]==============================\n"
                   "%10"
                   "[p]Купюр мимо: %11 на сумму: %12\n")
               .arg(collectId.left(8),
                    num_Trm,
                    trnId,
                    dateCreate,
                    datePre,
                    trnFrom,
                    trnTo,
                    bills,
                    coins,
                    total)
               .arg(moneyOutNum)
               .arg(moneyOutSum);

    if (!preCheck && trnId != "") {
        // Тут надо создать XML с инкассацией
        requestXml = "";
        getXmlForSend(requestXml,
                      collectId,
                      collectIdNext,
                      trnId,
                      dateCreate,
                      xmlDenom,
                      moneyOutNum,
                      moneyOutSum,
                      trnFrom,
                      trnTo);

        // Тут надо отправить XML на сервер
        QTimer::singleShot(5000, this, SLOT(sendCollectRequest()));
    }

    return trnId != "";
}

void CollectDaemons::sendCollectRequest() {
    emit emit_Loging(0, senderName, QString("Начинаем отправлять данные инкассации на сервер."));

    // Количество повторов для отправки инкассации
    if (countAllRep < RealRepeet) {

        // Увеличиваем количество повторов
        countAllRep++;

        if (!sendRequest(requestXml, 30000)) {
            // Тут надо отправить XML на сервер
            emit emit_Loging(0, senderName, QString("Тут надо отправить XML на сервер."));

            demonTimer->start(50000);
        } else {
            //            emit emit_Loging(0, senderName, QString("Это для проверки
            //            отправилось или нет"));

            //            emit emit_Loging(0, senderName, QString("countAllRep = %1
            //            RealRepeet = %2").arg(countAllRep).arg(RealRepeet));

            if (countAllRep <= 0)
                countAllRep = 1;

            // Это для проверки отправилось или нет
            if (!demonTimer->isActive()) {
                emit emit_Loging(
                    0, senderName, QString("Активируем таймер через %1 сек").arg(countAllRep * 55));

                demonTimer->start(countAllRep * 55000);
            }
        }
    } else {
        // Тут надо остановить демон проверки соединения
        emit emit_Loging(0,
                         senderName,
                         QString("Отключаем демон( превышено число отправок инкассации - %1 )")
                             .arg(countAllRep));
        demonTimer->stop();
    }
}

void CollectDaemons::getXmlForSend(QString &xml,
                                   QString cid,
                                   QString cidNext,
                                   QString sid,
                                   QString dateCreate,
                                   QString collectDenom,
                                   int moneyOutCount,
                                   double moneyOutSum,
                                   QString trnFrom,
                                   QString trnTo) {

    QString header_xml = getHeaderRequest(Request::Type::SendEncashment);

    QString footer_xml = getFooterRequest();

    QString center_xml = QString("<stacker cid=\"%1\" cid_next=\"%2\" sid=\"%3\" action=\"new\">\n"
                                 "<date>%4</date>\n"
                                 "%5\n"
                                 "<trn from=\"%6\" to=\"%7\"/>\n"
                                 "<moneyout count=\"%8\" sum=\"%9\"/>\n"
                                 "</stacker>\n")
                             .arg(cid, cidNext, sid, dateCreate, collectDenom, trnFrom, trnTo)
                             .arg(moneyOutCount)
                             .arg(moneyOutSum);

    xml = QString(header_xml + center_xml + footer_xml);

    QString bills = "Статистика по купюрам:\n";
    for (auto &bill : nominalData.bills) {
        bills += QString("%1 %2. количество - %3,  сумма - %4 \n")
                     .arg(bill.face)
                     .arg(nominalData.billCurrency)
                     .arg(bill.value)
                     .arg(bill.face * bill.value);
    }

    bills +=
        QString("Купюры: %1      Сумма: %2\n").arg(nominalData.billCount).arg(nominalData.billSum);

    QString coins = "";
    if (nominalData.coins.length() > 0) {
        coins = "------------------------------\n"
                "Статистика по монетам:\n";

        for (auto &coin : nominalData.coins) {
            coins += QString("%1 %2. количество - %3,   сумма - %4 \n")
                         .arg(coin.face)
                         .arg(nominalData.coinCurrency)
                         .arg(coin.value)
                         .arg(coin.face * coin.value);
        }

        coins += QString("Монеты: %1      Сумма: %2\n")
                     .arg(nominalData.coinCount)
                     .arg(getCoinTxt(nominalData.coinSum));
    }

    QString total = "";

    if (nominalData.coins.length() > 0) {
        auto totalCount = nominalData.billCount + nominalData.coinCount;
        auto totalSum = nominalData.billSum + nominalData.coinSum / nominalData.coinDivider;

        total = QString("------------------------------\n"
                        "Общее количество: %1 Общая сумма: %2\n")
                    .arg(totalCount)
                    .arg(totalSum);
    }

    QString strDenom_Log = QString("===================================================\n"
                                  "%1"
                                  "%2"
                                  "%3"
                                  "===================================================\n")
                              .arg(bills, coins, total);

    emit emit_Loging(0,
                     senderName,
                     QString("Формирование инкассации CID-%2, SID-%3, "
                             "Pay_From-%4, Pay_To-%5...\n\n %1 \n")
                         .arg(strDenom_Log, cid, sid, trnFrom, trnTo));
}

bool CollectDaemons::updateMoneyOut(QString collectionId, QString collectionStatus) {
    QSqlQuery updateMoneOut(db);

    QString strCollect = QString("UPDATE terminal_moneyout SET"
                                 " moneyout_collect_status = '%2'"
                                 " WHERE moneyout_collect_id = %1;")
                             .arg(collectionId, collectionStatus);

    if (!updateMoneOut.exec(strCollect)) {
        // if(Debugger) qDebug() << "Error UPDATE terminal_moneyout;";
        // if(Debugger) qDebug() << updateMoneOut.lastError();
        return false;
    }

    return true;
}

bool CollectDaemons::getCollectionInfo(QString collectionId,
                                       QString &collectionIdNext,
                                       QString &idTrn,
                                       QString &dateCreate,
                                       QString &denom_Xml) {
    QSqlQuery selectCollect(db);

    QString strQuery;

    strQuery = QString("SELECT * FROM terminal_collect WHERE collect_id = '%1';").arg(collectionId);

    if (!selectCollect.exec(strQuery)) {
        return false;
    }

    QSqlRecord recordCollect = selectCollect.record();

    if (selectCollect.next()) {
        idTrn = selectCollect.value(recordCollect.indexOf("stack_id")).toString();
        dateCreate = selectCollect.value(recordCollect.indexOf("date_create")).toString();
        denom_Xml = selectCollect.value(recordCollect.indexOf("denom")).toString();
        collectionIdNext = selectCollect.value(recordCollect.indexOf("collect_id_next")).toString();
    }

    return true;
}

bool CollectDaemons::parsDenomilSlot(const QString &xmlDenom) {
    for (auto &b : nominalData.bills) {
        b.value = 0;
    }

    for (auto &c : nominalData.coins) {
        c.value = 0;
    }

    // Объявляем XML объект
    QDom_Document dom_Doc;

    if (dom_Doc.setContent(xmlDenom.toUtf8())) {
        // if(Debugger) qDebug() << "SetContent from collect";
        QDom_Element dom_Element = dom_Doc.documentElement();

        QDom_Node dom_Node = dom_Element.firstChild();
        QDom_Element dom_Element1 = dom_Node.toElement();

        for (int loop = 1; loop <= 9; loop++) {
            int face = dom_Element1.attribute("face", "").toInt();
            int count = dom_Element1.text().toInt();

            for (auto &b : nominalData.bills) {
                if (face == b.face) {
                    b.value = count;
                    break;
                }
            }

            dom_Element1 = dom_Element1.nextSiblingElement();
        }

        for (int loop = 10; loop <= 15; loop++) {
            int face = dom_Element1.attribute("face", "").replace("M", "").toInt();
            int count = dom_Element1.text().toInt();

            for (auto &c : nominalData.coins) {
                if (face == c.face) {
                    c.value = count;
                    break;
                }
            }

            dom_Element1 = dom_Element1.nextSiblingElement();
        }
    } else {
        return false;
    }

    nominalData.calculateTotal();

    return true;
}

bool CollectDaemons::getMoneyOut(const QString &collectionId, int &num_Out, double &sum_Out) {
    QSqlQuery selectMoneyOut(db);

    QString strQuery;

    strQuery = QString("SELECT * FROM terminal_moneyout WHERE moneyout_collect_id = "
                       "'%1' AND moneyout_collect_status = 'send';")
                   .arg(collectionId);
    // if(Debugger) qDebug() << strQuery;

    if (!selectMoneyOut.exec(strQuery)) {
        // if(Debugger) qDebug() << "Error Select moneyout;";
        // if(Debugger) qDebug() << selectMoneyOut.lastError();
        return false;
    }
    // if(Debugger) qDebug() << strQuery;

    QSqlRecord recordCollect = selectMoneyOut.record();
    int num_Money = 0;
    double sum_Money = 0;

    while (selectMoneyOut.next()) {
        double sum = selectMoneyOut.value(recordCollect.indexOf("moneyout_value")).toDouble();
        sum_Money += sum;
        num_Money++;
    }

    num_Out = num_Money;
    sum_Out = sum_Money;

    return true;
}

bool CollectDaemons::getTrnOperation(const QString collectId, const QString &cmd, QString &trn) {
    QSqlQuery selectCollect(db);

    QString strCollect;

    QString minMax = cmd == "MIN" ? "ASC" : "DESC";

    strCollect = QString("SELECT operation_id FROM terminal_operation WHERE "
                         "operation_collect_id = '%1' ORDER BY "
                         "operation_date_create %2 LIMIT 1;")
                     .arg(collectId, minMax);

    if (!selectCollect.exec(strCollect)) {
        // if(Debugger) qDebug() << "Error Select operation trn;";
        if (debugger)
            qDebug() << selectCollect.lastError();
        return false;
    }

    if (debugger)
        qDebug() << strCollect;

    QSqlRecord recordCollect = selectCollect.record();

    trn = "0";

    if (selectCollect.next()) {
        trn = selectCollect.value(recordCollect.indexOf("operation_id")).toString();
        return true;
    }

    return false;
}

bool CollectDaemons::getDatePrevCollection(QString &date, QString collectionId) {
    QSqlQuery selectCollect(db);

    QString strCollect = QString("SELECT date_create FROM terminal_collect WHERE "
                                 "collect_id_next = '%1' LIMIT 1;")
                             .arg(collectionId);

    if (!selectCollect.exec(strCollect)) {
        // if(Debugger) qDebug() << selectCollect.lastError();
        return false;
    }

    QSqlRecord recordCollect = selectCollect.record();

    if (selectCollect.next()) {
        date = selectCollect.value(recordCollect.indexOf("date_create")).toString();
        if (date != "")
            return true;
    }
    return false;
}

QString CollectDaemons::getDenominalXml() {
    QString xml = "<collection>";

    for (auto &bill : nominalData.bills) {
        xml += QString("<bill face=\"%1\">%2</bill>").arg(bill.face).arg(bill.value);
    }

    for (auto &coin : nominalData.coins) {
        xml += QString("<bill face=\"%1M\">%2</bill>").arg(coin.face).arg(coin.value);
    }

    xml += "</collection>";

    return xml;
}

bool CollectDaemons::createNewCollection(QString id) {
    QSqlQuery selectCollect(db);

    auto dateCreate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    auto trnId = "1" + QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz").right(15);
    //    auto colId      = QUuid::createUuid().toString(QUuid::WithoutBraces);
    auto xmlDenom = "";

    QString strQuery = QString("INSERT INTO terminal_collect(collect_id, "
                               "stack_id, date_create, denom, status)"
                               " VALUES('%1', %2 , \"%3\", '%4', 'new');")
                           .arg(id, trnId, dateCreate, xmlDenom);

    if (!selectCollect.exec(strQuery)) {
        qDebug() << selectCollect.lastError().text();
        return false;
    }

    return true;
}

bool CollectDaemons::sendCollection(QString &trnId,
                                    const QString collectionId,
                                    QString &collectionIdNext,
                                    QString &dateCreate,
                                    QString xmlDenom) {
    dateCreate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    trnId = "1" + QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz").right(15);

    auto colIdNext = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery selectCollect(db);

    QString strCollect = QString("UPDATE terminal_collect SET stack_id = %2, date_create = "
                                 "\"%3\", denom = '%4', "
                                 "status = 'send', collect_id_next = '%5' WHERE collect_id = '%1'")
                             .arg(collectionId, trnId, dateCreate, xmlDenom, colIdNext);

    if (!selectCollect.exec(strCollect)) {
        qDebug() << selectCollect.lastError().text();
        return false;
    }

    if (createNewCollection(colIdNext)) {
        collectionIdNext = colIdNext;
    }

    emit emit_Loging(0,
                     senderName,
                     QString("Инкассация с collection_id - %1, stacker_id - %2 "
                             "готова к отправке на сервер")
                         .arg(collectionId, trnId));

    return true;
}

bool CollectDaemons::confirm_Collection(QString collectionId) {
    QSqlQuery updateCollect(db);
    QString strUpdateCollect;
    strUpdateCollect = QString("UPDATE terminal_collect SET status='confirmed' "
                               "WHERE collect_id = '%1'")
                           .arg(collectionId);

    if (!updateCollect.exec(strUpdateCollect)) {
        return false;
    }

    return true;
}

int CollectDaemons::getCollectCount(bool new_collect) {
    QSqlQuery selectCollect(db);

    QString param = "";
    if (new_collect)
        param = "WHERE collect_is_send = 0 OR collect_is_send is NULL";

    QString strCollect =
        QString("SELECT count(*) AS count FROM terminal_collect %1 LIMIT 1;").arg(param);
    if (!selectCollect.exec(strCollect)) {
        return 0;
    }

    QSqlRecord recordCollect = selectCollect.record();

    if (selectCollect.next()) {
        int count = selectCollect.value(recordCollect.indexOf("count")).toInt();
        return count;
    }
    return 0;
}

int CollectDaemons::getNonCollectOperationCount(const QString cid) {
    QSqlQuery selectQuery(db);

    QString query = QString("SELECT count(*) as count FROM terminal_operation "
                            "WHERE operation_collect_id = '%1';")
                        .arg(cid);

    if (!selectQuery.exec(query)) {
        emit emit_Loging(2, senderName, "Ошибка базы данных...");
        return 0;
    }

    QSqlRecord record = selectQuery.record();

    auto count = 0;
    if (selectQuery.next()) {
        count = selectQuery.value(record.indexOf("count")).toInt();
    }

    return count;
}

QString CollectDaemons::getCollectionId(QString status) {
    QSqlQuery selectQuery(db);

    QString strQuery =
        QString("SELECT collect_id FROM terminal_collect WHERE status = '%1';").arg(status);

    if (!selectQuery.exec(strQuery)) {
        if (debugger)
            qDebug() << selectQuery.lastError();
        // if(Debugger) qDebug() << "Error Select new Operation";
        return QString();
    }

    if (debugger)
        qDebug() << strQuery;

    QSqlRecord record = selectQuery.record();
    QString collectId = "";

    if (selectQuery.next()) {
        collectId = selectQuery.value(record.indexOf("collect_id")).toString();
    }

    return collectId;
}

int CollectDaemons::getCollectionCount(QString status) {
    QSqlQuery selectCollect(db);

    QString strQuery =
        QString("SELECT count(*) AS count FROM terminal_collect WHERE status='%1';").arg(status);

    if (!selectCollect.exec(strQuery)) {
        return false;
    }

    QSqlRecord recordCollect = selectCollect.record();

    int count = 0;

    if (selectCollect.next()) {
        count = selectCollect.value(recordCollect.indexOf("count")).toInt();
    }

    return count;
}
