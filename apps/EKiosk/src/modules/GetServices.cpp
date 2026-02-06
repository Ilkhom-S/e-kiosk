#include "GetServices.h"

GetServices::GetServices(QObject *parent) : SendRequest(parent) {
    this->senderName = "GET_SERVICES";
    connect(this, SIGNAL(emit_DomElement(QDomNode)), this, SLOT(setDataNote(QDomNode)));
    //    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(errorResponse()));
}

// void GetServices::errorResponse()
//{
//     getTerminalExtra(infoMap);
//     //Отправляем данные о терминале
//     emit this->emit_infoData(infoMap);

//    emit emit_getServices(true);
//}

void GetServices::setDataNote(const QDomNode &domElement) {
    this->getReqStatus = true;

    bool fullParsing = true;

    banners.clear();

    if (!getProviderCount()) {
        fullParsing = true;
    } else {
        // Парсим хеш
        this->parcerHash(domElement);

        // Сравниваем хешы
        QString db_hash = getHashConfig();

        if (db_hash == infoMap["hash"]) {
            // Берем данные Extra из БД
            fullParsing = false;
            getTerminalExtra(infoMap);
        } else {
            // Парсим полностью
            fullParsing = true;
        }
    }

    if (getReqStatus) {
        // Полный парсинг
        if (fullParsing) {
            // Данные удалились
            if (deleteOldData()) {
                toLogData = "";

                providerList.clear();
                servicesInputs.clear();

                // Парсим данные
                this->parcerNote(domElement);

                if (!getReqStatus) {
                    return;
                }

                // Делаем небольшую проверку
                if (providerList.count() == count_providers) {
                    // Записываем список провайдеров в БД
                    if (!saveServicesDB()) {
                        //                        emit emit_errorDB();
                    }

                    saveServicesInputsDB();

                    // Записываем данные Extra в БД
                    this->insertTerminalExtra(infoMap);
                }
            }
        }

        // Отправляем данные о терминале
        emit this->emit_infoData(infoMap);

        // Отправляем баланс на иследование
        emit this->emit_responseBalance(infoMap["balance"].toDouble(),
                                        infoMap["overdraft"].toDouble(),
                                        infoMap["threshold"].toDouble());

        // Проверяем активен ли терминал
        if (infoMap["active"].toString().trimmed() != "") {
            emit this->emit_responseIsActive(infoMap["active"].toInt() == 1);
        }

        // Получили время server_time
        emit emit_timeServer(infoMap["server_time"].toString());

        emit emit_banners(banners);

        // Говорим что конфигурация загружена
        emit this->emit_getServices(true);

        emit this->emit_Loging(
            0, this->senderName, "Кофигурация получена успешна...\n" + toLogData);
    } else {
        // Конфигурация не загруженна
        emit this->emit_getServices(false);
    }
}

void GetServices::parcerHash(const QDomNode &domElement) {
    QDomNode domNode = domElement.firstChild();

    while (!domNode.isNull()) {
        if (domNode.isElement()) {

            QDomElement domElement = domNode.toElement();
            QString strTag = domElement.tagName();

            // проверяем респонс
            if (strTag == "resultCode") {
                QString sts = domElement.text();
                if (sts == "150" || sts == "151" || sts == "245" || sts == "11" || sts == "12" ||
                    sts == "133" || sts == "14") {
                    emit this->lockUnlockAvtorization(true, sts.toInt());
                    getReqStatus = false;
                    return;
                }

                if (sts == "0") {
                    emit this->lockUnlockAvtorization(false, 0);
                }
            }

            // Данные о дилере
            if (strTag == "balance") {
                infoMap["balance"] = domElement.text();
                toLogData += QString("- Баланс агента              - %1\n")
                                 .arg(infoMap["balance"].toString());
            }

            if (strTag == "overdraft") {
                infoMap["overdraft"] = domElement.text();
                toLogData += QString("- Овердрафт агента           - %1  \n")
                                 .arg(infoMap["overdraft"].toString());
            }

            if (strTag == "threshold") {
                infoMap["threshold"] = domElement.text();
                toLogData += QString("- Порог отключения           - %1  \n")
                                 .arg(infoMap["threshold"].toString());
            }

            if (strTag == "hash") {
                infoMap["hash"] = domElement.text();
                toLogData +=
                    QString("- Идентификатор конфигурации - %1 \n").arg(infoMap["hash"].toString());
            }

            if (strTag == "active") {
                infoMap["active"] = domElement.text();
                toLogData += QString("- Активность терминала       - %1  \n")
                                 .arg(infoMap["active"].toString());
            }

            if (strTag == "cur") {
                int id = domElement.attribute("id", "").toInt();
                this->curMap[id] = domElement.text();
            }

            if (strTag == "server_time") {
                infoMap["server_time"] = domElement.text();
            }

            if (strTag == "coin_acceptor") {
                if (domElement.parentNode().toElement().tagName() == "devices") {
                    infoMap["coin_acceptor"] = domElement.text();
                }
            }

            if (strTag == "banner") {
                QVariantMap banner;
                banner["id"] = domElement.attribute("id").toInt();
                banner["delay"] = domElement.attribute("delay").toInt();
                banner["go_to"] = domElement.attribute("go_to").toInt();
                banner["action"] = domElement.attribute("action");

                banners.append(banner);
            }
        }
        parcerHash(domNode);
        domNode = domNode.nextSibling();
    }
}

void GetServices::parcerNote(const QDomNode &domElement) {
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
                    sts == "133" || sts == "14") {
                    emit this->lockUnlockAvtorization(true, sts.toInt());
                    getReqStatus = false;
                    // Говорим что конфигурация не загруженна
                    emit this->emit_getServices(false);
                    return;
                }

                if (sts == "0") {
                    emit this->lockUnlockAvtorization(false, 0);
                }
            }

            // Узнаем сколько провайдеров
            if (strTag == "providers") {
                count_providers = domNode.childNodes().count();
            }

            // Данные о дилере
            if (strTag == "balance") {
                infoMap["balance"] = domElement.text();
                toLogData += QString("- Баланс агента              - %1\n")
                                 .arg(infoMap["balance"].toString());
            }

            if (strTag == "overdraft") {
                infoMap["overdraft"] = domElement.text();
                toLogData += QString("- Овердрафт агента           - %1  \n")
                                 .arg(infoMap["overdraft"].toString());
            }

            if (strTag == "id_agent") {
                infoMap["id_agent"] = domElement.text();
                toLogData += QString("- ID агента                  - %1  \n")
                                 .arg(infoMap["id_agent"].toString());
            }

            if (strTag == "name") {
                infoMap["name_agent"] = domElement.text().replace("\'", "").replace("\"", "");
                toLogData += QString("- Наименование агента        - %1  \n")
                                 .arg(infoMap["name_agent"].toString());
            }

            if (strTag == "inn") {
                infoMap["inn_agent"] = domElement.text();
                toLogData += QString("- ИНН агента                 - %1  \n")
                                 .arg(infoMap["inn_agent"].toString());
            }

            if (strTag == "phone") {
                infoMap["phone_agent"] = domElement.text().replace("\'", "").replace("\"", "");
                toLogData += QString("- Служба поддержки           - %1 \n")
                                 .arg(infoMap["phone_agent"].toString());
            }

            if (strTag == "threshold") {
                infoMap["threshold"] = domElement.text();
                toLogData += QString("- Порог отключения           - %1  \n")
                                 .arg(infoMap["threshold"].toString());
            }

            if (strTag == "hash") {
                infoMap["hash"] = domElement.text();
                toLogData +=
                    QString("- Идентификатор конфигурации - %1 \n").arg(infoMap["hash"].toString());
            }

            if (strTag == "hash_conf") {
                infoMap["hashxml"] = domElement.text();
                infoMap["path_name"] = domElement.attribute("p", "");
                toLogData += QString("- Идентификатор интерфейса   - %1 \n")
                                 .arg(infoMap["hashxml"].toString());
                toLogData += QString("- Путь для закачки           - %1  \n")
                                 .arg(infoMap["path_name"].toString());
            }

            if (strTag == "currency_usd") {
                infoMap["usd_curce"] = domElement.text();
                toLogData += QString("- Курс доллара               - %1  \n")
                                 .arg(infoMap["usd_curce"].toString());
            }

            if (strTag == "active") {
                infoMap["active"] = domElement.text();
                toLogData += QString("- Активность терминала       - %1  \n")
                                 .arg(infoMap["active"].toString());
            }

            if (strTag == "address") {
                infoMap["address"] = domElement.text().replace("\'", "").replace("\"", "");
                toLogData += QString("- Адрес                      - %1  \n")
                                 .arg(infoMap["address"].toString());
            }

            if (strTag == "cur") {
                int id = domElement.attribute("id", "").toInt();
                this->curMap[id] = domElement.text();
            }

            // Данные провайдера
            if (strTag == "prv") {
                providerList[index_prv]["id"] = domElement.attribute("id", "");
                providerList[index_prv]["nbl"] = domElement.attribute("nbl", "");
                providerList[index_prv]["cid"] = domElement.attribute("cid", "");
                providerList[index_prv]["cms"] = domElement.attribute("cms", "");
                providerList[index_prv]["otp"] = domElement.attribute("otp", "");
                providerList[index_prv]["category_id"] = domElement.attribute("cat", "");
                providerList[index_prv]["amount_min"] = domElement.attribute("min", "");
                providerList[index_prv]["amount_max"] = domElement.attribute("max", "");
                providerList[index_prv]["name_local"] =
                    domElement.attribute("nloc", "").trimmed().replace("'", "''");
                providerList[index_prv]["name_secondary"] =
                    domElement.attribute("nsc", "").trimmed().replace("'", "''");
                providerList[index_prv]["name_ru"] = domElement.attribute("nru", "").trimmed();
                providerList[index_prv]["name_en"] = domElement.attribute("nen", "").trimmed();
                providerList[index_prv]["precheck"] = domElement.attribute("pchk", "");
                providerList[index_prv]["autosum"] = domElement.attribute("asum", "");
                providerList[index_prv]["presum"] = domElement.attribute("psum", "");
                providerList[index_prv]["sms_code"] = domElement.attribute("sms", "");
                providerList[index_prv]["cms_add"] = domElement.attribute("cadd", "");
                providerList[index_prv]["que"] = domElement.attribute("que", "");
                providerList[index_prv]["cms_warn"] = domElement.attribute("cwarn", "");
                providerList[index_prv]["return_max"] = domElement.attribute("hmax", "");
                providerList[index_prv]["regions"] = domElement.attribute("rgns", "");
                providerList[index_prv]["favorite"] = domElement.attribute("faque", "");

                index_prv++;
            }

            if (strTag == "categories") {
                QVariantMap category;
                category["id"] = domElement.attribute("id").toInt();
                category["name_local"] = domElement.attribute("name_loc").replace("'", "''");
                category["name_secondary"] =
                    domElement.attribute("name_secondary").replace("'", "''");
                category["name_ru"] = domElement.attribute("name_ru");
                category["name_en"] = domElement.attribute("name_en");
                category["description_local"] =
                    domElement.attribute("description_loc").replace("'", "''");
                category["description_secondary"] =
                    domElement.attribute("description_secondary").replace("'", "''");
                category["description_ru"] = domElement.attribute("description_ru");
                category["description_en"] = domElement.attribute("description_en");
                category["queue"] = domElement.attribute("queue").toInt();

                if (category.value("queue").toInt() >= 0) {
                    saveCategoriesDB(category);
                }
            }

            if (strTag == "inputs") {
                QVariantMap input;
                input["id"] = domElement.attribute("id").toInt();
                input["input_panel"] = domElement.attribute("ipan").toInt();
                input["regexp"] = domElement.attribute("reg");
                input["mask"] = domElement.attribute("mask");
                input["help_local"] = domElement.attribute("hloc").replace("'", "''");
                input["help_secondary"] = domElement.attribute("hlsl").replace("'", "''");
                input["help_ru"] = domElement.attribute("hru");
                input["help_en"] = domElement.attribute("hen");
                input["placeholder_local"] = domElement.attribute("ploc").replace("'", "''");
                input["placeholder_secondary"] = domElement.attribute("plsl").replace("'", "''");
                input["placeholder_ru"] = domElement.attribute("pru");
                input["placeholder_en"] = domElement.attribute("pen");

                saveInputsDB(input);
            }

            if (strTag == "si") {
                QVariantMap si;
                si["input_id"] = domElement.attribute("input_id");
                si["service_id"] = domElement.attribute("service_id");
                si["field"] = domElement.attribute("field");
                si["field_type"] = domElement.attribute("field_type");
                si["prefix"] = domElement.attribute("prefix");
                si["que"] = domElement.attribute("queue");

                servicesInputs.append(si);
            }

            // Данные о комиссии
            if (strTag == "commission") {
                auto serviceId = domElement.parentNode().toElement().attribute("id", "");
                auto cid = domElement.parentNode().toElement().attribute("cid", "");

                if (serviceId != "") {
                    auto index = domElement.attribute("index", "").toInt();
                    auto sumFrom = domElement.attribute("start", "").toDouble();
                    auto sumTo = domElement.attribute("end", "").toDouble();
                    auto value = domElement.attribute("value", "").toDouble();
                    auto typeId = domElement.attribute("type_id", "").toInt();

                    saveCommissionDB(
                        cid.toInt(), sumFrom, sumTo, typeId, value, serviceId.toInt(), index);
                }
            }

            if (strTag == "server_time") {
                infoMap["server_time"] = domElement.text();
            }

            if (strTag == "coin_acceptor") {
                if (domElement.parentNode().toElement().tagName() == "devices") {
                    infoMap["coin_acceptor"] = domElement.text();
                }
            }

            if (strTag == "banner") {
                QVariantMap banner;
                banner["id"] = domElement.attribute("id").toInt();
                banner["delay"] = domElement.attribute("delay").toInt();
                banner["go_to"] = domElement.attribute("go_to").toInt();
                banner["action"] = domElement.attribute("action");

                banners.append(banner);
            }
        }
        parcerNote(domNode);
        domNode = domNode.nextSibling();
    }
}

bool GetServices::saveServicesDB() {
    int prvCount = providerList.count();

    if (prvCount == 0) {
        return true;
    }

    QSqlQuery sqlQuery(db);

    db.transaction();

    sqlQuery.prepare("INSERT INTO terminal_services( services_id, services_nbl, "
                     "services_cid, services_cms, services_otp, category_id, "
                     "amount_min, amount_max, name_local, name_secondary, "
                     "name_ru, name_en, precheck, autosum, presum, sms_code, "
                     "cms_add, regions, favorite, que, cms_warn, return_max) "
                     "VALUES (:id, :nbl, :cid, :cms, :otp, :cat, :min, :max, "
                     ":nloc, :nsc, :nru, :nen, :pchk, :asum, :psum, :sms, :cadd, "
                     ":rgns, :faque, :que, :cwarn, :hmax);");

    bool result = true;

    for (int i = 0; i < providerList.count(); i++) {
        int id = providerList[i]["id"].toInt();
        int nbl = providerList[i]["nbl"] == "true" ? 1 : 0;
        int cid = providerList[i]["cid"].toInt();
        double cms = cid == 0 ? providerList[i]["cms"].toDouble() : 0;
        int otp = providerList[i]["otp"] == "true" ? 1 : 0;
        int cat = providerList[i]["category_id"].toInt();
        int min = providerList[i]["amount_min"].toInt();
        int max = providerList[i]["amount_max"].toInt();
        QString nloc = providerList[i]["name_local"];
        QString nsc = providerList[i]["name_secondary"];
        QString nru = providerList[i]["name_ru"];
        QString nen = providerList[i]["name_en"];
        int pchk = providerList[i]["precheck"] == "true" ? 1 : 0;
        int asum = providerList[i]["autosum"] == "true" ? 1 : 0;
        int psum = providerList[i]["presum"] == "true" ? 1 : 0;
        int sms = providerList[i]["sms_code"] == "true" ? 1 : 0;
        int cadd = providerList[i]["cms_add"] == "true" ? 1 : 0;
        int que = providerList[i]["que"].toInt();
        int cwarn = providerList[i]["cms_warn"] == "true" ? 1 : 0;
        int hmax = providerList[i]["return_max"] == "true" ? 1 : 0;
        int rgns = providerList[i]["regions"] == "true" ? 1 : 0;
        int faque = providerList[i]["favorite"].toInt();

        sqlQuery.bindValue(":id", id);
        sqlQuery.bindValue(":nbl", nbl);
        sqlQuery.bindValue(":cid", cid);
        sqlQuery.bindValue(":cms", cms);
        sqlQuery.bindValue(":otp", otp);
        sqlQuery.bindValue(":cat", cat);
        sqlQuery.bindValue(":min", min);
        sqlQuery.bindValue(":max", max);
        sqlQuery.bindValue(":nloc", nloc);
        sqlQuery.bindValue(":nsc", nsc);
        sqlQuery.bindValue(":nru", nru);
        sqlQuery.bindValue(":nen", nen);
        sqlQuery.bindValue(":pchk", pchk);
        sqlQuery.bindValue(":asum", asum);
        sqlQuery.bindValue(":psum", psum);
        sqlQuery.bindValue(":sms", sms);
        sqlQuery.bindValue(":cadd", cadd);
        sqlQuery.bindValue(":que", que);
        sqlQuery.bindValue(":cwarn", cwarn);
        sqlQuery.bindValue(":hmax", hmax);
        sqlQuery.bindValue(":rgns", rgns);
        sqlQuery.bindValue(":faque", faque);

        if (!sqlQuery.exec()) {
            result = false;
        }
    }

    if (result) {
        db.commit();
    } else {
        db.rollback();
    }

    return result;
}

bool GetServices::saveCommissionDB(const int id,
                                   const double sumFrom,
                                   const double sumTo,
                                   const int type,
                                   const double value,
                                   const int serviceId,
                                   const int index) {
    // Создаем объект QSqlQuery
    QSqlQuery insertDataQuery(this->db);

    QString query = QString("INSERT INTO terminal_commission(commission_id, commission_sum_from, "
                            "commission_sum_to, commission_i_type, commission_value, service_id, "
                            "idx) VALUES ( %1, '%2', '%3', %4, '%5', '%6', '%7');")
                        .arg(id)
                        .arg(sumFrom)
                        .arg(sumTo)
                        .arg(type)
                        .arg(value)
                        .arg(serviceId)
                        .arg(index);

    // Записываем данные в Базу
    if (!insertDataQuery.exec(query)) {
        // if(Debugger)
        qDebug() << insertDataQuery.lastError();
        return false;
    }

    return true;
}

bool GetServices::saveCategoriesDB(const QVariantMap category) {
    // Создаем объект QSqlQuery
    QSqlQuery insertDataQuery(this->db);

    QString query = QString("INSERT INTO terminal_categories(id, name_local, name_secondary, "
                            "name_ru, name_en, description_local, description_secondary, "
                            "description_ru, description_en, que) VALUES ( %1, '%2', '%3', "
                            "'%4', '%5', '%6', '%7', '%8', '%9', %10);")
                        .arg(category.value("id").toInt())
                        .arg(category.value("name_local").toString())
                        .arg(category.value("name_secondary").toString())
                        .arg(category.value("name_ru").toString())
                        .arg(category.value("name_en").toString())
                        .arg(category.value("description_local").toString())
                        .arg(category.value("description_secondary").toString())
                        .arg(category.value("description_ru").toString())
                        .arg(category.value("description_en").toString())
                        .arg(category.value("queue").toInt());

    // Записываем данные в Базу
    if (!insertDataQuery.exec(query)) {
        // if(Debugger)
        qDebug() << insertDataQuery.lastError();
        return false;
    }

    return true;
}

bool GetServices::saveInputsDB(const QVariantMap input) {
    // Создаем объект QSqlQuery
    QSqlQuery insertDataQuery(this->db);

    QString query = QString("INSERT INTO terminal_inputs(id, input_panel, regexp, mask, "
                            "help_local, help_secondary, help_ru, help_en, placeholder_local, "
                            "placeholder_secondary, placeholder_ru, placeholder_en) VALUES ( %1, "
                            "%2, '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10', '%11', '%12');")
                        .arg(input.value("id").toInt())
                        .arg(input.value("input_panel").toInt())
                        .arg(input.value("regexp").toString())
                        .arg(input.value("mask").toString())
                        .arg(input.value("help_local").toString())
                        .arg(input.value("help_secondary").toString())
                        .arg(input.value("help_ru").toString())
                        .arg(input.value("help_en").toString())
                        .arg(input.value("placeholder_local").toString())
                        .arg(input.value("placeholder_secondary").toString())
                        .arg(input.value("placeholder_ru").toString())
                        .arg(input.value("placeholder_en").toString());

    // Записываем данные в Базу
    if (!insertDataQuery.exec(query)) {
        // if(Debugger)
        qDebug() << insertDataQuery.lastError();
        return false;
    }

    return true;
}

bool GetServices::saveServicesInputsDB() {
    if (servicesInputs.count() == 0) {
        return true;
    }

    QSqlQuery sqlQuery(db);

    db.transaction();

    sqlQuery.prepare("INSERT INTO terminal_services_inputs(input_id, service_id, field, "
                     "field_type, prefix, que) "
                     "VALUES (:input_id, :service_id, :field, :field_type, :prefix, :que);");

    bool result = true;

    for (auto &i : servicesInputs) {
        auto input = i.toMap();

        sqlQuery.bindValue(":input_id", input.value("input_id").toInt());
        sqlQuery.bindValue(":service_id", input.value("service_id").toInt());
        sqlQuery.bindValue(":field", input.value("field").toString());
        sqlQuery.bindValue(":field_type", input.value("field_type").toString());
        sqlQuery.bindValue(":prefix", input.value("prefix").toString());
        sqlQuery.bindValue(":que", input.value("que").toInt());

        if (!sqlQuery.exec()) {
            result = false;
        }
    }

    if (result) {
        db.commit();
    } else {
        db.rollback();
    }

    return result;
}

bool GetServices::insertTerminalExtra(QVariantMap map) {
    // Создаем объект QSqlQuery
    QSqlQuery insertDataQuery(this->db);

    QString query = QString("INSERT INTO terminal_extra( id, id_agent, name, inn, phone, "
                            "threshold, hash, hash_conf, currency_usd, active, address, "
                            "path, server_time ) VALUES ( 1, '%1', '%2', '%3', '%4', '%5', "
                            "'%6', '%7', '%8', '%9', '%10', '%11', '%12');")
                        .arg(map["id_agent"].toString())
                        .arg(map["name_agent"].toString())
                        .arg(map["inn_agent"].toString())
                        .arg(map["phone_agent"].toString())
                        .arg(map["threshold"].toString())
                        .arg(map["hash"].toString())
                        .arg(map["hashxml"].toString())
                        .arg(map["usd_curce"].toString())
                        .arg(map["active"].toString())
                        .arg(map["address"].toString())
                        .arg(map["path_name"].toString())
                        .arg(map["server_time"].toString());

    //    if(Debugger) qDebug() << query;

    // Записываем данные в Базу
    if (!insertDataQuery.exec(query)) {
        if (debugger)
            qDebug() << insertDataQuery.lastError();
        return false;
    }

    return true;
}

bool GetServices::getTerminalExtra(QVariantMap &map) {
    QSqlQuery selectQuery(this->db);
    QString strSelect = QString("SELECT * FROM terminal_extra;");

    if (!selectQuery.exec(strSelect)) {
        return false;
    }

    QSqlRecord record = selectQuery.record();

    if (selectQuery.next()) {
        map["id_agent"] = selectQuery.value(record.indexOf("id_agent")).toString();
        map["name_agent"] = selectQuery.value(record.indexOf("name")).toString();
        map["inn_agent"] = selectQuery.value(record.indexOf("inn")).toString();
        map["phone_agent"] = selectQuery.value(record.indexOf("phone")).toString();
        map["hashxml"] = selectQuery.value(record.indexOf("hash_conf")).toString();
        map["usd_curce"] = selectQuery.value(record.indexOf("currency_usd")).toString();
        map["address"] = selectQuery.value(record.indexOf("address")).toString();
        map["path_name"] = selectQuery.value(record.indexOf("path")).toString();
    }

    return true;
}

QString GetServices::getHashConfig() {
    QSqlQuery userSql(this->db);

    QString userQuery = "SELECT * FROM terminal_extra;";

    if (!userSql.exec(userQuery)) {
        return "";
    }

    QSqlRecord record = userSql.record();

    QString hash_config = "";

    if (userSql.next()) {
        hash_config = userSql.value(record.indexOf("hash")).toString();
    }

    return hash_config;
}

int GetServices::getProviderCount() {
    QSqlQuery querySql(this->db);

    QString sqlQuery = "SELECT count(*) AS count FROM terminal_services;";

    if (!querySql.exec(sqlQuery)) {
        return 0;
    }

    if (!querySql.isSelect()) {
        return 0;
    } else {
        QSqlRecord record = querySql.record();

        if (querySql.next()) {
            int id = querySql.value(record.indexOf("count")).toInt();
            return id;
        }
    }

    return 0;
}

void GetServices::sendGetServicesQuery() {
    QString header_xml = getHeaderRequest(Request::Type::GetServices);
    QString footer_xml = getFooterRequest();

    QString request = header_xml + footer_xml;

    // Обънуляем переменные
    index_prv = 0;
    count_providers = 0;

    infoMap.clear();
    toLogData = "";

    //    if(Debugger) qDebug() << "\n================REQUEST=================\n";
    //    if(Debugger) qDebug() << request;

    emit emit_Loging(0, senderName, "Отправлен запрос на получение конфигурации.");

    if (!sendRequest(request, 60000)) {
        emit emit_ErrResponse();
    }
}

bool GetServices::deleteOldData() {
    bool result = true;

    // Создаем объект QSqlQuery
    QSqlQuery deleteData(this->db);

    // Очищаем Таблицы
    QString sqlDeletePrv = "DELETE FROM terminal_services;";

    if (!deleteData.exec(sqlDeletePrv)) {
        if (debugger)
            qDebug() << deleteData.lastError();
        result = false;
    }

    sqlDeletePrv = "DELETE FROM terminal_categories;";

    if (!deleteData.exec(sqlDeletePrv)) {
        if (debugger)
            qDebug() << deleteData.lastError();
        result = false;
    }

    sqlDeletePrv = "DELETE FROM terminal_commission;";

    if (!deleteData.exec(sqlDeletePrv)) {
        if (debugger)
            qDebug() << deleteData.lastError();
        result = false;
    }

    sqlDeletePrv = "DELETE FROM terminal_extra;";

    if (!deleteData.exec(sqlDeletePrv)) {
        if (debugger)
            qDebug() << deleteData.lastError();
        result = false;
    }

    sqlDeletePrv = "DELETE FROM terminal_inputs;";

    if (!deleteData.exec(sqlDeletePrv)) {
        if (debugger)
            qDebug() << deleteData.lastError();
        result = false;
    }

    sqlDeletePrv = "DELETE FROM terminal_services_inputs;";

    if (!deleteData.exec(sqlDeletePrv)) {
        if (debugger)
            qDebug() << deleteData.lastError();
        result = false;
    }

    return result;
}
