#include "ClassDevice.h"

SearchDevices::SearchDevices(QObject *parent)
    : QThread(parent), validator(new ClassValidator()), coinAccepter(new ClassAcceptor()),
      printer(new ClassPrinter()), modem(new ClassModem()), watchDogs(new WatchDogs()),
      modem_ConUp(false), debugger(false) {}

void SearchDevices::setCom_ListInfo(QStringList comList) {
    com_List = comList;
}

void SearchDevices::setDbName(QSqlDatabase &dbName) {
    this->db = dbName;
}

void SearchDevices::run() {
    QString vName = "";
    QString cPort = "";
    QString vComment = "";

    QStringList foundPort;

    for (int ii = 0; ii < com_List.count(); ii++) {
        if (debugger) {
            qDebug() << "EXIST --- " << com_List.at(ii);
        }
    }

    // сделаем небольшую проверку для принтера km1x
    QString vrmName = "";
    QString vrmPort = "";
    if (getDeviseInBase(SearchDev::search_printer, vrmName, vrmPort)) {
        if (vrmName == PrinterModel::KM1X) {
            // убираем его порт из поиска (потому что печатает)
            int i = com_List.indexOf(vrmPort);
            com_List.removeAt(i);
            foundPort << vrmPort;
        }
    }

    //((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
    // Начинаем искать купюроприёмник

    // Вытаскиваем из базы устройство Купюроприёмник;
    if (getDeviseInBase(SearchDev::search_validator, vName, cPort)) {
        if (debugger) {
            qDebug() << "Devices present in database.db";
        }
    }

    if (searchValidator) {
        emit emitDeviceSearch(
            SearchDev::search_validator, SearchDev::start_search, vName, vComment, cPort);

        // Даем выбранные устройства на проверку
        if (searchDeviceMethod(SearchDev::search_validator, vName, cPort, vComment)) {
            // Устройство получило результат true
            // Тут будет отпрален сигнал устройство найдено...
            emit emitDeviceSearch(
                SearchDev::search_validator, SearchDev::device_found, vName, vComment, cPort);
            this->setDeviseInBase(SearchDev::search_validator, vName, cPort, vComment, 1);

            int i = com_List.indexOf(cPort);
            com_List.removeAt(i);
            foundPort << cPort;
        } else {
            // Устройство не найдено
            emit emitDeviceSearch(
                SearchDev::search_validator, SearchDev::device_notfound, vName, vComment, cPort);
            this->setDeviseInBase(SearchDev::search_validator, vName, cPort, vComment, 0);
        }
    }

    for (int ii = 0; ii < com_List.count(); ii++) {
        if (debugger) {
            qDebug() << "EXIST --- " << com_List.at(ii);
        }
    }

    SearchDevices::msleep(500);

    //((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
    // Начинаем искать монетоприемник

    // Вытаскиваем из базы устройство монетоприемник;
    if (getDeviseInBase(SearchDev::search_coin_acceptor, vName, cPort)) {
        if (debugger) {
            qDebug() << "Coin Acceptor present in database.db";
        }
    }

    if (searchCoinAcceptor) {
        emit emitDeviceSearch(
            SearchDev::search_coin_acceptor, SearchDev::start_search, vName, vComment, cPort);

        // Даем выбранные устройства на проверку
        if (searchDeviceMethod(SearchDev::search_coin_acceptor, vName, cPort, vComment)) {
            // Устройство получило результат true
            // Тут будет отпрален сигнал устройство найдено...
            emit emitDeviceSearch(
                SearchDev::search_coin_acceptor, SearchDev::device_found, vName, vComment, cPort);
            this->setDeviseInBase(SearchDev::search_coin_acceptor, vName, cPort, vComment, 1);

            int i = com_List.indexOf(cPort);
            com_List.removeAt(i);
            foundPort << cPort;
        } else {
            // Устройство не найдено
            emit emitDeviceSearch(SearchDev::search_coin_acceptor,
                                  SearchDev::device_notfound,
                                  vName,
                                  vComment,
                                  cPort);
            this->setDeviseInBase(SearchDev::search_coin_acceptor, vName, cPort, vComment, 0);
        }
    }

    for (int ii = 0; ii < com_List.count(); ii++) {
        qDebug() << "EXIST --- " << com_List.at(ii);
    }

    //((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
    // Начинаем искать принтер
    vName = "";
    cPort = "";
    vComment = "";

    // Вытаскиваем из базы устройство принтер;
    if (getDeviseInBase(SearchDev::search_printer, vName, cPort)) {
        if (debugger) {
            qDebug() << "Devices present in database.db";
        }
    }

    if (vName != PrinterModel::Windows_Printer) {
        // Надо ли искать принтер вообще
        if (searchPrinter) {
            emit emitDeviceSearch(
                SearchDev::search_printer, SearchDev::start_search, vName, vComment, cPort);

            // если это принтер km1x, то всегда отправляем found
            if (vName == PrinterModel::KM1X) {

                emit emitDeviceSearch(
                    SearchDev::search_printer, SearchDev::device_found, vName, vComment, cPort);

                int i = com_List.indexOf(cPort);
                com_List.removeAt(i);
                foundPort << cPort;
            } else {

                for (int ii = 0; ii < foundPort.count(); ii++) {
                    if (foundPort.at(ii) == cPort) {
                        cPort = "";
                    }
                }

                // Даем выбранные устройства на проверку
                if (searchDeviceMethod(SearchDev::search_printer, vName, cPort, vComment)) {
                    // Устройство получило результат true
                    // Тут будет отпрален сигнал устройство найдено...
                    emit emitDeviceSearch(
                        SearchDev::search_printer, SearchDev::device_found, vName, vComment, cPort);
                    setDeviseInBase(SearchDev::search_printer, vName, cPort, vComment, 1);

                    int i = com_List.indexOf(cPort);
                    com_List.removeAt(i);
                    foundPort << cPort;
                } else {
                    // Устройство не найдено
                    emit emitDeviceSearch(SearchDev::search_printer,
                                          SearchDev::device_notfound,
                                          vName,
                                          vComment,
                                          cPort);
                    setDeviseInBase(SearchDev::search_printer, vName, cPort, vComment, 0);
                }
            }
            //))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
        }
    } else {
        // Тут будет отображаться WinPrinter
        vName = PrinterModel::Windows_Printer;

        // Тут коментарий надо показывать какой именно принтер установлен
        vComment = prtWinName;
        cPort = "";
        emit emitDeviceSearch(
            SearchDev::search_printer, SearchDev::device_found, vName, vComment, cPort);
        setDeviseInBase(SearchDev::search_printer, vName, cPort, vComment, 1);
    }

    for (int ii = 0; ii < com_List.count(); ii++) {
        if (debugger) {
            qDebug() << "EXIST --- " << com_List.at(ii);
        }
    }

    // Ищем сторожевой таймер
    vName = "";
    cPort = "";
    vComment = "";

    // Вытаскиваем из базы устройство;
    if (getDeviseInBase(SearchDev::search_watchdog, vName, cPort)) {
        if (debugger) {
            qDebug() << "Devices watchdogs present in database.db";
        }
    } else {
        if (debugger) {
            qDebug() << "Devices watchdogs not present in database.db";
        }
    }

    for (int ii = 0; ii < foundPort.count(); ii++) {
        if (foundPort.at(ii) == cPort) {
            cPort = "";
        }
    }

    if (searchWD) {
        emit emitDeviceSearch(
            SearchDev::search_watchdog, SearchDev::start_search, vName, vComment, cPort);

        // Даем выбранные устройства на проверку
        if (searchDeviceMethod(SearchDev::search_watchdog, vName, cPort, vComment)) {
            // Устройство получило результат true
            // Тут будет отпрален сигнал устройство найдено...
            emit emitDeviceSearch(
                SearchDev::search_watchdog, SearchDev::device_found, vName, vComment, cPort);
            setDeviseInBase(SearchDev::search_watchdog, vName, cPort, vComment, 1);

            int i = com_List.indexOf(cPort);
            com_List.removeAt(i);
            foundPort << cPort;
        } else {
            // Устройство не найдено
            emit emitDeviceSearch(
                SearchDev::search_watchdog, SearchDev::device_notfound, vName, vComment, cPort);
            setDeviseInBase(SearchDev::search_watchdog, vName, cPort, vComment, 0);
        }
    }

    for (int ii = 0; ii < com_List.count(); ii++) {
        if (debugger) {
            qDebug() << "EXIST --- " << com_List.at(ii);
        }
    }

    //((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((
    // Начинаем искать модем
    vName = "";
    cPort = "";
    vComment = "";

    // Вытаскиваем из базы устройство модем;
    if (getDeviseInBase(SearchDev::search_modem, vName, cPort)) {
        if (debugger) {
            qDebug() << "Devices present in database.db";
        }
    }

    // Надо ли искать модем вообще
    if (searchModem) {
        emit emitDeviceSearch(
            SearchDev::search_modem, SearchDev::start_search, vName, vComment, cPort);

        if (!testMode) {
            msleep(30000);
        }

        for (int ii = 0; ii < foundPort.count(); ii++) {
            if (foundPort.at(ii) == cPort) {
                cPort = "";
            }
        }

        // Даем выбранные устройства на проверку
        if (!modem_ConUp) {
            if (searchDeviceMethod(SearchDev::search_modem, vName, cPort, vComment)) {
                // Устройство получило результат true
                // Тут будет отпрален сигнал устройство найдено...
                emit emitDeviceSearch(
                    SearchDev::search_modem, SearchDev::device_found, vName, vComment, cPort);
                setDeviseInBase(SearchDev::search_modem, vName, cPort, vComment, 1);

                int i = com_List.indexOf(cPort);
                com_List.removeAt(i);
                foundPort << cPort;

                if (nowSim_Present) {
                    if (takeBalanceSim) {
                        takeBalanceSim = true;
                    }
                    if (takeSim_Number) {
                        takeSim_Number = true;
                    }
                } else {
                    takeBalanceSim = false;
                    takeSim_Number = false;
                }
                vrm_Modem_Port = cPort;
                SearchDevices::msleep(2000);
            } else {
                // Устройство не найдено
                emit emitDeviceSearch(
                    SearchDev::search_modem, SearchDev::device_notfound, vName, vComment, cPort);
                setDeviseInBase(SearchDev::search_modem, vName, cPort, vComment, 0);

                takeBalanceSim = false;
                takeSim_Number = false;
            }
        } else {
            // Устройство не найдено
            emit emitDeviceSearch(
                SearchDev::search_modem, SearchDev::device_notfound, vName, vComment, cPort);
            setDeviseInBase(SearchDev::search_modem, vName, cPort, vComment, 0);

            takeBalanceSim = false;
            takeSim_Number = false;
        }
        //))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))

        msleep(1000);

        if (takeSim_Number) {
            modem->setPort(vrm_Modem_Port);
            modem->ussdRequestNumberSim = s_ussdRequestNumberSim;

            if (s_ussdRequestNumberSim.trimmed() != "") {
                modem->execCommand(Modem_ProtocolCommands::GetSim_Number, false);
                sim_Number = modem->nowNumberSim;
                msleep(1000);
            }
        } else {
            msleep(2000);
        }

        msleep(1000);

        if (takeBalanceSim) {
            if (!takeSim_Number) {
                modem->setPort(cPort);
            }

            modem->ussdRequestBalanseSim = s_ussdRequestBalanseSim;
            modem->indexBalanceParse = s_indexBalanceParse;

            if (s_ussdRequestBalanseSim.trimmed() != "") {
                modem->execCommand(Modem_ProtocolCommands::GetBalance, false);
                sim_Balance = modem->nowBalanceSim;
                msleep(1000);
            }
        } else {
            msleep(2000);
        }
    }

    QCoreApplication::processEvents();
    emit emitDeviceSearchFinished();
}

void SearchDevices::getModem_Info() {
    //        if(takeBalanceSim){
    //            ClassModem *Modem = new ClassModem();
    //            Modem->setPort(vrm_Modem_Port);
    //            Modem->execCommand(Modem_ProtocolCommands::GetBalance);
    ////            connect();
    ////            timeSearch = 10000;
    ////            this->msleep(15000);
    //        }
}

bool SearchDevices::setDeviseInBase(int idDevice,
                                    const QString &nameDevice,
                                    const QString &port,
                                    const QString &comment,
                                    int state) {
    QSqlQuery updateDevices(this->db);
    QString strUpdate;
    idDevice++;
    strUpdate =
        QString("UPDATE terminal_devices SET name = \"%1\", port = "
                "\"%2\", comment = \"%3\", state = %4 WHERE id = %5")
            .arg(nameDevice, port, comment, QString::number(state), QString::number(idDevice));

    if (!updateDevices.exec(strUpdate)) {
        //        if(Debugger) qDebug() << updateDevices.lastError();
        //        if(Debugger) qDebug() << "There are no data to update";

        return false;
    }

    return true;
}

bool SearchDevices::insertDeviseInBase(int idDevice, QString &nameDevice, QString &port) {
    Q_UNUSED(nameDevice)
    Q_UNUSED(port)

    QSqlQuery selectDevices(this->db);
    QString strSelect;
    idDevice++;

    strSelect =
        QString("INSERT INTO terminal_devices VALUES('',%1,'null','null',0);").arg(idDevice);

    if (!selectDevices.exec(strSelect)) {
        //        if(Debugger) qDebug() << selectDevices.lastError();
        //        if(Debugger) qDebug() << "There are no data " << name_device;

        return false;
    }

    return true;
}

bool SearchDevices::getDeviseInBase(int idDevice, QString &nameDevice, QString &port) {
    QSqlQuery selectDevices(this->db);
    QString strSelect;
    idDevice++;

    strSelect = QString("SELECT * FROM terminal_devices WHERE id = %1").arg(idDevice);

    if (!selectDevices.exec(strSelect)) {
        //        if(Debugger) qDebug() << selectDevices.lastError();
        //        if(Debugger) qDebug() << "There are no data " << name_device;
        return false;
    }

    QSqlRecord record = selectDevices.record();

    if (selectDevices.next()) {
        nameDevice = selectDevices.value(record.indexOf("name")).toString();
        port = selectDevices.value(record.indexOf("port")).toString();

        return true;
    }
    idDevice = idDevice - 1;
    insertDeviseInBase(idDevice, nameDevice, port);

    return false;
}

bool SearchDevices::searchDeviceMethod(
    int device, QString &devName, QString &comStr, QString &devComent, const bool withTest) {
    bool result = false;
    if (device == SearchDev::search_validator) {
        if (validator->isItYou(com_List, devName, comStr, devComent)) {
            this->validatorPartNum = validator->vPartNumber;
            this->validatorSerialNum = validator->vSerialNumber;
            result = true;
        }
    }

    if (device == SearchDev::search_coin_acceptor) {
        if (coinAccepter->isItYou(com_List, devName, comStr, devComent)) {
            this->coinAcceptorSerialNum = coinAccepter->v_SerialNumber;
            this->coinAcceptorPartNum = coinAccepter->v_PartNumber;
            result = true;
        }
    }

    if (device == SearchDev::search_printer) {
        if (devName == PrinterModel::Windows_Printer) {
            result = true;

            if (withTest) {
                printer->setPrinterModel(devName);
                printer->winPrinterName = devComent;
                printer->textToPrint = receiptTest;
                printer->CPrint();
            }

            return result;
        }
        if (printer->isItYou(com_List, devName, comStr, devComent)) {
            result = true;
        }
    }

    if (device == SearchDev::search_modem) {
        if (modem->isItYou(com_List, devName, comStr, devComent)) {
            result = true;
            this->nowSim_Present = modem->nowSim_Present;
            this->signalQuality = modem->nowModem_Quality;
            this->operatorName = modem->nowProviderSim;
            this->modem_Comment = modem->nowModem_Comment;

            this->modem_Found = true;
        } else {
            modem->close();
            this->modem_Found = false;
        }
    }

    if (device == SearchDev::search_watchdog) {
        if (watchDogs->isItYou(com_List, devName, comStr, devComent)) {
            result = true;
        }
    }

    return result;
}
