#include "PayDaemons.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QUuid>

PayDaemons::PayDaemons(QObject *parent)
    : SendRequest(parent), count_non_send(0), restatTimer(new QTimer(this)),
      payTimer(new QTimer(this)), firstSend(true), abortPresent(false) {

    senderName = "PAY_DAEMONS";

    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));

    restatTimer->setSingleShot(true);
    connect(restatTimer, SIGNAL(timeout()), this, SIGNAL(emit_RestartTerminal()));

    connect(payTimer, SIGNAL(timeout()), this, SLOT(sendPayRequest()));
    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(getErrResponse()));
}

void PayDaemons::startTimer(const int sec) {
    if (!payTimer->isActive()) {
        payTimer->start(sec * 1000);
    }
}

void PayDaemons::getErrResponse() {
    if (status_get) {
        // Если есть статус отправляем сигнал на main для проверки номера
        emit emit_AnimateForSearch(false);
        emit emit_statusUpdatetoIn();
        // if(Debugger) qDebug() <<  "---emit emit_statusUpdatetoIn();---";
        status_get = false;
    }

    abortPresent = true;
}

void PayDaemons::setDataNote(const QDomNode &domElement) {
    abortPresent = false;

    if (restatTimer->isActive()) {
        restatTimer->stop();
    }

    status_get = false;
    gbl_overdraft = 999.999;
    gbl_balance = 999.999;

    // Парсим данные
    parseNode(domElement);

    // Делаем небольшую проверку
    if (gbl_balance != 999.999 && gbl_overdraft != 999.999) {
        // Отправляем баланс на иследование
        emit emit_responseBalance(gbl_balance, gbl_overdraft, 1.111);
        firstSend = false;
    }

    if (status_get) {
        // Если есть статус отправляем сигнал на main для проверки номера
        emit emit_AnimateForSearch(false);
        emit emit_statusUpdatetoIn();

        status_get = false;
    }
}

void PayDaemons::parseNode(const QDomNode &domElement) {

    // Необходимо отпарсить документ
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull()) {
        if (domNode.isElement()) {

            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            // if(Debugger) qDebug() << strTag + " " + domElement.text();

            // проверям респонс
            if (strTag == "resultCode") {
                QString sts = domElement.text();
                if (sts == "150" || sts == "151" || sts == "245" || sts == "11" || sts == "12" ||
                    sts == "133") {
                    emit lockUnlockAuthorization(true, sts.toInt());
                    return;
                }

                if (sts.toInt() > 0) {
                    emit emit_Loging(
                        2, senderName, QString("Пришел ответ от сервера resultCode: %1").arg(sts));
                }
            }

            // Данные о дилере
            if (strTag == "balance") {
                gbl_balance = domElement.text().toDouble();
            }

            if (strTag == "overdraft") {
                gbl_overdraft = domElement.text().toDouble();
            }

            if (strTag == "status") {
                // Если это статус даем знать что ответ есть
                status_get = true;
            }

            // проверям респонс платежей
            if (strTag == "payment") {
                QString strSts = domElement.attribute("resultCode", "");
                QString vrmTrn = domElement.attribute("trn", "");
                QString vrmDataConfirm = domElement.attribute("timeget", "");

                // обновляем данные о платеже
                bool updateSts = updateOperationStatus(vrmTrn, strSts, vrmDataConfirm);

                if (!updateSts) {
                    emit emit_Loging(
                        1,
                        senderName,
                        QString("Не могу обновить платеж с транзакцией - %1.").arg(vrmTrn));
                } else {
                    emit emit_Loging(0,
                                     senderName,
                                     QString("Платеж с транзакцией - %1: обновлен на "
                                             "статус - %2 в %3 часов.")
                                         .arg(vrmTrn, strSts, vrmDataConfirm));

                    count_non_send = 0;

                    // Отправляем сигнал на разблокирование терминала по резулт кодам
                    emit lockUnlockAuthorization(false, 0);
                }
            }
        }

        parseNode(domNode);
        domNode = domNode.nextSibling();
    }
}

void PayDaemons::sendPayRequest() {
    sendPaymentToServer(firstSend);
}

void PayDaemons::get_new_pay(QVariantMap payment) {
    auto account = payment.value("account").toString();
    auto accountMasked = payment.value("account_masked").toString();
    auto prvId = payment.value("prv_id").toString();
    auto prvName = payment.value("prv_name").toString();
    auto trnId = payment.value("trn_id").toString();
    auto sumFrom = payment.value("sum_from").toDouble();
    auto sumTo = payment.value("sum_to").toDouble();
    auto denomination = payment.value("denomination").toString();
    auto ratioPercent = payment.value("ratio_percent").toDouble();
    auto ratioSum = payment.value("ratio_sum").toDouble();
    auto notifyRoute = payment.value("notify_route").toString();

    auto extraInfo = payment.value("extra_info").toMap();
    QString extra = extraInfo.isEmpty()
                        ? ""
                        : QJsonDocument::fromVariant(extraInfo).toJson(QJsonDocument::Compact);

    if (GetOperationCount(trnId)) {
        emit emit_Loging(1,
                         senderName,
                         QString("Такая транзакция уже есть в базе operation_id - %1.").arg(trnId));
        return;
    }

    // дата создания
    auto dateCreate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    auto collectionId = getCollectionId();

    QSqlQuery createNewOperation(db);

    QString strCreateNewOperation =
        QString("INSERT INTO terminal_operation"
                " (operation_ratio_number, operation_ratio_persent, "
                "operation_account_design, operation_account"
                ", operation_collect_id, operation_date_create, "
                "operation_date_confirm, operation_id"
                ", operation_is_send, operation_money_amount, "
                "operation_money_cash, operation_money_denom"
                ", operation_services_id, operation_status, "
                "operation_print_status, operation_services_name, "
                "notify_route, extra_info)"
                " VALUES(%1,%2,'%3','%4','%5','%6','%6',%7,0,%8,%9,'%10',%11,54,"
                "0,'%12','%13','%14');")
            .arg(ratioSum)
            .arg(ratioPercent)
            .arg(accountMasked, account, collectionId, dateCreate, trnId)
            .arg(sumTo)
            .arg(sumFrom)
            .arg(denomination, prvId, prvName, notifyRoute, extra);

    // if(Debugger) qDebug() << strCreateNewOperation;

    if (!createNewOperation.exec(strCreateNewOperation)) {
        // if(Debugger) qDebug() << "Error INSERT INTO operation new Data\n";
        // if(Debugger) qDebug() << createNewOperation.lastError();
        emit emit_Loging(2,
                         senderName,
                         QString("Ошибка Базы Данных... Невозможно создать платёж (номер - %1, "
                                 "сумма - %2). Error: %3. Query: %4")
                             .arg(account)
                             .arg(sumFrom)
                             .arg(createNewOperation.lastError().text(), strCreateNewOperation));
        emit emit_errorDB();
        return;
    }

    emit emit_Loging(0,
                     senderName,
                     QString("Платеж с номером- %1, на провайдер- %2,"
                             " транзакцией- %3, суммой от- %4, суммой к- %5,"
                             " занесен в базу %6 числа.")
                         .arg(account, prvName, trnId)
                         .arg(sumFrom)
                         .arg(sumTo)
                         .arg(dateCreate));
}

void PayDaemons::get_update_pay(QVariantMap payment) {
    auto trnId = payment.value("trn_id").toString();
    auto sumFrom = payment.value("sum_from").toDouble();
    auto sumTo = payment.value("sum_to").toDouble();
    auto denomination = payment.value("denomination").toString();
    auto ratioPercent = payment.value("ratio_percent").toDouble();
    auto ratioSum = payment.value("ratio_sum").toDouble();

    // дата создания
    QString dateCreate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QSqlQuery updateOperation(db);
    QString strUpdateOperation = QString("UPDATE terminal_operation SET"
                                         " operation_ratio_number = %1,"
                                         " operation_ratio_persent = %2,"
                                         " operation_date_create = '%3',"
                                         " operation_date_confirm = '%3',"
                                         " operation_money_amount = %4,"
                                         " operation_money_cash = %5,"
                                         " operation_money_denom = '%6'"
                                         " WHERE operation_id = %7")
                                     .arg(ratioSum)
                                     .arg(ratioPercent)
                                     .arg(dateCreate)
                                     .arg(sumTo)
                                     .arg(sumFrom)
                                     .arg(denomination, trnId);

    if (!updateOperation.exec(strUpdateOperation)) {
        // if(Debugger) qDebug() << "Error Update Operation for status LOCK in
        // function get_update_pay";
        emit emit_Loging(2,
                         senderName,
                         QString("Ошибка Базы Данных... Невозможно обновить платёж. Error: %1. "
                                 "Query: %2")
                             .arg(updateOperation.lastError().text(), strUpdateOperation));
        emit emit_errorDB();
        return;
    }

    emit emit_Loging(0,
                     senderName,
                     QString("Платеж с"
                             " транзакцией- %1, суммой от- %2, суммой к- %3,"
                             " обновил параметры в базе %4 числа.")
                         .arg(trnId)
                         .arg(sumFrom)
                         .arg(sumTo)
                         .arg(dateCreate));
}

bool PayDaemons::GetOperationCount(const QString &idTrn) {
    QSqlQuery selectNewOperationQuery(db);

    QString operationQuery = QString("SELECT count(*) AS count FROM terminal_operation WHERE "
                                     "operation_id = %1 LIMIT 1;")
                                 .arg(idTrn);

    if (!selectNewOperationQuery.exec(operationQuery)) {
        // if(Debugger) qDebug() << selectNewOperationQuery.lastError();
        // if(Debugger) qDebug() << "Error Select Operation count from operation_id";
        return false;
    }

    QSqlRecord record = selectNewOperationQuery.record();

    if (selectNewOperationQuery.next()) {
        int count = selectNewOperationQuery.value(record.indexOf("count")).toInt();

        if (count > 0) {
            return true;
        }
    }

    return false;
}

void PayDaemons::get_confirm_pay(QString tranzId, bool print) {
    // if(Debugger) qDebug() <<  "update NewOperation-->  with status
    // confirm_create";

    QDateTime date;
    // дата создания
    QString vrmDateCreate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QSqlQuery updateOperation(db);

    QString strUpdateOperation;

    if (print) {
        strUpdateOperation = QString("UPDATE terminal_operation SET"
                                     " operation_print_status = 1"
                                     " WHERE operation_id = %1")
                                 .arg(tranzId);
    } else {
        strUpdateOperation = QString("UPDATE terminal_operation SET"
                                     " operation_status = 55"
                                     " WHERE operation_id = %1 AND operation_status = 54;")
                                 .arg(tranzId);
    }

    if (!updateOperation.exec(strUpdateOperation)) {
        // if(Debugger) qDebug() << "Error Update Operation for status LOCK in
        // function get_confirm_pay)";
        return;
    }

    if (print) {
        emit emit_Loging(0,
                         senderName,
                         QString("К платежу с транзакцией- %1 чек был распечатан %2 числа.")
                             .arg(tranzId, vrmDateCreate));
    } else {
        emit emit_Loging(0,
                         senderName,
                         QString("Платеж с транзакцией- %1 полностью оплачен %2 числа.")
                             .arg(tranzId, vrmDateCreate));

        // моментально отправляем платеж
        this->sendPayRequest();
    }
}

void PayDaemons::get_print_id(QString tranzId) {
    // Берём текст печати
    QString chec = getReceiptInfo(tranzId);

    // Сигнал на печать
    emit emit_to_print(chec);
}

QString PayDaemons::getReceiptInfo(QString tranzId) {
    QString rec = receiptGet(tpl);

    // Данные о платеже
    QString dateCreate;
    QString prvName;
    QString account;
    QString sumFrom;
    QString sumTo;
    QString ratioSum;
    QString ratioPersent;

    if (!getPayData(
            tranzId, dateCreate, prvName, account, sumFrom, sumTo, ratioSum, ratioPersent)) {
        dateCreate = "2018-01-01 11:11:11";
        prvName = "Babilon NGN";
        account = "44-640-5544";
        sumFrom = "00";
        sumTo = "00";
        ratioSum = "0";
        ratioPersent = "0";
    }

    rec = rec.arg(tranzId,
                  num_Trm,
                  dateCreate,
                  kassir,
                  "",
                  prvName,
                  account,
                  sumFrom,
                  sumTo,
                  ratioSum,
                  phone);

    return rec;
}

bool PayDaemons::getPayData(QString idTrn,
                            QString &dateCreate,
                            QString &prvName,
                            QString &account,
                            QString &sumFrom,
                            QString &sumTo,
                            QString &ratioSum,
                            QString &ratioPersent) {
    QSqlQuery selectOperation(db);

    QString strOperation =
        QString("SELECT * FROM terminal_operation WHERE operation_id = '%1' LIMIT 1;").arg(idTrn);

    if (!selectOperation.exec(strOperation)) {
        return false;
    }

    QSqlRecord recordOperation = selectOperation.record();

    if (selectOperation.next()) {
        dateCreate =
            selectOperation.value(recordOperation.indexOf("operation_date_create")).toString();
        prvName =
            selectOperation.value(recordOperation.indexOf("operation_services_name")).toString();
        account =
            selectOperation.value(recordOperation.indexOf("operation_account_design")).toString();
        sumFrom = selectOperation.value(recordOperation.indexOf("operation_money_cash")).toString();
        sumTo = selectOperation.value(recordOperation.indexOf("operation_money_amount")).toString();

        ratioSum =
            selectOperation.value(recordOperation.indexOf("operation_ratio_number")).toString();
        ratioPersent =
            selectOperation.value(recordOperation.indexOf("operation_ratio_persent")).toString();

        return true;
    }

    return false;
}

void PayDaemons::sendPaymentToServer(bool withNon) {
    bool exitP = false;

    // При первом запуске
    if (withNon) {
        // Обновляем статус 54 и 3 на 55
        confirm_Payments();

        firstSend = false;
    }

    // Берем количество платежей
    int countO = 0;
    // TODO getCountPayment first send
    bool res = getCountPayment(countO);

    if (res) {
        bool payUnlokSts = false;

        // Проверяем количество платежей
        if (countO > 0) {

            //            if(count_o == 3 && abortPresent){
            //                //Надо проверить были ли аборты
            //                //Отправляем сигнал на перезагрузку соединения
            //                emit emit_RestartNet();
            //                abortPresent = false;
            //            }

            // Проверяем блокировать ли терминал
            if (countO >= 1) {
                count_non_send += 1;

                if (count_non_send > 3) {
                    payUnlokSts = true;
                    emit emit_Loging(1, senderName, QString("Количество платежей в системе >= 1"));
                }

                // Попробуем перезагрузить терминал через 30 мин.
                if (!restatTimer->isActive() && count_non_send == 20) {
                    restatTimer->start(300000);
                    emit emit_Loging(0, senderName, "Начинаем перезагрузку АСО через 5 мин.");
                }
            }
        } else {
            // Выходим из процедуры
            //            withNon = false;
            firstSend = false;
            exitP = true;
        }

        // Отправляем параметр блокировки
        emit lockUnlockNonSend(payUnlokSts);
    } else {
        // Выходим из процедуры
        //        withNon = false;
        firstSend = false;
        exitP = true;
    }

    // Выходим из процедуры
    if (exitP) {
        return;
    }

    QString paymentXml = "";
    int countPay = 0;
    double allSum = 0;

    if (getPaymentMap(paymentXml, countPay, allSum)) {
        // Если есть результат
        // Смотрим сколько платежей
        if (countPay > 0) {
            // Формируем XML для отправки на сервер

            QString headerXml = getHeaderRequest(Request::Type::PayAuth);

            QString footer = getFooterRequest();

            QString request = QString(headerXml +
                                      "<auth count=\"%1\" toAmount=\"%2\">\n"
                                      "%3"
                                      "</auth>\n" +
                                      footer)
                                  .arg(countPay)
                                  .arg(allSum)
                                  .arg(paymentXml);

            if (debugger) {
                qDebug() << request;
            }
            sendRequest(request, 25000);
        }
    }
}

bool PayDaemons::getPaymentMap(QString &payment, int &countPay, double &allSum) {
    QSqlQuery selectOperationForSend(db);

    QString strQueryOperation = QString("SELECT * FROM terminal_operation"
                                        " WHERE operation_status = 55 LIMIT 1;");

    if (!selectOperationForSend.exec(strQueryOperation)) {
        // if(Debugger) qDebug() << selectOperationForSend.lastError();
        // if(Debugger) qDebug() << "Error Select Operation Data for Send in function
        // bool PayDaemons::SendNewOperation()";
        return false;
    }

    QSqlRecord recordOperationForSend = selectOperationForSend.record();

    while (selectOperationForSend.next()) {

        QString vrmIdTrn =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_id")).toString();
        QString vrmFSum =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_money_cash"))
                .toString();
        double vrmTSum =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_money_amount"))
                .toDouble();
        QString billInfo =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_money_denom"))
                .toString();
        QString vrmPrvId =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_services_id"))
                .toString();
        QString vrmAccount =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_account"))
                .toString();
        QString vrmDateCreate =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_date_create"))
                .toString();
        QString stsPrint =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_print_status"))
                .toString();
        QString collectionId =
            selectOperationForSend.value(recordOperationForSend.indexOf("operation_collect_id"))
                .toString();
        QString vrmNotifyRoute =
            selectOperationForSend.value(recordOperationForSend.indexOf("notify_route")).toString();
        QString extraInfo =
            selectOperationForSend.value(recordOperationForSend.indexOf("extra_info")).toString();

        // Убираем лишние знаки
        vrmDateCreate.replace("-", "");
        vrmDateCreate.replace(":", "");
        vrmDateCreate.replace(" ", "");

        QString extra = extraInfo.isEmpty() ? "" : QString("param='%12'").arg(extraInfo);

        payment += QString("<payment trn=\"%1\" fsum=\"%2\" tsum=\"%3\" prv_id=\"%4\" "
                           "account=\"%5\" receipt=\"%6\" time=\"%7\" "
                           "receipt_sts=\"%8\" bill_info=\"%9\" collection_id=\"%10\" "
                           "notify_route=\"%11\" %12/>\n")
                       .arg(vrmIdTrn, vrmFSum)
                       .arg(vrmTSum)
                       .arg(vrmPrvId,
                            vrmAccount,
                            vrmIdTrn,
                            vrmDateCreate,
                            stsPrint,
                            billInfo,
                            collectionId,
                            vrmNotifyRoute,
                            extra);

        countPay++;
        allSum += vrmTSum;

        emit emit_Loging(0,
                         senderName,
                         QString("Платеж с номером- %1 суммой на счет- %2 и "
                                 "транзакцией- %3 в PAY_REQUEST_XML сформирован.")
                             .arg(vrmAccount)
                             .arg(vrmTSum)
                             .arg(vrmIdTrn));
    }
    return true;
}

bool PayDaemons::updateOperationStatus(const QString &idTrm,
                                       const QString &status,
                                       const QString &dateConfirm) {
    QSqlQuery updateOperation(db);
    QString strUpdateOperation;
    QString strSend = "";
    if (dateConfirm != "0") {
        strSend = ", operation_is_send = 1";
    }
    strUpdateOperation = QString("UPDATE terminal_operation SET operation_status "
                                 "= %2, operation_date_confirm = '%3'%4 "
                                 "WHERE operation_id = %1")
                             .arg(idTrm, status, dateConfirm, strSend);

    if (!updateOperation.exec(strUpdateOperation)) {
        // if(Debugger) qDebug() << updateOperation.lastError();
        // if(Debugger) qDebug() << "Error Update Operation for status UNKNOWN in
        // function bool PayDaemons::UpdateOperationForUnknown(const QString
        // &id_trm)";
        return false;
    }
    return true;
}

bool PayDaemons::getCountPayment(int &count) {
    QSqlQuery selectOperationQuery(db);

    QString operationQuery = QString("SELECT count(*) AS count FROM terminal_operation WHERE "
                                     "operation_status = 55 LIMIT 1;");
    // if(Debugger) qDebug() << operationQuery;

    if (!selectOperationQuery.exec(operationQuery)) {
        // if(Debugger) qDebug() << selectOperationQuery.lastError();
        // if(Debugger) qDebug() << "Error Select new Operation";
        return false;
    }

    QSqlRecord record = selectOperationQuery.record();

    if (selectOperationQuery.next()) {
        count = selectOperationQuery.value(record.indexOf("count")).toInt();
        return true;
    }

    return false;
}

void PayDaemons::checkPayStatus54() {

    QSqlQuery selectOperationQuery(db);

    int unconfrmPayCount = 0;

    // TODO remove after update operation_status = 3
    QString operationQuery = QString("SELECT count(*) AS count FROM terminal_operation WHERE "
                                     "operation_status = 54 OR operation_status = 3;");
    // if(Debugger) qDebug() << operationQuery;

    if (!selectOperationQuery.exec(operationQuery)) {
        // if(Debugger) qDebug() << selectOperationQuery.lastError();
        // if(Debugger) qDebug() << "Error Select new Operation";
        return;
    }

    QSqlRecord record = selectOperationQuery.record();

    if (selectOperationQuery.next()) {
        unconfrmPayCount = selectOperationQuery.value(record.indexOf("count")).toInt();
    }

    // есть ли платеж(и) со статусом 54
    if (unconfrmPayCount > 0) {
        // обновим на статус 55
        confirm_Payments();
    }
}

void PayDaemons::confirm_Payments() {
    QSqlQuery updateOperation(db);

    QString strUpdateOperation;

    // TODO remove after update operation_status = 3
    strUpdateOperation = QString("UPDATE terminal_operation SET operation_status = 55  WHERE "
                                 "operation_status = 54 OR operation_status = 3;");

    if (!updateOperation.exec(strUpdateOperation)) {
        // if(Debugger) qDebug() << updateOperation.lastError();
        return;
    }
}

QString PayDaemons::getCollectionId() {
    QSqlQuery selectQuery(db);

    QString strQuery = QString("SELECT collect_id FROM terminal_collect WHERE status='new';");

    if (!selectQuery.exec(strQuery)) {
        // if(Debugger) qDebug() << selectOperationQuery.lastError();
        // if(Debugger) qDebug() << "Error Select new Operation";
        return {};
    }

    QSqlRecord record = selectQuery.record();
    QString collectId = "";

    if (selectQuery.next()) {
        collectId = selectQuery.value(record.indexOf("collect_id")).toString();
    }

    // Создаём инкассацию если нет нулевая инкассация
    if (collectId == "") {
        auto dateCreate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        auto trnId = "1" + QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz").right(15);
        auto colId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto *xmlDenom = "";

        strQuery = QString("INSERT INTO terminal_collect(collect_id, stack_id, "
                           "date_create, denom, status)"
                           " VALUES('%1', %2 , \"%3\", '%4', 'new');")
                       .arg(colId, trnId, dateCreate, xmlDenom);

        if (!selectQuery.exec(strQuery)) {
            return {};
        }

        collectId = colId;
    }

    return collectId;
}
