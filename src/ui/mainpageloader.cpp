#include "mainpageloader.h"
#include "ui_mainpageloader.h"
#include <QTextStream>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QTextDocument>


MainPageLoader::MainPageLoader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPageLoader)
{
    ui->setupUi(this);

    _lang = Lang::RU;

    payTimer = new QTimer(this);
    payTimer->setSingleShot(true);
    connect(payTimer,SIGNAL(timeout()),this,SLOT(paymentConfirm()));

    goInputSum = new QTimer(this);
    goInputSum->setSingleShot(true);
    connect(goInputSum,SIGNAL(timeout()),this,SLOT(btnGotoInputSumClc()));

    webView = new QWebView(this);
    webView->setAttribute(Qt::WA_NativeWindow, true);
    webView->setAttribute(Qt::WA_DeleteOnClose);
    webView->setContextMenuPolicy(Qt::NoContextMenu);

    connect(webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(populateJavaScriptWindowObject()));

    ui->webViewLayout->addWidget(webView);
}

void MainPageLoader::populateJavaScriptWindowObject()
{
    webView->page()->mainFrame()->addToJavaScriptWindowObject("ctl", this);
}

void MainPageLoader::setDbName(QSqlDatabase &dbName)
{
    db = dbName;
}

void MainPageLoader::setTemplate(QString tpl)
{
    this->tpl = tpl;

    mainWebPage.setFile(QString("assets/front/index.html"));
    lockWebPage.setFile(QString("assets/front/lock.html"));
}

void MainPageLoader::inspectEnable()
{
    webView->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    webView->setContextMenuPolicy(Qt::DefaultContextMenu);
}

void MainPageLoader::playSound(QString fileName)
{
    Q_UNUSED(fileName)

//    QString file = ConstData::Path::Sound + lang + "/" + fileName;

//    if (QFile::exists(file)) {
//        QSound::play(file);
//    }
}

void MainPageLoader::playNumpadSound(QString fileName){
    Q_UNUSED(fileName)
//    QString file = "assets/sound/keys/"+ fileName + ".wav";

//    if (QFile::exists(file)) {
//        QSound::play(file);
//    }
}

void MainPageLoader::userInfoCheck(QString account, QString idPrv)
{
    emit emit_getUserInfo(account, idPrv);
}

void MainPageLoader::gotoPageInputFromPayment(const int serviceId)
{
    //Отключаем купюрник
    emit validator_activate(false);

    //playSound(Sound::sInputNumber);

    if (serviceId > 0) {
        _serviceCurrent = serviceInfo(serviceId);
    }

    gotoPage(PageIn::InputNumber);
}

void MainPageLoader::paymentConfirm()
{
    //Отключение купюроприемника
    emit validator_activate(false);

    //Обновляем платёж и даем ему статус confirm
    emit emit_confirm_pay(gblNowTranzaction,false);

    gotoPage(PageIn::PrintDialog);
}

void MainPageLoader::payToWhenBoxOpen()
{
    //Нажата кнопка оплатить
    btnPayClck = true;

    //Обновляем платёж и даем ему статус confirm
    emit emit_confirm_pay(gblNowTranzaction,false);

    //Обновляем платёж и ставим статус напечатан чек
    emit emit_confirm_pay(gblNowTranzaction,true);

    //Отправляем сигнал на печать
    emit emit_print_pay(gblNowTranzaction);

    btnPayClck = false;

    //Переходим на главную страницу
    gotoPage(PageIn::Main);
}

PageIn::page MainPageLoader::getStepByStepPage()
{
    return currentPage;
}

QVariantMap MainPageLoader::serviceMaxSum()
{
    QVariantMap data;

    if (!_serviceCurrent.isEmpty()){
        data.insert("max_sum_reject", _serviceCurrent.value("return_max").toBool());
        data.insert("max_sum", _serviceCurrent.value("amount_max").toInt());
    }

    return data;
}

bool MainPageLoader::moneyExistInPay()
{
    if(nominalCash > 0)
        return true;
    else
        return false;
}

void MainPageLoader::showHideReturnNominal(bool status)
{
    int amountMax = _serviceCurrent.value("amount_max").toInt();

    //Показываем значения клиенту
    QString jsFunction = status ? QString("showReturnNominal(%1)").arg(amountMax) : "hideReturnNominal()";

    webView->page()->mainFrame()->evaluateJavaScript(jsFunction);
}

void MainPageLoader::setPageStatus(PageIn::page page)
{
    auto updaterLock = false;

    if (page == PageIn::InputNumber || page == PageIn::InputSum || page == PageIn::PrintDialog || page == PageIn::AfterPrint) {
        updaterLock = true;
    }

    emit emit_updaterLock(updaterLock);
}

void MainPageLoader::precheck(QString idPrv, QString account, double amount)
{
    // Сделаем небольшую проверку
    if (idPrv == "" || account == "") {
        return;
    }

    gblTrnOnlineCheck = QString("%1%2").arg(QDateTime::currentDateTime().toString("yyMMddHHmmsszzz").left(14), terminalInfo["terminal_num"].toString().right(4));

    if (tpl != "tjk") {
        for (auto &key : fieldsData.keys()) {
            paramPrecheck[key] = fieldsData[key];
        }
    }

    QString param = QJsonDocument::fromVariant(paramPrecheck).toJson(QJsonDocument::Compact);
    emit emit_toLoging(0, "MAIN", QString("Переходим на страницу пречека account- %1, param- %2").arg(account, param));

    emit emit_checkOnline(gblTrnOnlineCheck, idPrv, account, amount, paramPrecheck);
}

void MainPageLoader::orzuUserDefine(QString inn)
{
    emit emit_toLoging(0, "ORZU", QString("Введен ИНН: %1").arg(inn));

    emit emit_sendJsonRequest(QJsonObject(), QString("orzu/user/%1").arg(inn), "orzu_user_define", 0, 20);
}

void MainPageLoader::orzuOtpResend()
{
    auto phone = orzuInfo["phone_number"].toString();
    emit emit_toLoging(0, "ORZU", QString("Отправка отп на номер: %1").arg(phone));

    auto applicationId = QDateTime::currentDateTime().toString("yyMMddHHmmsszzz");

    QJsonObject json;
    json["id"]      = orzuInfo["inn"].toString();
    json["account"] = "+992" + phone;

    emit emit_sendJsonRequest(json, "orzu/otp/send", "orzu_otp_send", 1, 20);
}

void MainPageLoader::orzuOtpConfirm(QString inn, QString otp)
{
    QJsonObject json;
    json["id"]    = inn;
    json["value"] = otp;

    orzuInfo["inn"] = inn;

    emit emit_toLoging(0, "ORZU", QString("Подтверждение введенного otp: %1").arg(otp));

    emit emit_sendJsonRequest(json, "orzu/otp/confirm-otp", "orzu_otp_confirm", 1, 20);
}

void MainPageLoader::orzuConditionsGet(int amount)
{
    auto orzuId = orzuInfo.value("orzu_id").toString();

    auto url = QString("orzu/business/conditions?orzu_id=%1&service_id=%2&summa=%3").arg(orzuId, "118").arg(amount);

    QVariantMap header;
    header["token"] = orzuInfo["token"].toString();

    emit emit_sendJsonRequest(QJsonObject(), url, "orzu_business_conditions", 0, 20, header);

    emit emit_toLoging(0, "ORZU", QString("Отправляется запрос на получения условий кредита. Введенная сумма: %1").arg(amount));
}

bool MainPageLoader::orzuConditionIsValid(int amount, int term)
{
    for (auto c : orzuInfo["conditions"].toList()) {
        auto condition = c.toMap();
        auto minSumma = condition.value("min_summa").toInt();
        auto maxSumma = condition.value("max_summa").toInt();
        auto term_  = condition.value("term").toInt();

        if (amount >= minSumma && amount <= maxSumma && term == term_) {
            orzuInfo["current_condition"] = condition;
            orzuInfo["pSum"] = amount;
            return true;
        }
    }

    return false;
}

void MainPageLoader::panCheck(QString pan)
{
    orzuInfo["pan"] = pan;

    auto orzuId = orzuInfo.value("orzu_id").toString();

    auto url = QString("orzu/user/check-pan?orzu_id=%1&pan=%2").arg(orzuId, pan);

    QVariantMap header;
    header["token"] = orzuInfo["token"].toString();

    emit emit_toLoging(0, "ORZU", QString("Введен номер карты: %1").arg(pan));

    emit emit_sendJsonRequest(QJsonObject(), url, "check_pan", 0, 30, header);
}

void MainPageLoader::orzuTranshCreate()
{
    auto condition = orzuInfo.value("current_condition").toMap();

    QJsonObject json;
    json["pInn"]  = orzuInfo["inn"].toString();
    json["pOrzuId"] = orzuInfo.value("orzu_id").toInt();
    json["pSum"] = orzuInfo.value("pSum").toInt();
    json["pCredConditions"] = condition.value("condition_id").toLongLong();
    json["pan"] = orzuInfo.value("pan").toString();
    json["terminal_id"] = terminalInfo["terminal_num"].toInt();
    json["pRecipient"] = QDateTime::currentDateTime().toString("yyMMddHHmmsszzz");

    orzuInfo["transh_trn"] = json["pRecipient"].toString();

    QVariantMap header;
    header["token"] = orzuInfo["token"].toString();

    emit emit_sendJsonRequest(json, "orzu/business/transh", "orzu_business_transh", 1, 30, header);

}

void MainPageLoader::jsonResponseSuccess(QVariantMap response, QString requestName)
{
    QString jsFunc;

    if (requestName == "orzu_user_define") {
        orzuInfo.clear();
        orzuInfo = response.value("payload").toMap();

        emit emit_toLoging(0, "ORZU", QString("Определен клиент: %1").arg(orzuInfo.value("client_name").toString()));

        jsFunc = QString("userDefineSuccess()");
    }

    if (requestName == "orzu_otp_send") {
        jsFunc = QString("otpResendSuccess()");
    }

    if (requestName == "orzu_otp_confirm") {
        orzuInfo["token"] = response.value("token").toString();

        emit emit_toLoging(0, "ORZU", "Otp успешно подтвержден");

        jsFunc = QString("otpConfirmSuccess()");
    }

    if (requestName == "orzu_business_conditions") {

        QVariantList conditions;

        for (auto &condition : response.value("data").toList()) {
            if (condition.toMap().value("interval_units").toString() == "M") {
                conditions.append(condition);
            }
        }

        orzuInfo["conditions"] = conditions;

        int min = 0;
        int max = 0;

        QStringList terms;

        if (conditions.length() > 0) {
            min = conditions.at(0).toMap().value("min_summa").toInt();
            max = conditions.at(0).toMap().value("max_summa").toInt();

            for (auto &condition : conditions) {
                auto c = condition.toMap();
                auto minSumma = c.value("min_summa").toInt();
                auto maxSumma = c.value("max_summa").toInt();

                if (minSumma < min) {
                    min = minSumma;
                }

                if (maxSumma > max) {
                    max = maxSumma;
                }

                terms << c.value("term").toString();
            }
        }

        orzuInfo["credit_amount_min"] = min;
        orzuInfo["credit_amount_max"] = max;

        emit emit_toLoging(0, "ORZU", QString("Получены условия кредита - мин. сумма: %1, макс. сумма: %2, срок кредита: %3").arg(min).arg(max).arg(terms.join(",")));

        jsFunc = QString("conditionsSuccess()");
    }

    if (requestName == "check_pan") {
        emit emit_toLoging(0, "ORZU", "Карта успешно подтвеждена, отправляется запрос на получение кредита");

        orzuTranshCreate();
        return;
    }

    if  (requestName == "orzu_business_transh") {
        emit emit_toLoging(0, "ORZU", "Кредит успешно оформлен");

        jsFunc = QString("transhSuccess()");
    }

    webView->page()->mainFrame()->evaluateJavaScript(jsFunc);
}

QVariantMap MainPageLoader::orzuData() {
    return orzuInfo;
}

void MainPageLoader::jsonResponseError(QString error, QString requestName)
{
    Q_UNUSED(requestName)

    error = error.replace("\"", "&quot;");

    if (error == "timeout") {
        error = "Отсутствует соединение с сервером";
    }

    QString jsFunc = QString("resultError(\"%1\")").arg(error);

    webView->page()->mainFrame()->evaluateJavaScript(jsFunc);

    emit emit_toLoging(2, "ORZU", error);
}

void MainPageLoader::notifyRouteSet(QString value)
{
    fieldsData["notify_route"] = value;
    fieldsData["notify_route_mask"] = value;

    extraInfo["notify_route"] = value;
    extraInfo["notify_route_mask"] = value;
}

void MainPageLoader::receiptSend(QString phone)
{
    if (phone.trimmed() == "") {
        return;
    }

    emit emit_sendReceipt(gblNowTranzaction, phone);
}

void MainPageLoader::otpSend(QString account)
{
    if (account.trimmed() == "") {
        return;
    }

    emit emit_toLoging(0, "OTP", QString("Отправка otp на номер: %1").arg(account));

    gblOtpId = "";

    emit emit_otpSend(account);
}

void MainPageLoader::otpConfirm(QString otp)
{
    if (otp.trimmed() == "") {
        return;
    }

    emit emit_toLoging(0, "OTP", QString("Подтверждение введенного otp: %1").arg(otp));

    emit emit_otpConfirm(gblOtpId, otp);
}

void MainPageLoader::loadHtmlPage(PageIn::page page){

    QString currentWebPage = "";

    switch(page){
        case PageIn::Main:
            currentWebPage = QString("file:///%1?lng=%2").arg(mainWebPage.absoluteFilePath(), _langDefault);
        break;
        case PageIn::LockTerminal:
            currentWebPage = QString("file:///%1?lng=%2").arg(lockWebPage.absoluteFilePath(), _langDefault);
        break;
        default:
        break;
    }

    gotoPage(page);

    webView->load(currentWebPage);
}

void MainPageLoader::gotoPageMain(bool disableValidator){
    if (disableValidator) {
        emit validator_activate(false);
    }

    gotoPage(PageIn::Main);
}

void MainPageLoader::loadMainPage(){

    emit validator_activate(false);

    //Обновляем платёж и даем ему статус confirm
    emit emit_confirm_pay(gblNowTranzaction,false);

    loadHtmlPage(PageIn::Main);
}

void MainPageLoader::gotoPageInputAccount(int serviceId){
    _serviceCurrent = serviceInfo(serviceId);
    paramPrecheck.clear();
    fieldsData.clear();
    _precheckItems.clear();

    gotoPage(PageIn::InputNumber);
}

void MainPageLoader::gotoPageInfo(){
    gotoPage(PageIn::Information);
}

void MainPageLoader::gotoPageCategories() {
    gotoPage(PageIn::SelectGroup);
}

void MainPageLoader::gotoPageServices(int categoryId) {
    for (auto &c: categories) {
        auto category = c.toMap();
        if (category.value("id").toInt() == categoryId) {
            _categoryCurrent = category;
            break;
        }
    }

    gotoPage(PageIn::SelectServices);
}

void MainPageLoader::gotoPageInsertNominal(QString account, QString accountDesign, int serviceId){
    originalNumber  = account;
    afterMaskNumber = accountDesign;
    _serviceCurrent = serviceInfo(serviceId);

    //Очищаем поля
    clearNominalData();

    for (auto &key : fieldsData.keys()) {
        if (fieldsData[key].type() == QVariant::String) {
            extraInfo[key] = normalizeField(fieldsData[key].toString());
        } else {
            extraInfo[key] = fieldsData[key];
        }
    }

    //Переходим на страницу
    gotoPage(PageIn::InputSum);

    //Активируем купюроприемник
    goInputSum->start(500);
}

QVariantMap MainPageLoader::extraInfoGet()
{
    QVariantMap extra;

    for (auto &input: _serviceCurrent.value("inputs").toList()) {
        auto i = input.toMap();
        auto field = i.value("field").toString();

        if (extraInfo.keys().contains(field)) {
            extra[field] = extraInfo[field];
        }
    }

    // Invisible fields
    for (auto &input: _serviceCurrent.value("fields").toList()) {
        auto i = input.toMap();
        auto field = i.value("field").toString();

        if (extraInfo.keys().contains(field)) {
            extra[field] = extraInfo[field];
        }
    }

    return extra;
}

QString MainPageLoader::normalizeField(QString value)
{
    auto v = value.replace("<br>", "")
        .replace("<", " ")
        .replace(">", " ")
        .replace("?", " ")
        .replace("/", " ")
        .replace("%", " ")
        .replace("^", " ")
        .replace("!", " ")
        .replace("|", " ")
        .replace("'", " ")
        .replace("`", " ");

    return v.trimmed();
}

void MainPageLoader::gotoPage(PageIn::page page)
{
    //Ставим софт занят или нет для апдейтера
    setPageStatus(page);

    currentPage = page;

    QString fileNmae = "";

    switch (page) {
        //Переход на главную страницу
        case PageIn::Main : {
            extraInfo.clear();
            paramPrecheck.clear();
            fieldsData.clear();
            _precheckItems.clear();
            _lang = _langDefault;

            emit emit_checkStatus54();
            emit emit_toLoging(0,"MAIN", "Переходим на главную страницу");
        }
        break;
        case PageIn::SelectGroup : {
            fileNmae = Sound::sGetCategory;

            emit emit_toLoging(0,"MAIN", "Переходим на страницу группа операторов");
        }
        break;
        case PageIn::SelectServices : {
            fileNmae = Sound::sGetOperator;

            emit emit_toLoging(0,"MAIN", QString("Переходим на страницу выбора оператора в группе %1").arg(_categoryCurrent.value("id").toInt()));
        }
        break;

        case PageIn::InputNumber : {
            extraInfo.clear();

            gblTrnOnlineCheck = "";

            //Надо отправить сигнал чтобы проверить статус Validatora
            emit emit_sendStatusValidator();

            if (printerStatus) {
                //ui->lblErrorPrinterInput->setVisible(false);
            } else {
                //ui->lblErrorPrinterInput->setVisible(true);
            }

            fileNmae = Sound::sInputNumber;

            emit emit_toLoging(0,"MAIN", QString("Переходим на страницу ввода номера выбрав оператор %1").arg(_serviceCurrent.value("name_ru").toString()));

        }
        break;

        case PageIn::InputSum : {
            emit emit_toLoging(0,"MAIN",QString("Номер абонента %1 на провайдер %2 подтвержден, переходим на страницу ввода денег.").arg(originalNumber, _serviceCurrent.value("name_ru").toString()));

            fileNmae = Sound::sInputMoney;

            //Берем коммисии
            getCommissionMap();

            btnPayClck = false;
        }
        break;
        case PageIn::PrintDialog :
            emit emit_toLoging(0,"MAIN","Переходим на страницу печати чека...");
        break;
        case PageIn::AfterPrint:
        break;
        case PageIn::LockTerminal:
        break;
        case PageIn::Information :
            emit emit_toLoging(0,"MAIN", "Переходим на страницу Информация");
        break;
        default :
        break;

    }

    //Проигрываем соунд
    if (fileNmae != "") {
        playSound(fileNmae);
    }
}

void MainPageLoader::setTerminalInfo(QString data)
{
    QString jsFunc = QString("balanceInfo(\"%1\")").arg(data);
    webView->page()->mainFrame()->evaluateJavaScript( jsFunc );
}

void MainPageLoader::setCheckOnlineResult(QString resultCode, QString status, QString message, QVariantList items)
{
    Q_UNUSED(status)

    QString jsFunc;

    _precheckItems = items;

    QTextDocument doc;
    message.replace("\n", "<br>");
    doc.setHtml(message);
    message = doc.toPlainText().replace("\"", "&quot;").replace("\n", "<br>");

    //если resultCode == 1 OR no Response
    if (resultCode == "") {
        if (message.trimmed() == "") message = "Отсутствует соединение с сервером";
        jsFunc = QString("resultError(\"%1\")").arg(message);
    } else if (resultCode == "1") {
        jsFunc = QString("resultError(\"%1\")").arg(message);
    } else if (resultCode == "0") {
        jsFunc = QString("resultSuccess(\"%1\")").arg(message);
    }

    webView->page()->mainFrame()->evaluateJavaScript( jsFunc );
}

QVariantList MainPageLoader::precheckItems(){
    return _precheckItems;
}

void MainPageLoader::sendReceiptResult(QString resultCode, QString trn, QString status)
{
    Q_UNUSED(trn)
    Q_UNUSED(status)

    QString jsFunc;
    QString message;

    // Ошибка авторизации
    if (resultCode == "1") {
        message = QString("Ошибка при отправке эл.чека (код ошибки %1)").arg(resultCode);
        jsFunc = QString("resultError(\"%1\")").arg(message);
    // Успешно
    } else if(resultCode == "0") {
        message = "Электронный чек успешно отправлен";
        jsFunc = QString("resultSuccess(\"%1\")").arg(message);
    // Нет связи
    } else {
        message = "Отсутствует связь с сервером";
        jsFunc = QString("resultError(\"%1\")").arg(message);
    }

    webView->page()->mainFrame()->evaluateJavaScript( jsFunc );
}

void MainPageLoader::sendOtpResult(QString resultCode, QString otpId)
{
    QString jsFunc;
    QString message;

    // Ошибка авторизации
    if (resultCode == "1") {
        message = QString("Ошибка при отправке смс кода (код ошибки %1)").arg(resultCode);
//        jsInterface->otpSendError(message);
        jsFunc = QString("otpSendError(\"%1\")").arg(message);

        emit emit_toLoging(1, "OTP", message);
        // Успешно
    } else if(resultCode == "0") {
        gblOtpId = otpId;
        jsFunc = QString("otpSendSuccess()");

        emit emit_toLoging(0, "OTP", QString("Смс код успешно отправлен"));

        // Нет связи
    } else {
        message = "Отсутствует связь с сервером";
        jsFunc = QString("otpSendError(\"%1\")").arg(message);

        emit emit_toLoging(1, "OTP", message);
    }

    webView->page()->mainFrame()->evaluateJavaScript( jsFunc );
}

void MainPageLoader::confirmOtpResult(QString resultCode)
{
    QString jsFunc;
    QString message;

    // Ошибка авторизации
    if (resultCode == "1") {
        message = QString("Введен неверный код подтверждения");
        jsFunc = QString("otpConfirmError(\"%1\")").arg(message);

        emit emit_toLoging(1, "OTP", message);
        // Успешно
    } else if(resultCode == "0") {
        jsFunc = QString("otpConfirmSuccess()");

        emit emit_toLoging(0, "OTP", QString("Otp успешно подтвержден"));
        // Нет связи
    } else {
        message = "Отсутствует связь с сервером";
        jsFunc = QString("otpConfirmError(\"%1\")").arg(message);

        emit emit_toLoging(1, "OTP", message);
    }

    webView->page()->mainFrame()->evaluateJavaScript( jsFunc );
}

QString MainPageLoader::getPrinterStatus(){
    return printerStatus ? "1" : "0";
}

QString MainPageLoader::showPrintDialog(){
    return printDialogShow ? "1" : "0";
}

QString MainPageLoader::getLockReason(){
    return lockReason;
}

QString MainPageLoader::defaultLang(){
    return _langDefault;
}

QString MainPageLoader::lang(){
    return _lang;
}

QString MainPageLoader::theme(){
    return tpl;
}

QVariantMap MainPageLoader::homeData()
{
    QVariantMap data;

    // Logo
    data["logoUrl"] = "images/icons/" + tpl + "/logo.svg";

    // Banners
    QString pathBanner = "images/" + tpl + "/banners/";
    QDir directory(pathBanner);
    QStringList images = directory.entryList(QStringList() << "*.png" << "*.jpg" << "*.svg", QDir::Files);

    QVariantList bannersUrl;

    foreach(QString filename, images) {
        bannersUrl.append(pathBanner + filename);
    }

    data["bannersUrl"] = bannersUrl;

    data["banners"] = banners;

    data["terminalData"] = terminalInfo;
    return data;
}

QVariantMap MainPageLoader::terminalData()
{
    return terminalInfo;
}

QVariantList MainPageLoader::favoriteServices(){
    return favoriteList;
}

void MainPageLoader::favoriteServicesInit(){
    QVariantList favorites;

    for (auto &s: services) {
        auto service = s.toMap();
        if (service.value("favorite").toInt() > 0) {
            favorites.append(service);
        }
    }

    std::stable_sort(favorites.begin(), favorites.end(), [=](const QVariant &d1, const QVariant &d2) -> bool {
        return d1.toMap().value("favorite").toInt() < d2.toMap().value("favorite").toInt();
    });

    favoriteList = favorites;
}

QVariantList MainPageLoader::serviceList(const int categoryId){
    if (categoryId > 0) {
        QVariantList list;

        for (auto &s: services) {
            auto service = s.toMap();
            if (service.value("category_id").toInt() == categoryId) {
                list.append(service);
            }
        }

        return list;
    }

    return services;
}

QVariantList MainPageLoader::categoryList(){
    return categories;
}

QVariantMap MainPageLoader::serviceInfo(int id){
    for (auto &s: services) {
        auto service = s.toMap();
        if (service.value("id").toInt() == id) {
            return service;
        }
    }

    return QVariantMap();
}

QVariantMap MainPageLoader::serviceCurrent(){
    return _serviceCurrent;
}

QVariantMap MainPageLoader::categoryCurrent(){
    return _categoryCurrent;
}

QString MainPageLoader::paymentId()
{
    return gblNowTranzaction;
}

void MainPageLoader::setFieldData(QString key, QString val){
    fieldsData[key] = val;
};

QString MainPageLoader::getFieldData(QString key){
    return fieldsData.value(key).toString();
};

void MainPageLoader::receiptPrint(bool withSound)
{
    if (withSound) {
        playSound(Sound::sPrintChek);
    }

    if (printerStatus) {
        //Обновляем платёж и ставим статус напечатан чек
        emit emit_confirm_pay(gblNowTranzaction,true);

        //Отправляем сигнал на печать
        emit emit_print_pay(gblNowTranzaction);
    }
}

QString getSmallHtmlText(QString text)
{
    QString resultText = text;
    int lenText = text.length();
    if(lenText > 30){
        int indexIn = text.indexOf(" ",20);
        if(indexIn > 0)
            resultText = text.replace(indexIn,1,"<br>");
    }

    return resultText;
}

void MainPageLoader::inputNominal(int nominal, bool coin)
{
    if (getStepByStepPage() != PageIn::InputSum) {
        return;
    }

    double _nominal = nominal;

    if (coin) {
        double divider = 100;
        _nominal = nominal / divider;
    }

    //Увеличиваем количество поступивших денег
    nominalInserted ++;

    //если это первая купюра то отображаем существующую
    if (nominalInserted == 1) {
        //Генерируем ID транзакции платежа
        gblNowTranzaction = gblTrnOnlineCheck != "" ? gblTrnOnlineCheck : QString("%1%2").arg(QDateTime::currentDateTime().toString("yyMMddHHmmsszzz").left(14), terminalInfo["terminal_num"].toString().right(4));

        if (coin) {
            nominalDenomination = QString("%1").arg(nominal) + "M";
        } else {
            nominalDenomination = QString("%1").arg(_nominal);
        }

        nominalCash = _nominal;

    } else {
        nominalCash += _nominal;

        if (coin) {
            nominalDenomination += "," + QString("%1").arg(nominal) + "M";
        } else {
            nominalDenomination += "," + QString("%1").arg(_nominal);
        }
    }

    //Вычисляем комиссии
    getSumToFromMinusCommis(nominalCash);

    QVariantMap payment;
    payment["trn_id"] = gblNowTranzaction;
    payment["sum_from"] = nominalCash;
    payment["sum_to"] = nominalAmount;
    payment["denomination"] = nominalDenomination;
    payment["ratio_percent"] = gblRatioPrv;
    payment["ratio_sum"] = gblCmsSum;

    //Заносим данные в платежный демон
    if (nominalInserted == 1) {
        //Тут отправляем сигнал о создании нового платежа
        payment["account"] = originalNumber;
        payment["account_masked"] = afterMaskNumber;
        payment["prv_id"] = _serviceCurrent.value("id").toString();
        payment["prv_name"] = _serviceCurrent.value("name_ru").toString();
        payment["extra_info"] = extraInfoGet();
        payment["notify_route"] = extraInfo["notify_route"];

        emit emit_pay_new(payment);

    } else {
        //Тут отправляем сигнал об update-е платежа
        emit emit_update_pay(payment);
    }

    //Показываем значения клиенту
    QString jsFunction = QString("insertNominal(%1, %2, %3, %4, %5)").arg(nominalCash).arg(_nominal).arg(nominalAmount).arg(gblCmsSum).arg(gblRatioPrv);
    webView->page()->mainFrame()->evaluateJavaScript(jsFunction);
}

void MainPageLoader::getSumToFromMinusCommis(double amountFrom)
{
    int count_comis = commissionMap.count();

    if(count_comis > 1){
        //Число кимиссий больше одного
        for(int i = 1; i <= count_comis; i++){
            if(amountFrom >= commissionMap[i]["sum_from"].toDouble() && amountFrom <= commissionMap[i]["sum_to"].toDouble()){
                nominalAmount = getMoneyToFromAll(commissionMap[i]["type"].toInt(), amountFrom, commissionMap[i]["value"].toDouble());
                return;
            }
        }
    }else{
        //Имеется только одна запись с комиссией
        nominalAmount = getMoneyToFromAll(commissionMap[1]["type"].toInt(), amountFrom, commissionMap[1]["value"].toDouble());
        return;
    }

    nominalAmount = amountFrom;
    gblRatioPrv = 0;
    gblCmsSum = 0;

    return;
}
void MainPageLoader::clearNominalData()
{
    gblNowTranzaction = "";
    nominalInserted = 0;
    nominalAmount = 0;
    gblRatioPrv = 0;
    gblCmsSum = 0;
    nominalDenomination = "";
    nominalCash = 0;
}

double MainPageLoader::getMoneyToFromAll(int cmsType, double from, double value)
{
    double sum_to_in = 0;

    if (cmsType == 1) {
        //Статическая в сомони
        sum_to_in = from - value;
        gblRatioPrv = getRatioFromIn(from, sum_to_in);
    } else if (cmsType == 2) {
        double pp = (from * value)/100;
        sum_to_in = from - pp;
        gblRatioPrv = value;
    }

    gblCmsSum = from - sum_to_in;

    return sum_to_in;
}

double MainPageLoader::getRatioFromIn(double from, double to)
{
    double rat = 100 - ((to * 100) / from);
    rat = QString::number(rat, 'f', 2).toDouble();
    return rat;
}

QString MainPageLoader::commissionProfile()
{
    QVariantList items;

    int cid = _serviceCurrent.value("commission_id").toInt();

    if (cid > 0) {
        for (int i = 1; i <= commissionMap.count(); i++) {
            QVariantMap item;
            item["type"] = commissionMap[i]["type"].toInt();
            item["sum_from"] = commissionMap[i]["sum_from"].toDouble();
            item["sum_to"] = commissionMap[i]["sum_to"].toDouble();
            item["value"] = commissionMap[i]["value"].toDouble();

            items.append(item);
        }
    }

    QJsonDocument doc;
    doc.setArray(QJsonArray::fromVariantList(items));

    return doc.toJson();
}

void MainPageLoader::getCommissionMap()
{
    commissionMap.clear();

    QSqlQuery userSql(db);

    int cid = _serviceCurrent.value("commission_id").toInt();
    int serviceId = _serviceCurrent.value("id").toInt();

    if (cid == 0) {
        commissionMap[1]["sum_from"] = _serviceCurrent.value("amount_min").toString();
        commissionMap[1]["sum_to"] = _serviceCurrent.value("amount_max").toString();
        commissionMap[1]["type"] = "1";
        commissionMap[1]["value"] = _serviceCurrent.value("commission").toString();
    } else {
        QString userQuery = QString("SELECT * FROM terminal_commission WHERE service_id = %1 AND commission_id = %2 ORDER BY idx;").arg(serviceId).arg(cid);

        if (userSql.exec(userQuery)) {

            QSqlRecord record1 = userSql.record();
            int j = 1;

            while(userSql.next()){
                commissionMap[j]["sum_from"] = userSql.value(record1.indexOf("commission_sum_from")).toString();
                commissionMap[j]["sum_to"] = userSql.value(record1.indexOf("commission_sum_to")).toString();
                commissionMap[j]["type"] = userSql.value(record1.indexOf("commission_i_type")).toString();
                commissionMap[j]["value"] = userSql.value(record1.indexOf("commission_value")).toString();

                if (commissionMap[j]["sum_to"].toInt() <= 0) {
                    commissionMap[j]["sum_to"] = _serviceCurrent.value("amount_max").toString();
                }

                j++;
            }
        }
    }
}

MainPageLoader::~MainPageLoader()
{
    webView->close();
    delete ui;
}

void MainPageLoader::btnGotoInputSumClc()
{
    //Активируем купюроприемник
    emit validator_activate(true);
}

void MainPageLoader::adminOpen(){

    //Переходим на страницу
    gotoPage(PageIn::Main);

    //Оповещаем о том что надо открыть админку
    emit emit_openAvtorizationDialog();
}

void MainPageLoader::langSet(QString lang)
{
    if (lang == "") {
        return;
    };

    _lang = lang;
}

void MainPageLoader::langDefaultSet(QString lang)
{
    if (lang == "") {
        return;
    }

    _langDefault = lang;
}

void MainPageLoader::playSoundRepeet(int page)
{
    QString fileNmae = "";

    switch(page){
        //Переход на главную страницу
        case PageIn::Main :
        break;
        case PageIn::SelectGroup :
            fileNmae = Sound::sGetCategory;
        break;
        case PageIn::SelectServices :
            fileNmae = Sound::sGetOperator;
        break;

        case PageIn::InputNumber :
            fileNmae = Sound::sInputNumber;
        break;

        case PageIn::InputSum :
            fileNmae = Sound::sInputMoney;
        break;
        case PageIn::PrintDialog : {
            //Проверяем надо ли показывать диалог печати чека
            if (printDialogShow) {
//                fileNmae = Sound::sThanks;
            }else{
//                fileNmae = Sound::sThanksFull;
                fileNmae = Sound::sPrintChek;
            }
        }
        break;

        case PageIn::AfterPrint:
        break;
        case PageIn::LockTerminal:
        break;
        case PageIn::Information :
        break;
        default:
        break;
    }

    //Проигрываем соунд
    if (fileNmae != "") {
        playSound(fileNmae);
    }
}

void MainPageLoader::interfaceCacheClear()
{
    //очищаем кеш
    webView->page()->settings()->clearMemoryCaches();
}

bool MainPageLoader::connectionCheck() {
    return connectionIsUp;
}
