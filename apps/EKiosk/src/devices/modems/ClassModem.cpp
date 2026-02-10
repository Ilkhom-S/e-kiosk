#include "ClassModem.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

QStringList Modem_List;

ClassModem::ClassModem(QObject *parent) : QThread(parent), ATDevice(new ATProtocol()) {

    Modem_List << "AT-Modem";
}

void ClassModem::sendSmsToAgentFew(QStringList smsId) {
    smsIdIn = smsId;
}

void ClassModem::setPort(QString portName) {
    ATDevice->setPortName(portName);
}

void ClassModem::close() {
    ATDevice->closePort();
}

bool ClassModem::e_Data_Execute() {
    bool resp = false;
    // Открываем порт
    ATDevice->openPort();

    //        if(ATDevice->is_open){
    QByteArray request;
    QByteArray response;

    if (command == Modem_Cmd::GetOperator) {
        if (ATDevice->processCommand(Modem_ProtocolCommands::GetOperator, request, response)) {
            // qDebug() << "\n<<======Answer true";
            resp = true;
        } else {

            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == Modem_Cmd::GetSignalQuality) {
        if (ATDevice->processCommand(Modem_ProtocolCommands::GetSignalQuality, request, response)) {
            // qDebug() << "\n<<======Answer true";
            resp = true;
        } else {
            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == Modem_Cmd::CmdRestart) {
        if (ATDevice->processCommand(Modem_ProtocolCommands::CmdRestart, request, response)) {
            // qDebug() << "\n<<======Answer true";
            resp = true;
        } else {
            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == Modem_Cmd::GetBalance) {
        this->setBalanceRequest(
            this->ussdRequestBalanseSim, CModem_Constants::regExpBalance, this->indexBalanceParse);
        if (ATDevice->processCommand(Modem_ProtocolCommands::GetBalance, request, response)) {
            this->nowBalanceSim = ATDevice->nowSim_Balance;
            // qDebug() << "\n<<======Answer true";
            resp = true;
        } else {
            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == Modem_Cmd::GetSim_Number) {
        this->setSim_NumberRequest(this->ussdRequestNumberSim, CModem_Constants::regExpNumberSim);
        if (ATDevice->processCommand(Modem_ProtocolCommands::GetSim_Number, request, response)) {
            this->nowNumberSim = ATDevice->nowSim_Number;
            // qDebug() << "\n<<======Answer true";
            resp = true;
        } else {
            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == Modem_Cmd::CmdSendSms) {

        //            QString vrm_Inf;
        //            for(int jek = 0; jek < this->smsIdIn.count(); jek++){
        //                vrm_Inf += this->ATDevice->smsTextInit.at(jek) + "\n";
        //            }

        //            qDebug() << vrm_Inf;

        //            QString text = this->numberTrm + "\n" +
        //                           QDateTime::currentDateTime().toString("yyyy-MM-dd
        //                           HH:mm:ss") + "\n" + vrm_Inf;

        // qDebug() << text;
        if (this->ATDevice->sendSMSParam(this->SMSTEXT_TO)) {
            emit this->emit_statusSmsSend(true, this->smsIdIn);
        } else {
            emit this->emit_statusSmsSend(false, this->smsIdIn);
        }
    }
    // Закрываем порт
    ATDevice->closePort();
    QCoreApplication::processEvents();
    //    }else{
    //        //qDebug() << "Error opend Modem Port";
    //    }
    return resp;
}

void ClassModem::run() {
    this->e_Data_Execute();
}

bool ClassModem::isItYou(QStringList &comList,
                         QString &modemName,
                         QString &comStr,
                         QString &modemComent) {
    modemName = Modem_List.at(0);

    if ((modemName != "") && (comStr != "") && (comStr.contains("COM"))) {
        this->setPort(comStr);
        if (this->isItYou(modemComent)) {
            return true;
        }
    } else {
        // qDebug() << "Error COM port name;";
        //         return false;
    }

    int comLstC = comList.count();
    for (int comCount = 0; comCount < comLstC; comCount++) {

        const QString &vrmPort = comList.at(comCount);
        // qDebug() << "--- com_count  - " << com_count;
        // qDebug() << "--- vrm_Port    - " << vrm_Port;

        this->setPort(vrmPort);

        if (this->isItYou(modemComent)) {
            comStr = vrmPort;
            return true;
        }
        ATDevice->closePort();
    }

    return false;
}

void ClassModem::setBalanceRequest(QString text1, QString text2, int position) {
    ATDevice->balanceRequest = text1;
    ATDevice->regExpBalance = text2;
    ATDevice->position = position;
}
void ClassModem::setSim_NumberRequest(QString text1, QString text2) {
    ATDevice->sim_NumberRequest = text1;
    ATDevice->regExpSim_Number = text2;
}

bool ClassModem::isItYou(QString &modemComent) {
    // открываем модем для записи
    if (ATDevice->openPort()) {
        qDebug() << "modem open  port";

        nowSim_Present = true;

        QByteArray cmdData;
        QByteArray answerData;

        Modem_ProtocolCommands::Enum protocolCommand = Modem_ProtocolCommands::Reset;
        // 1) Сброс настроек
        // несколько попыток
        bool resultReset = false;
        for (int i = 0; i < 2; ++i) {
            if (ATDevice->processCommand(protocolCommand, cmdData, answerData)) {
                resultReset = true;
                break;
            }
        }
        if (!resultReset) {
            // qDebug() << ": Error processing protocol command = RESET";
            return false;
        }

        // qDebug() << "\nModem reset OK\n";

        // 2) отключим эхо вывод команд
        protocolCommand = Modem_ProtocolCommands::OffEcho;
        if (!ATDevice->processCommand(protocolCommand, cmdData, answerData)) {
            // qDebug() << ": Error processing protocol command = OFFECHO";
            return false;
        }

        // qDebug() << "\nModem OFFECHO OK\n";

        // 3) проверим симку
        protocolCommand = Modem_ProtocolCommands::IsPin;
        if (!ATDevice->processCommand(protocolCommand, cmdData, answerData)) {
            nowSim_Present = false;
            // qDebug() << ": Error processing protocol command = ISPIN, may be SIM is
            // absent";
            //             return false;
        }

        // qDebug() << "\nModem isPin OK\n";

        // Уровент сигнала
        protocolCommand = Modem_ProtocolCommands::GetSignalQuality;

        if (ATDevice->processCommand(protocolCommand, cmdData, answerData)) {
            this->nowModem_Quality = ATDevice->nowSignalQuality;
            // qDebug() << "\n<<======Answer true";
        } else {
            // qDebug() << "\n<<======Answer false";
        }

        // qDebug() << "\nModem signalQuality OK\n";

        // Какой модем
        protocolCommand = Modem_ProtocolCommands::GetComment;

        if (ATDevice->processCommand(protocolCommand, cmdData, answerData)) {
            this->nowModem_Comment = ATDevice->nowModem_Comment;
            // qDebug() << "\n<<======Answer true";
        } else {
            // qDebug() << "\n<<======Answer false";
        }

        // qDebug() << "\nModem modem_Comment OK\n";

        if (nowSim_Present) {
            // Берём оператора
            protocolCommand = Modem_ProtocolCommands::GetOperator;

            if (ATDevice->processCommand(protocolCommand, cmdData, answerData)) {
                nowProviderSim = ATDevice->nowProviderSim;
                // qDebug() << "\n<<======Answer true";
            } else {
                // qDebug() << "\n<<======Answer false";
            }

            // qDebug() << "\nModem getOperator OK\n";
        }

        //    modem_coment = QString("");
        if (nowProviderSim != "") {
            modemComent = QString("( %1 )").arg(nowProviderSim);
        } else {
            modemComent = QString("( ---NO--- )");
        }

        QString vrm = modemComent;
        if (nowModem_Comment != "") {
            modemComent = QString("(%1 %2)").arg(nowModem_Comment, vrm);
        }

        ATDevice->closePort();
        return true;
    }
    ATDevice->closePort();
    return false;
}

bool ClassModem::execCommand(Modem_ProtocolCommands::Enum aCommand, bool thread) {
    switch (aCommand) {
    case Modem_ProtocolCommands::GetSignalQuality: {
        command = Modem_Cmd::GetSignalQuality;
    } break;
    case Modem_ProtocolCommands::GetOperator: {
        command = Modem_Cmd::GetOperator;
    } break;
    case Modem_ProtocolCommands::GetBalance: {
        command = Modem_Cmd::GetBalance;
    } break;
    case Modem_ProtocolCommands::Identification: {
        command = Modem_Cmd::Identification;
    } break;
    case Modem_ProtocolCommands::OffEcho: {
        command = Modem_Cmd::OffEcho;
    } break;
    case Modem_ProtocolCommands::GetComment: {
        command = Modem_Cmd::GetComment;
    } break;
    case Modem_ProtocolCommands::GetSim_Number: {
        command = Modem_Cmd::GetSim_Number;
    } break;
    case Modem_ProtocolCommands::Reset: {
        command = Modem_Cmd::Reset;
    } break;
    case Modem_ProtocolCommands::CmdRestart: {
        command = Modem_Cmd::CmdRestart;
    } break;
    case Modem_ProtocolCommands::CmdSendSMS: {
        command = Modem_Cmd::CmdSendSms;
    } break;
    default: {
        command = Modem_Cmd::Uknown;
    }
    };
    if (thread) {
        this->start();
        return true;
    }

    return this->e_Data_Execute();
}
