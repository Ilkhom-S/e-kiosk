// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// Project
#include "ClassModem.h"

QStringList Modem_List;

ClassModem::ClassModem(QObject *parent) : QThread(parent)
{
    ATDevice = new ATProtocol();

    Modem_List << "AT-Modem";
}

void ClassModem::sendSmsToAgentFew(QStringList smsId)
{
    smsIdIn = smsId;
}

void ClassModem::setPort(QString port_name)
{
    ATDevice->setPortName(port_name);
}

void ClassModem::close()
{
    ATDevice->closePort();
}

bool ClassModem::e_Data_Execute()
{
    bool resp = false;
    // Открываем порт
    ATDevice->openPort();

    //        if(ATDevice->is_open){
    QByteArray request;
    QByteArray response;

    if (command == ModemCmd::GetOperator)
    {
        if (ATDevice->processCommand(ModemProtocolCommands::GetOperator, request, response))
        {
            // qDebug() << "\n<<======Answer true";
            resp = true;
        }
        else
        {

            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == ModemCmd::GetSignalQuality)
    {
        if (ATDevice->processCommand(ModemProtocolCommands::GetSignalQuality, request, response))
        {
            // qDebug() << "\n<<======Answer true";
            resp = true;
        }
        else
        {
            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == ModemCmd::CmdRestart)
    {
        if (ATDevice->processCommand(ModemProtocolCommands::CmdRestart, request, response))
        {
            // qDebug() << "\n<<======Answer true";
            resp = true;
        }
        else
        {
            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == ModemCmd::GetBalance)
    {
        this->setBalanceRequest(this->ussdRequestBalanseSim, CModemConstants::regExpBalance, this->indexBalanceParse);
        if (ATDevice->processCommand(ModemProtocolCommands::GetBalance, request, response))
        {
            this->nowBalanceSim = ATDevice->nowSimBalance;
            // qDebug() << "\n<<======Answer true";
            resp = true;
        }
        else
        {
            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == ModemCmd::GetSimNumber)
    {
        this->setSimNumberRequest(this->ussdRequestNumberSim, CModemConstants::regExpNumberSim);
        if (ATDevice->processCommand(ModemProtocolCommands::GetSimNumber, request, response))
        {
            this->nowNumberSim = ATDevice->nowSimNumber;
            // qDebug() << "\n<<======Answer true";
            resp = true;
        }
        else
        {
            // qDebug() << "\n<<======Answer false";
            resp = false;
        }
    }

    if (command == ModemCmd::CmdSendSms)
    {

        //            QString vrmInf;
        //            for(int jek = 0; jek < this->smsIdIn.count(); jek++){
        //                vrmInf += this->ATDevice->smsTextInit.at(jek) + "\n";
        //            }

        //            qDebug() << vrmInf;

        //            QString text = this->numberTrm + "\n" +
        //                           QDateTime::currentDateTime().toString("yyyy-MM-dd
        //                           HH:mm:ss") + "\n" + vrmInf;

        // qDebug() << text;
        if (this->ATDevice->sendSMSParam(this->SMSTEXT_TO))
            emit this->emit_statusSmsSend(true, this->smsIdIn);
        else
        {
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

void ClassModem::run()
{
    this->e_Data_Execute();
}

bool ClassModem::isItYou(QStringList &comList, QString &modem_name, QString &com_str, QString &modem_coment)
{
    modem_name = Modem_List.at(0);

    if ((modem_name != "") && (com_str != "") && (com_str.contains("COM")))
    {
        this->setPort(com_str);
        if (this->isItYou(modem_coment))
        {
            return true;
        }
    }
    else
    {
        // qDebug() << "Error COM port name;";
        //         return false;
    }

    int com_lst_c = comList.count();
    for (int com_count = 0; com_count < com_lst_c; com_count++)
    {

        QString vrmPort = comList.at(com_count);
        // qDebug() << "--- com_count  - " << com_count;
        // qDebug() << "--- vrmPort    - " << vrmPort;

        this->setPort(vrmPort);

        if (this->isItYou(modem_coment))
        {
            com_str = vrmPort;
            return true;
        }
        else
        {
            ATDevice->closePort();
        }
    }

    return false;
}

void ClassModem::setBalanceRequest(QString text1, QString text2, int position)
{
    ATDevice->balanceRequest = text1;
    ATDevice->regExpBalance = text2;
    ATDevice->position = position;
}
void ClassModem::setSimNumberRequest(QString text1, QString text2)
{
    ATDevice->simNumberRequest = text1;
    ATDevice->regExpSimNumber = text2;
}

bool ClassModem::isItYou(QString &modem_coment)
{
    // открываем модем для записи
    if (ATDevice->openPort())
    {
        qDebug() << "modem open  port";

        nowSimPresent = true;

        QByteArray cmdData, answerData;

        ModemProtocolCommands::Enum protocolCommand = ModemProtocolCommands::Reset;
        // 1) Сброс настроек
        // несколько попыток
        bool resultReset = false;
        for (int i = 0; i < 2; ++i)
        {
            if (ATDevice->processCommand(protocolCommand, cmdData, answerData))
            {
                resultReset = true;
                break;
            }
        }
        if (!resultReset)
        {
            // qDebug() << ": Error processing protocol command = RESET";
            return false;
        }

        // qDebug() << "\nModem reset OK\n";

        // 2) отключим эхо вывод команд
        protocolCommand = ModemProtocolCommands::OffEcho;
        if (!ATDevice->processCommand(protocolCommand, cmdData, answerData))
        {
            // qDebug() << ": Error processing protocol command = OFFECHO";
            return false;
        }

        // qDebug() << "\nModem OFFECHO OK\n";

        // 3) проверим симку
        protocolCommand = ModemProtocolCommands::IsPin;
        if (!ATDevice->processCommand(protocolCommand, cmdData, answerData))
        {
            nowSimPresent = false;
            // qDebug() << ": Error processing protocol command = ISPIN, may be SIM is
            // absent";
            //             return false;
        }

        // qDebug() << "\nModem isPin OK\n";

        // Уровент сигнала
        protocolCommand = ModemProtocolCommands::GetSignalQuality;

        if (ATDevice->processCommand(protocolCommand, cmdData, answerData))
        {
            this->nowModemQuality = ATDevice->nowSignalQuality;
            // qDebug() << "\n<<======Answer true";
        }
        else
        {
            // qDebug() << "\n<<======Answer false";
        }

        // qDebug() << "\nModem signalQuality OK\n";

        // Какой модем
        protocolCommand = ModemProtocolCommands::GetComment;

        if (ATDevice->processCommand(protocolCommand, cmdData, answerData))
        {
            this->nowModemComment = ATDevice->nowModemComment;
            // qDebug() << "\n<<======Answer true";
        }
        else
        {
            // qDebug() << "\n<<======Answer false";
        }

        // qDebug() << "\nModem modemComment OK\n";

        if (nowSimPresent)
        {
            // Берём оператора
            protocolCommand = ModemProtocolCommands::GetOperator;

            if (ATDevice->processCommand(protocolCommand, cmdData, answerData))
            {
                nowProviderSim = ATDevice->nowProviderSim;
                // qDebug() << "\n<<======Answer true";
            }
            else
            {
                // qDebug() << "\n<<======Answer false";
            }

            // qDebug() << "\nModem getOperator OK\n";
        }

        //    modem_coment = QString("");
        if (nowProviderSim != "")
            modem_coment = QString("( %1 )").arg(nowProviderSim);
        else
            modem_coment = QString("( ---NO--- )");

        QString vrm = modem_coment;
        if (nowModemComment != "")
        {
            modem_coment = QString("(%1 %2)").arg(nowModemComment, vrm);
        }

        ATDevice->closePort();
        return true;
    }
    else
    {
        ATDevice->closePort();
        return false;
    }
}

bool ClassModem::execCommand(ModemProtocolCommands::Enum aCommand, bool thread)
{
    switch (aCommand)
    {
        case ModemProtocolCommands::GetSignalQuality:
        {
            command = ModemCmd::GetSignalQuality;
        }
        break;
        case ModemProtocolCommands::GetOperator:
        {
            command = ModemCmd::GetOperator;
        }
        break;
        case ModemProtocolCommands::GetBalance:
        {
            command = ModemCmd::GetBalance;
        }
        break;
        case ModemProtocolCommands::Identification:
        {
            command = ModemCmd::Identification;
        }
        break;
        case ModemProtocolCommands::OffEcho:
        {
            command = ModemCmd::OffEcho;
        }
        break;
        case ModemProtocolCommands::GetComment:
        {
            command = ModemCmd::GetComment;
        }
        break;
        case ModemProtocolCommands::GetSimNumber:
        {
            command = ModemCmd::GetSimNumber;
        }
        break;
        case ModemProtocolCommands::Reset:
        {
            command = ModemCmd::Reset;
        }
        break;
        case ModemProtocolCommands::CmdRestart:
        {
            command = ModemCmd::CmdRestart;
        }
        break;
        case ModemProtocolCommands::CmdSendSMS:
        {
            command = ModemCmd::CmdSendSms;
        }
        break;
        default:
        {
            command = ModemCmd::Uknown;
        }
    };
    if (thread)
    {
        this->start();
        return true;
    }
    else
    {
        bool respData;
        respData = this->e_Data_Execute();
        return respData;
    }
    return false;
}
