#include "GetServices.h"

GetServices::GetServices(QObject *parent) : SendRequest(parent) {
    this->senderName = "GET_SERVICES";
    connect(this, SIGNAL(emit_Dom_Element(QDomNode)), this, SLOT(setDataNote(QDomNode)));
    //    connect(this, SIGNAL(emit_ErrResponse()), this, SLOT(errorResponse()));
}

// void GetServices::errorResponse()
//{
//     getTerminalExtra(infoMap);
//     //Отправляем данные о терминале
//     emit this->emit_infoData(infoMap);

//    emit emit_getServices(true);
//}

void GetServices::setDataNote(const QDomNode &dom_Element) {
    this->getReqStatus = true;

    bool fullParsing = true;

    banners.clear();

    if (!getProviderCount()) {
        fullParsing = true;
    } else {
        // Парсим хеш
        this->parcerHash(dom_Element);

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
                this->parcerNote(dom_Element);

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

void GetServices::parcerHash(const QDomNode &dom_Element) {
    QDomNode dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {

            QDomElement dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            // проверяем респонс
            if (strTag == "resultCode") {
                QString sts = dom_Element.text();
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
                infoMap["balance"] = dom_Element.text();
                toLogData += QString("- Баланс агента              - %1\n")
                                 .arg(infoMap["balance"].toString());
            }

            if (strTag == "overdraft") {
                infoMap["overdraft"] = dom_Element.text();
                toLogData += QString("- Овердрафт агента           - %1  \n")
                                 .arg(infoMap["overdraft"].toString());
            }

            if (strTag == "threshold") {
                infoMap["threshold"] = dom_Element.text();
                toLogData += QString("- Порог отключения           - %1  \n")
                                 .arg(infoMap["threshold"].toString());
            }

            if (strTag == "hash") {
                infoMap["hash"] = dom_Element.text();
                toLogData +=
                    QString("- Идентификатор конфигурации - %1 \n").arg(infoMap["hash"].toString());
            }

            if (strTag == "active") {
                infoMap["active"] = dom_Element.text();
                toLogData += QString("- Активность терминала       - %1  \n")
                                 .arg(infoMap["active"].toString());
            }

            if (strTag == "cur") {
                int id = dom_Element.attribute("id", "").toInt();
                this->curMap[id] = dom_Element.text();
            }

            if (strTag == "server_time") {
                infoMap["server_time"] = dom_Element.text();
            }

            if (strTag == "coin_acceptor") {
                if (dom_Element.parentNode().toElement().tagName() == "devices") {
                    infoMap["coin_acceptor"] = dom_Element.text();
                }
            }

            if (strTag == "banner") {
                QVariantMap banner;
                banner["id"] = dom_Element.attribute("id").toInt();
                banner["delay"] = dom_Element.attribute("delay").toInt();
                banner["go_to"] = dom_Element.attribute("go_to").toInt();
                banner["action"] = dom_Element.attribute("action");

                banners.append(banner);
            }
        }
        parcerHash(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void GetServices::parcerNote(const QDomNode &dom_Element) {
    QDomNode dom_Node = dom_Element.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {

            QDomElement dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            // if(Debugger) qDebug() << strTag + " " + dom_Element.text();

            // проверям респонс
            if (strTag == "resultCode") {
                QString sts = dom_Element.text();

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
                count_providers = dom_Node.childNodes().count();
            }

            // Данные о дилере
            if (strTag == "balance") {
                infoMap["balance"] = dom_Element.text();
                toLogData += QString("- Баланс агента              - %1\n")
                                 .arg(infoMap["balance"].toString());
            }

            if (strTag == "overdraft") {
                infoMap["overdraft"] = dom_Element.text();
                toLogData += QString("- Овердрафт агента           - %1  \n")
                                 .arg(infoMap["overdraft"].toString());
            }

            if (strTag == "id_agent") {
                infoMap["id_agent"] = dom_Element.text();
                toLogData += QString("- ID агента                  - %1  \n")
                                 .arg(infoMap["id_agent"].toString());
            }

            if (strTag == "name") {
                infoMap["name_agent"] = dom_Element.text().replace("\'", "").replace("\"", "");
                toLogData += QString("- Наименование агента        - %1  \n")
                                 .arg(infoMap["name_agent"].toString());
            }

            if (strTag == "inn") {
                infoMap["inn_agent"] = dom_Element.text();
                toLogData += QString("- ИНН агента                 - %1  \n")
                                 .arg(infoMap["inn_agent"].toString());
            }

            if (strTag == "phone") {
                infoMap["phone_agent"] = dom_Element.text().replace("\'", "").replace("\"", "");
                toLogData += QString("- Служба поддержки           - %1 \n")
                                 .arg(infoMap["phone_agent"].toString());
            }

            if (strTag == "threshold") {
                infoMap["threshold"] = dom_Element.text();
                toLogData += QString("- Порог отключения           - %1  \n")
                                 .arg(infoMap["threshold"].toString());
            }

            if (strTag == "hash") {
                infoMap["hash"] = dom_Element.text();
                toLogData +=
                    QString("- Идентификатор конфигурации - %1 \n").arg(infoMap["hash"].toString());
            }

            if (strTag == "hash_conf") {
                infoMap["hashxml"] = dom_Element.text();
                infoMap["path_name"] = dom_Element.attribute("p", "");
                toLogData += QString("- Идентификатор интерфейса   - %1 \n")
                                 .arg(infoMap["hashxml"].toString());
                toLogData += QString("- Путь для закачки           - %1  \n")
                                 .arg(infoMap["path_name"].toString());
            }

            if (strTag == "currency_usd") {
                infoMap["usd_curce"] = dom_Element.text();
                toLogData += QString("- Курс доллара               - %1  \n")
                                 .arg(infoMap["usd_curce"].toString());
            }

            if (strTag == "active") {
                infoMap["active"] = dom_Element.text();
                toLogData += QString("- Активность терминала       - %1  \n")
                                 .arg(infoMap["active"].toString());
            }

            if (strTag == "address") {
                infoMap["address"] = dom_Element.text().replace("\'", "").replace("\"", "");
                toLogData += QString("- Адрес                      - %1  \n")
                                 .arg(infoMap["address"].toString());
            }

            if (strTag == "cur") {
                int id = dom_Element.attribute("id", "").toInt();
                this->curMap[id] = dom_Element.text();
            }

            // Данные провайдера
            if (strTag == "prv") {
                providerList[index_prv]["id"] = dom_Element.attribute("id", "");
                providerList[index_prv]["nbl"] = dom_Element.attribute("nbl", "");
                providerList[index_prv]["cid"] = dom_Element.attribute("cid", "");
                providerList[index_prv]["cms"] = dom_Element.attribute("cms", "");
                providerList[index_prv]["otp"] = dom_Element.attribute("otp", "");
                providerList[index_prv]["category_id"] = dom_Element.attribute("cat", "");
                providerList[index_prv]["amount_min"] = dom_Element.attribute("min", "");
                providerList[index_prv]["amount_max"] = dom_Element.attribute("max", "");
                providerList[index_prv]["name_local"] =
                    dom_Element.attribute("nloc", "").trimmed().replace("'", "''");
                providerList[index_prv]["name_secondary"] =
                    dom_Element.attribute("nsc", "").trimmed().replace("'", "''");
                providerList[index_prv]["name_ru"] = dom_Element.attribute("nru", "").trimmed();
                providerList[index_prv]["name_en"] = dom_Element.attribute("nen", "").trimmed();
                providerList[index_prv]["precheck"] = dom_Element.attribute("pchk", "");
                providerList[index_prv]["autosum"] = dom_Element.attribute("asum", "");
                providerList[index_prv]["presum"] = dom_Element.attribute("psum", "");
                providerList[index_prv]["sms_code"] = dom_Element.attribute("sms", "");
                providerList[index_prv]["cms_add"] = dom_Element.attribute("cadd", "");
                providerList[index_prv]["que"] = dom_Element.attribute("que", "");
                providerList[index_prv]["cms_warn"] = dom_Element.attribute("cwarn", "");
                providerList[index_prv]["return_max"] = dom_Element.attribute("hmax", "");
                providerList[index_prv]["regions"] = dom_Element.attribute("rgns", "");
                providerList[index_prv]["favorite"] = dom_Element.attribute("faque", "");

                index_prv++;
            }

            if (strTag == "categories") {
                QVariantMap category;
                category["id"] = dom_Element.attribute("id").toInt();
                category["name_local"] = dom_Element.attribute("name_loc").replace("'", "''");
                category["name_secondary"] =
                    dom_Element.attribute("name_secondary").replace("'", "''");
                category["name_ru"] = dom_Element.attribute("name_ru");
                category["name_en"] = dom_Element.attribute("name_en");
                category["description_local"] =
                    dom_Element.attribute("description_loc").replace("'", "''");
                category["description_secondary"] =
                    dom_Element.attribute("description_secondary").replace("'", "''");
                category["description_ru"] = dom_Element.attribute("description_ru");
                category["description_en"] = dom_Element.attribute("description_en");
                category["queue"] = dom_Element.attribute("queue").toInt();

                if (category.value("queue").toInt() >= 0) {
                    saveCategoriesDB(category);
                }
            }

            if (strTag == "inputs") {
                QVariantMap input;
                input["id"] = dom_Element.attribute("id").toInt();
                input["input_panel"] = dom_Element.attribute("ipan").toInt();
                input["regexp"] = dom_Element.attribute("reg");
                input["mask"] = dom_Element.attribute("mask");
                input["help_local"] = dom_Element.attribute("hloc").replace("'", "''");
                input["help_secondary"] = dom_Element.attribute("hlsl").replace("'", "''");
                input["help_ru"] = dom_Element.attribute("hru");
                input["help_en"] = dom_Element.attribute("hen");
                input["placeholder_local"] = dom_Element.attribute("ploc").replace("'", "''");
                input["placeholder_secondary"] = dom_Element.attribute("plsl").replace("'", "''");
                input["placeholder_ru"] = dom_Element.attribute("pru");
                input["placeholder_en"] = dom_Element.attribute("pen");

                saveInputsDB(input);
            }

            if (strTag == "si") {
                QVariantMap si;
                si["input_id"] = dom_Element.attribute("input_id");
                si["service_id"] = dom_Element.attribute("service_id");
                si["field"] = dom_Element.attribute("field");
                si["field_type"] = dom_Element.attribute("field_type");
                si["prefix"] = dom_Element.attribute("prefix");
                si["que"] = dom_Element.attribute("queue");

                servicesInputs.append(si);
            }

            // Данные о комиссии
            if (strTag == "commission") {
                auto serviceId = dom_Element.parentNode().toElement().attribute("id", "");
                auto cid = dom_Element.parentNode().toElement().attribute("cid", "");

                if (serviceId != "") {
                    auto index = dom_Element.attribute("index", "").toInt();
                    auto sum_From = dom_Element.attribute("start", "").toDouble();
                    auto sum_To = dom_Element.attribute("end", "").toDouble();
                    auto value = dom_Element.attribute("value", "").toDouble();
                    auto typeId = dom_Element.attribute("type_id", "").toInt();

                    saveCommissionDB(
                        cid.toInt(), sum_From, sum_To, typeId, value, serviceId.toInt(), index);
                }
            }

            if (strTag == "server_time") {
                infoMap["server_time"] = dom_Element.text();
            }

            if (strTag == "coin_acceptor") {
                if (dom_Element.parentNode().toElement().tagName() == "devices") {
                    infoMap["coin_acceptor"] = dom_Element.text();
                }
            }

            if (strTag == "banner") {
                QVariantMap banner;
                banner["id"] = dom_Element.attribute("id").toInt();
                banner["delay"] = dom_Element.attribute("delay").toInt();
                banner["go_to"] = dom_Element.attribute("go_to").toInt();
                banner["action"] = dom_Element.attribute("action");

                banners.append(banner);
            }
        }
        parcerNote(dom_Node);
        dom_Node = dom_Node.nextSibling();
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
                                   const double sum_From,
                                   const double sum_To,
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
                        .arg(sum_From)
                        .arg(sum_To)
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
    }

    QSqlRecord record = querySql.record();

    if (querySql.next()) {
        int id = querySql.value(record.indexOf("count")).toInt();
        return id;
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
