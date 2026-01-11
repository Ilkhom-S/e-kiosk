#include "ClassDevice.h"

SearchDevices::SearchDevices(QObject *parent) : QThread(parent) {
    validator = new ClassValidator();
    coinAccepter = new ClassAcceptor();
    printer = new ClassPrinter();
    modem = new ClassModem();
    watchDogs = new WatchDogs();

    modemConUp = false;

    debugger = false;
}

void SearchDevices::setComListInfo(QStringList com_list) { comList = com_list; }

void SearchDevices::setDbName(QSqlDatabase &dbName) { this->db = dbName; }

void SearchDevices::run() {
    QString v_name = "";
    QString c_port = "";
    QString v_comment = "";

    QStringList foundPort;

    for (int ii = 0; ii < comList.count(); ii++) {
        if (debugger) qDebug() << "EXIST --- " << comList.at(ii);
    }

    // сделаем небольшую проверку для принтера km1x
    QString vrm_name = "";
    QString vrm_port = "";
    if (getDeviseInBase(SearchDev::search_printer, vrm_name, vrm_port)) {
        if (vrm_name == PrinterModel::KM1X) {
            // убираем его порт из поиска (потому что печатает)
            int i = comList.indexOf(vrm_port);
            comList.removeAt(i);
            foundPort << vrm_port;
        }
    }

    //((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
    // Начинаем искать купюроприёмник

    // Вытаскиваем из базы устройство Купюроприёмник;
    if (getDeviseInBase(SearchDev::search_validator, v_name, c_port)) {
        if (debugger) qDebug() << "Devices present in database.db";
    }

    if (searchValidator) {
        emit emitDeviceSearch(SearchDev::search_validator, SearchDev::start_search, v_name,
                              v_comment, c_port);

        // Даем выбранные устройства на проверку
        if (searchDeviceMethod(SearchDev::search_validator, v_name, c_port, v_comment)) {
            // Устройство получило результат true
            // Тут будет отпрален сигнал устройство найдено...
            emit emitDeviceSearch(SearchDev::search_validator, SearchDev::device_found, v_name,
                                  v_comment, c_port);
            this->setDeviseInBase(SearchDev::search_validator, v_name, c_port, v_comment, 1);

            int i = comList.indexOf(c_port);
            comList.removeAt(i);
            foundPort << c_port;
        } else {
            // Устройство не найдено
            emit emitDeviceSearch(SearchDev::search_validator, SearchDev::device_notfound, v_name,
                                  v_comment, c_port);
            this->setDeviseInBase(SearchDev::search_validator, v_name, c_port, v_comment, 0);
        }
    }

    for (int ii = 0; ii < comList.count(); ii++) {
        if (debugger) qDebug() << "EXIST --- " << comList.at(ii);
    }

    this->msleep(500);

    //((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
    // Начинаем искать монетоприемник

    // Вытаскиваем из базы устройство монетоприемник;
    if (getDeviseInBase(SearchDev::search_coin_acceptor, v_name, c_port)) {
        if (debugger) qDebug() << "Coin Acceptor present in database.db";
    }

    if (searchCoinAcceptor) {
        emit emitDeviceSearch(SearchDev::search_coin_acceptor, SearchDev::start_search, v_name,
                              v_comment, c_port);

        // Даем выбранные устройства на проверку
        if (searchDeviceMethod(SearchDev::search_coin_acceptor, v_name, c_port, v_comment)) {
            // Устройство получило результат true
            // Тут будет отпрален сигнал устройство найдено...
            emit emitDeviceSearch(SearchDev::search_coin_acceptor, SearchDev::device_found, v_name,
                                  v_comment, c_port);
            this->setDeviseInBase(SearchDev::search_coin_acceptor, v_name, c_port, v_comment, 1);

            int i = comList.indexOf(c_port);
            comList.removeAt(i);
            foundPort << c_port;
        } else {
            // Устройство не найдено
            emit emitDeviceSearch(SearchDev::search_coin_acceptor, SearchDev::device_notfound,
                                  v_name, v_comment, c_port);
            this->setDeviseInBase(SearchDev::search_coin_acceptor, v_name, c_port, v_comment, 0);
        }
    }

    for (int ii = 0; ii < comList.count(); ii++) {
        qDebug() << "EXIST --- " << comList.at(ii);
    }

    //((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
    // Начинаем искать принтер
    v_name = "";
    c_port = "";
    v_comment = "";

    // Вытаскиваем из базы устройство принтер;
    if (getDeviseInBase(SearchDev::search_printer, v_name, c_port)) {
        if (debugger) qDebug() << "Devices present in database.db";
    }

    if (v_name != PrinterModel::Windows_Printer) {
        // Надо ли искать принтер вообще
        if (searchPrinter) {
            emit emitDeviceSearch(SearchDev::search_printer, SearchDev::start_search, v_name,
                                  v_comment, c_port);

            // если это принтер km1x, то всегда отправляем found
            if (v_name == PrinterModel::KM1X) {
                emit emitDeviceSearch(SearchDev::search_printer, SearchDev::device_found, v_name,
                                      v_comment, c_port);

                int i = comList.indexOf(c_port);
                comList.removeAt(i);
                foundPort << c_port;

            } else {
                for (int ii = 0; ii < foundPort.count(); ii++) {
                    if (foundPort.at(ii) == c_port) {
                        c_port = "";
                    }
                }

                // Даем выбранные устройства на проверку
                if (searchDeviceMethod(SearchDev::search_printer, v_name, c_port, v_comment)) {
                    // Устройство получило результат true
                    // Тут будет отпрален сигнал устройство найдено...
                    emit emitDeviceSearch(SearchDev::search_printer, SearchDev::device_found,
                                          v_name, v_comment, c_port);
                    setDeviseInBase(SearchDev::search_printer, v_name, c_port, v_comment, 1);

                    int i = comList.indexOf(c_port);
                    comList.removeAt(i);
                    foundPort << c_port;
                } else {
                    // Устройство не найдено
                    emit emitDeviceSearch(SearchDev::search_printer, SearchDev::device_notfound,
                                          v_name, v_comment, c_port);
                    setDeviseInBase(SearchDev::search_printer, v_name, c_port, v_comment, 0);
                }
            }
            //))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
        }
    } else {
        // Тут будет отображаться WinPrinter
        v_name = PrinterModel::Windows_Printer;

        // Тут коментарий надо показывать какой именно принтер установлен
        v_comment = prtWinName;
        c_port = "";
        emit emitDeviceSearch(SearchDev::search_printer, SearchDev::device_found, v_name, v_comment,
                              c_port);
        setDeviseInBase(SearchDev::search_printer, v_name, c_port, v_comment, 1);
    }

    for (int ii = 0; ii < comList.count(); ii++) {
        if (debugger) qDebug() << "EXIST --- " << comList.at(ii);
    }

    // Ищем сторожевой таймер
    v_name = "";
    c_port = "";
    v_comment = "";

    // Вытаскиваем из базы устройство;
    if (getDeviseInBase(SearchDev::search_watchdog, v_name, c_port)) {
        if (debugger) qDebug() << "Devices watchdogs present in database.db";
    } else {
        if (debugger) qDebug() << "Devices watchdogs not present in database.db";
    }

    for (int ii = 0; ii < foundPort.count(); ii++) {
        if (foundPort.at(ii) == c_port) {
            c_port = "";
        }
    }

    if (searchWD) {
        emit emitDeviceSearch(SearchDev::search_watchdog, SearchDev::start_search, v_name,
                              v_comment, c_port);

        // Даем выбранные устройства на проверку
        if (searchDeviceMethod(SearchDev::search_watchdog, v_name, c_port, v_comment)) {
            // Устройство получило результат true
            // Тут будет отпрален сигнал устройство найдено...
            emit emitDeviceSearch(SearchDev::search_watchdog, SearchDev::device_found, v_name,
                                  v_comment, c_port);
            setDeviseInBase(SearchDev::search_watchdog, v_name, c_port, v_comment, 1);

            int i = comList.indexOf(c_port);
            comList.removeAt(i);
            foundPort << c_port;
        } else {
            // Устройство не найдено
            emit emitDeviceSearch(SearchDev::search_watchdog, SearchDev::device_notfound, v_name,
                                  v_comment, c_port);
            setDeviseInBase(SearchDev::search_watchdog, v_name, c_port, v_comment, 0);
        }
    }

    for (int ii = 0; ii < comList.count(); ii++) {
        if (debugger) qDebug() << "EXIST --- " << comList.at(ii);
    }

    //((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
    // Начинаем искать модем
    v_name = "";
    c_port = "";
    v_comment = "";

    // Вытаскиваем из базы устройство модем;
    if (getDeviseInBase(SearchDev::search_modem, v_name, c_port)) {
        if (debugger) qDebug() << "Devices present in database.db";
    }

    // Надо ли искать модем вообще
    if (searchModem) {
        emit emitDeviceSearch(SearchDev::search_modem, SearchDev::start_search, v_name, v_comment,
                              c_port);

        if (!testMode) {
            msleep(30000);
        }

        for (int ii = 0; ii < foundPort.count(); ii++) {
            if (foundPort.at(ii) == c_port) {
                c_port = "";
            }
        }

        // Даем выбранные устройства на проверку
        if (!modemConUp) {
            if (searchDeviceMethod(SearchDev::search_modem, v_name, c_port, v_comment)) {
                // Устройство получило результат true
                // Тут будет отпрален сигнал устройство найдено...
                emit emitDeviceSearch(SearchDev::search_modem, SearchDev::device_found, v_name,
                                      v_comment, c_port);
                setDeviseInBase(SearchDev::search_modem, v_name, c_port, v_comment, 1);

                int i = comList.indexOf(c_port);
                comList.removeAt(i);
                foundPort << c_port;

                if (nowSimPresent) {
                    if (takeBalanceSim) takeBalanceSim = true;
                    if (takeSimNumber) takeSimNumber = true;
                } else {
                    takeBalanceSim = false;
                    takeSimNumber = false;
                }
                vrmModemPort = c_port;
                this->msleep(2000);

            } else {
                // Устройство не найдено
                emit emitDeviceSearch(SearchDev::search_modem, SearchDev::device_notfound, v_name,
                                      v_comment, c_port);
                setDeviseInBase(SearchDev::search_modem, v_name, c_port, v_comment, 0);

                takeBalanceSim = false;
                takeSimNumber = false;
            }
        } else {
            // Устройство не найдено
            emit emitDeviceSearch(SearchDev::search_modem, SearchDev::device_notfound, v_name,
                                  v_comment, c_port);
            setDeviseInBase(SearchDev::search_modem, v_name, c_port, v_comment, 0);

            takeBalanceSim = false;
            takeSimNumber = false;
        }
        //))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))

        msleep(1000);

        if (takeSimNumber) {
            modem->setPort(vrmModemPort);
            modem->ussdRequestNumberSim = s_ussdRequestNumberSim;

            if (s_ussdRequestNumberSim.trimmed() != "") {
                modem->execCommand(ModemProtocolCommands::GetSimNumber, false);
                simNumber = modem->nowNumberSim;
                msleep(1000);
            }
        } else {
            msleep(2000);
        }

        msleep(1000);

        if (takeBalanceSim) {
            if (!takeSimNumber) {
                modem->setPort(c_port);
            }

            modem->ussdRequestBalanseSim = s_ussdRequestBalanseSim;
            modem->indexBalanceParse = s_indexBalanceParse;

            if (s_ussdRequestBalanseSim.trimmed() != "") {
                modem->execCommand(ModemProtocolCommands::GetBalance, false);
                simBalance = modem->nowBalanceSim;
                msleep(1000);
            }
        } else {
            msleep(2000);
        }
    }

    QCoreApplication::processEvents();
    emit emitDeviceSearchFinished();

    return;
}

void SearchDevices::getModemInfo() {
    //        if(takeBalanceSim){
    //            ClassModem *Modem = new ClassModem();
    //            Modem->setPort(vrmModemPort);
    //            Modem->execCommand(ModemProtocolCommands::GetBalance);
    ////            connect();
    ////            timeSearch = 10000;
    ////            this->msleep(15000);
    //        }
}

bool SearchDevices::setDeviseInBase(int id_device, const QString &name_device, const QString &port,
                                    const QString &comment, int state) {
    QSqlQuery updateDevices(this->db);
    QString strUpdate;
    id_device++;
    strUpdate =
        QString(
            "UPDATE terminal_devices SET name = \"%1\", port = \"%2\", comment = \"%3\", state = "
            "%4 WHERE id = %5")
            .arg(name_device, port, comment, QString::number(state), QString::number(id_device));

    if (!updateDevices.exec(strUpdate)) {
        //        if(Debuger) qDebug() << updateDevices.lastError();
        //        if(Debuger) qDebug() << "There are no data to update";

        return false;
    }

    return true;
}

bool SearchDevices::insertDeviseInBase(int id_device, QString &name_device, QString &port) {
    Q_UNUSED(name_device)
    Q_UNUSED(port)

    QSqlQuery selectDevices(this->db);
    QString strSelect;
    id_device++;

    strSelect =
        QString("INSERT INTO terminal_devices VALUES('',%1,'null','null',0);").arg(id_device);

    if (!selectDevices.exec(strSelect)) {
        //        if(Debuger) qDebug() << selectDevices.lastError();
        //        if(Debuger) qDebug() << "There are no data " << name_device;

        return false;
    }

    return true;
}

bool SearchDevices::getDeviseInBase(int id_device, QString &name_device, QString &port) {
    QSqlQuery selectDevices(this->db);
    QString strSelect;
    id_device++;

    strSelect = QString("SELECT * FROM terminal_devices WHERE id = %1").arg(id_device);

    if (!selectDevices.exec(strSelect)) {
        //        if(Debuger) qDebug() << selectDevices.lastError();
        //        if(Debuger) qDebug() << "There are no data " << name_device;
        return false;
    }

    QSqlRecord record = selectDevices.record();

    if (selectDevices.next()) {
        name_device = selectDevices.value(record.indexOf("name")).toString();
        port = selectDevices.value(record.indexOf("port")).toString();

        return true;
    } else {
        id_device = id_device - 1;
        insertDeviseInBase(id_device, name_device, port);
    }
    return false;
}

bool SearchDevices::searchDeviceMethod(int device, QString &dev_name, QString &com_str,
                                       QString &dev_coment, const bool with_test) {
    bool result = false;
    if (device == SearchDev::search_validator) {
        if (validator->isItYou(comList, dev_name, com_str, dev_coment)) {
            this->validatorPartNum = validator->vPartNumber;
            this->validatorSerialNum = validator->vSerialNumber;
            result = true;
        }
    }

    if (device == SearchDev::search_coin_acceptor) {
        if (coinAccepter->isItYou(comList, dev_name, com_str, dev_coment)) {
            this->coinAcceptorSerialNum = coinAccepter->v_SerialNumber;
            this->coinAcceptorPartNum = coinAccepter->v_PartNumber;
            result = true;
        }
    }

    if (device == SearchDev::search_printer) {
        if (dev_name == PrinterModel::Windows_Printer) {
            result = true;

            if (with_test) {
                printer->setPrinterModel(dev_name);
                printer->winPrinterName = dev_coment;
                printer->textToPrint = receiptTest;
                printer->CPrint();
            }

            return result;
        }
        if (printer->isItYou(comList, dev_name, com_str, dev_coment)) {
            result = true;
        }
    }

    if (device == SearchDev::search_modem) {
        if (modem->isItYou(comList, dev_name, com_str, dev_coment)) {
            result = true;
            this->nowSimPresent = modem->nowSimPresent;
            this->signalQuality = modem->nowModemQuality;
            this->operatorName = modem->nowProviderSim;
            this->modemComment = modem->nowModemComment;

            this->modemFound = true;
        } else {
            modem->close();
            this->modemFound = false;
        }
    }

    if (device == SearchDev::search_watchdog) {
        if (watchDogs->isItYou(comList, dev_name, com_str, dev_coment)) {
            result = true;
        }
    }

    return result;
}
