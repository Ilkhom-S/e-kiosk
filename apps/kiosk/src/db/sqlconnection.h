#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include <QFile>
#include <QSqlDatabase>
#include <QStringList>
#include <QtSql>

inline bool createMainDB(QSqlDatabase &dbName) {
    QStringList queryList;
    queryList
        << "CREATE TABLE terminal_collect (collect_id TEXT, stack_id NUMERIC, denom TEXT, "
           "date_create TEXT, status TEXT, collect_id_next TEXT);"
        << "CREATE TABLE terminal_commission (commission_id NUMERIC, commission_sum_from NUMERIC, "
           "commission_sum_to NUMERIC, commission_i_type NUMERIC, commission_value NUMERIC, "
           "service_id NUMERIC, idx NUMERIC);"
        << "CREATE TABLE terminal_devices (comment TEXT, id NUMERIC, name TEXT, port TEXT, state "
           "NUMERIC);"
        << "INSERT INTO terminal_devices VALUES('',1,'null','null',0);"
        << "INSERT INTO terminal_devices VALUES('',2,'null','null',0);"
        << "INSERT INTO terminal_devices VALUES('',3,'AT-Modem',2,0);"
        << "INSERT INTO terminal_devices VALUES('',4,'null','null',0);"
        << "INSERT INTO terminal_devices VALUES('',5,'null','null',0);"
        << "CREATE TABLE terminal_md5(md5_id NUMERIC, md5_hash TEXT, md5_version TEXT, md5_date "
           "TEXT);"
        << "INSERT INTO terminal_md5 VALUES(1,'','','');"
        << "CREATE TABLE terminal_moneyout (moneyout_id INTEGER PRIMARY KEY, moneyout_collect_id "
           "TEXT, moneyout_collect_status TEXT, moneyout_date_create TEXT, moneyout_value TEXT, "
           "moneyout_comment TEXT, moneyout_is_send NUMERIC);"
        << "CREATE TABLE terminal_operation (operation_services_name TEXT, operation_ratio_number "
           "NUMERIC, operation_ratio_persent NUMERIC, operation_account_design TEXT, "
           "operation_account TEXT, operation_collect_id TEXT, operation_date_create TEXT, "
           "operation_date_confirm TEXT, operation_id NUMERIC, operation_is_send NUMERIC, "
           "operation_money_amount NUMERIC, operation_money_cash NUMERIC, operation_money_denom "
           "TEXT, operation_services_id NUMERIC, operation_status NUMERIC, operation_print_status "
           "NUMERIC, is_coin NUMERIC, notify_route TEXT, extra_info TEXT);"
        << "CREATE TABLE terminal_services (services_id NUMERIC, services_cid NUMERIC, "
           "services_nbl NUMERIC, services_cms NUMERIC, services_otp NUMERIC, category_id NUMERIC, "
           "amount_min NUMERIC, amount_max NUMERIC, name_local TEXT, name_secondary TEXT, name_ru "
           "TEXT, name_en TEXT, precheck NUMERIC, autosum NUMERIC, presum NUMERIC, sms_code "
           "NUMERIC, cms_add NUMERIC, regions NUMERIC, favorite NUMERIC, que NUMERIC, cms_warn "
           "NUMERIC, return_max NUMERIC);"
        << "CREATE TABLE terminal_data (id NUMERIC, login TEXT, token TEXT, secret_login TEXT, "
           "secret_pass TEXT);"
        << "CREATE TABLE terminal_commands (trn TEXT, cmd NUMERIC, account TEXT, comment TEXT, "
           "status TEXT);"
        << "CREATE TABLE terminal_extra (id NUMERIC, id_agent TEXT, name TEXT, inn TEXT, phone "
           "TEXT, threshold TEXT, hash TEXT, hash_conf TEXT, currency_usd TEXT, active NUMERIC, "
           "address TEXT, path TEXT, server_time TEXT);"
        << "CREATE TABLE terminal_categories (id NUMERIC, name_local TEXT, name_secondary TEXT, "
           "name_ru TEXT, name_en TEXT, description_local TEXT, description_secondary TEXT, "
           "description_ru TEXT, description_en TEXT, que NUMERIC);"
        << "CREATE TABLE terminal_inputs (id NUMERIC, input_panel NUMERIC, regexp TEXT, mask TEXT, "
           "help_local TEXT, help_secondary TEXT, help_ru TEXT, help_en TEXT, placeholder_local "
           "TEXT, placeholder_secondary TEXT, placeholder_ru TEXT, placeholder_en TEXT);"
        << "CREATE TABLE terminal_services_inputs (input_id NUMERIC, service_id NUMERIC, field "
           "TEXT, field_type TEXT, prefix TEXT, que NUMERIC);"
        << "CREATE TABLE terminal_bvalidator (event TEXT, date_time TEXT, status TEXT);";

    QSqlQuery updateSql(dbName);

    for (auto &query : queryList) {
        if (!updateSql.exec(query)) {
            qDebug() << updateSql.lastError();
            qDebug() << "Error Update SQL DATABASE IN LINE <<" + query + ">>";
            return false;
        }
    }

    return true;
}

inline bool createUpdaterDB(QSqlDatabase &dbName) {
    Q_UNUSED(dbName);

    return true;
}

inline bool createConnection(QSqlDatabase &db, QString db_path) {
    QFile info;

    bool fileExist = info.exists(db_path);

    info.setFileName(db_path);

    QString flName = info.fileName();
    db = QSqlDatabase::addDatabase("QSQLITE", flName);

    db.setDatabaseName(db_path);

    if (!db.open()) {
        return false;
    }

    // Если файла базы данных не было то создаем каркас
    if (!fileExist) {
        return createMainDB(db);
    }

    QSqlQuery addColumnQuery(db);
    QString strQuery = QString("ALTER TABLE terminal_services ADD services_otp NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD category_id NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD amount_min NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD amount_max NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD name_local TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD name_ru TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD name_en TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD precheck NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD autosum NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD presum NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD sms_code NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD cms_add NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD que NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD cms_warn NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD return_max NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD regions NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD favorite NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_services ADD name_secondary TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_commission ADD service_id NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_commission ADD idx NUMERIC");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_operation ADD extra_info TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString(
        "CREATE TABLE IF NOT EXISTS terminal_inputs (id NUMERIC, input_panel NUMERIC, regexp TEXT, "
        "mask TEXT, help_local TEXT, help_ru TEXT, help_en TEXT, placeholder_local TEXT, "
        "placeholder_ru TEXT, placeholder_en TEXT);");
    addColumnQuery.exec(strQuery);

    strQuery = QString(
        "CREATE TABLE IF NOT EXISTS terminal_categories (id NUMERIC, name_local TEXT, name_ru "
        "TEXT, name_en TEXT, description_local TEXT, description_ru TEXT, description_en TEXT, que "
        "NUMERIC);");
    addColumnQuery.exec(strQuery);

    strQuery = QString(
        "CREATE TABLE terminal_services_inputs (input_id NUMERIC, service_id NUMERIC, field TEXT, "
        "field_type TEXT, prefix TEXT, que NUMERIC);");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_categories ADD name_secondary TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_categories ADD description_secondary TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_inputs ADD help_secondary TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString("ALTER TABLE terminal_inputs ADD placeholder_secondary TEXT");
    addColumnQuery.exec(strQuery);

    strQuery = QString(
        "CREATE TABLE IF NOT EXISTS terminal_bvalidator (event TEXT, date_time TEXT, status "
        "TEXT);");
    addColumnQuery.exec(strQuery);

    return true;
}

inline bool createConnectionFile(QSqlDatabase &db, QString db_path) {
    QFile info;

    bool fileExist = info.exists(db_path);

    info.setFileName(db_path);

    QString flName = info.fileName();
    db = QSqlDatabase::addDatabase("QSQLITE", flName);

    db.setDatabaseName(db_path);

    if (!db.open()) {
        return false;
    }

    // Если файла базы данных не было то создаем каркас
    if (!fileExist) {
        return createUpdaterDB(db);
    }

    return true;
}

#endif  // SQLCONNECTION_H
