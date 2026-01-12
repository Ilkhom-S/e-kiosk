// Project
#include "ClassAcceptor.h"

QStringList acceptorList;

ClassAcceptor::ClassAcceptor(QObject *parent) : QThread(parent)
{
    acceptorList << AcceptorModel::CCTalk;
}

void ClassAcceptor::termanatedThread()
{
    //qDebug() << "******************************************";
    //qDebug() << "Validator command terminated";
    //qDebug() << "******************************************";
}

void ClassAcceptor::setValidator(QString name)
{
    validatorName = name;

    if (validatorName == AcceptorModel::CCTalk) {
        CCTalkAcceptor = new CCTalk();
        connect(CCTalkAcceptor,SIGNAL(emitNominal(int)),this,SIGNAL(eNominal(int)));
        connect(CCTalkAcceptor,SIGNAL(emitNominalDuplicate(int)),this,SIGNAL(eNominalDuplicate(int)));
        connect(CCTalkAcceptor,SIGNAL(emitAnimateStatus(bool)),this,SIGNAL(showHideDialogAnimate(bool)));
        connect(CCTalkAcceptor,SIGNAL(emitStatus(int,QString)),this,SLOT(getStatusFromAcceptor(int,QString)));
        connect(CCTalkAcceptor,SIGNAL(emitLoging(int,QString,QString)),SIGNAL(emitLoging(int,QString,QString)));
        connect(CCTalkAcceptor,SIGNAL(emitBillTable(QString)), this, SIGNAL(emitBillTable(QString)));
    }
}

void ClassAcceptor::getStatusFromAcceptor(int sts, QString comment)
{
    status = sts;

    emit emitStatusCoinAcceptor(status,comment);
}

void ClassAcceptor::setPortName(QString portName)
{
    comPort = portName;

    if (validatorName == AcceptorModel::CCTalk) {
        CCTalkAcceptor->setPortName(comPort);
    }
}

void ClassAcceptor::setPartNumber(QString partNumber){

    if (validatorName == AcceptorModel::CCTalk) {
        CCTalkAcceptor->setPartNumber(partNumber);
    }
}

void ClassAcceptor::setPortListInfo(QStringList port_list)
{
    portList = port_list;
}

bool ClassAcceptor::openPort()
{
    bool result = false;

    if(validatorName == AcceptorModel::CCTalk) {
        result = CCTalkAcceptor->OpenPort();
    }

    return result;
}

void ClassAcceptor::execCommand(int cmd)
{
    cmdExec = cmd;

    if (cmdExec == AcceptorCommands::StopPolling) {
        if (validatorName == AcceptorModel::CCTalk) {
            CCTalkAcceptor->stopPoll = true;
            return;
        }
    }

    this->start();
}

bool ClassAcceptor::pollState()
{
    bool pollState_in = false;

    if (validatorName == AcceptorModel::CCTalk) {
        if (!CCTalkAcceptor->stopPoll) {
            pollState_in = true;
        }
    }

    return pollState_in;
}

void ClassAcceptor::run()
{
    switch(cmdExec)
    {
        case AcceptorCommands::Restart :
            if(validatorName == AcceptorModel::CCTalk) {
                CCTalkAcceptor->CmdRestart();
            }
        break;

        case AcceptorCommands::StartPolling :
            if (validatorName == AcceptorModel::CCTalk) {
                CCTalkAcceptor->CmdStartPoll();
            }
        break;

        case AcceptorCommands::StopPolling :
            if (validatorName == AcceptorModel::CCTalk) {
                this->msleep(100);
                CCTalkAcceptor->CmdStopPoll();
            }
        break;

        case AcceptorCommands::Poll :
            if (validatorName == AcceptorModel::CCTalk) {
                CCTalkAcceptor->CmdGetStatus();
            }
        break;

        case AcceptorCommands::SetNominalTable :
            if (validatorName == AcceptorModel::CCTalk) {
                CCTalkAcceptor->CmdInit();
            }
        break;

        case AcceptorCommands::ComClear :
        break;
    }

    return;
}

bool ClassAcceptor::isItYou(QStringList &comList,QString &validator_name, QString &com_str, QString &validator_coment)
{
    if ((validator_name != "") && (com_str != "") && (com_str.contains("COM"))) {
        this->setValidator(validator_name);
        this->setPortName(com_str);

        if (validator_name == AcceptorModel::CCTalk) {
            if (CCTalkAcceptor->isItYou()) {
                nowValidatorName = validator_name;
                nowPortName      = com_str;
                nowComent        = validator_coment = CCTalkAcceptor->PartNumber;
                this->v_PartNumber   = CCTalkAcceptor->PartNumber;
                this->v_SerialNumber = CCTalkAcceptor->SerialNumber;
                CCTalkAcceptor->closePort();
                return true;
              }
         }
    }

    for (int dev_count = 0; dev_count < acceptorList.count(); dev_count++) {

        this->setValidator(acceptorList.at(dev_count));

        for (int com_count = 0; com_count < comList.count(); com_count++) {
            this->setPortName(comList.at(com_count));

            if (validatorName == AcceptorModel::CCTalk) {
                if (CCTalkAcceptor->isItYou()) {
                    nowValidatorName = validator_name = acceptorList.at(dev_count);
                    nowPortName      = com_str = comList.at(com_count);
                    nowComent        = validator_coment = CCTalkAcceptor->PartNumber;
                    this->v_PartNumber   = CCTalkAcceptor->PartNumber;
                    this->v_SerialNumber = CCTalkAcceptor->SerialNumber;
                    CCTalkAcceptor->closePort();
                    return true;
                }
            }
        }
    }

    return false;
}

bool ClassAcceptor::CIsItYou(QString &validat_name)
{
    if (validat_name == AcceptorModel::CCTalk) {
        if (CCTalkAcceptor->isItYou()) {
            this->v_PartNumber   = CCTalkAcceptor->PartNumber;
            this->v_SerialNumber = CCTalkAcceptor->SerialNumber;
            CCTalkAcceptor->closePort();
            return true;
        } else {
            return false;
        }
    }

    return false;
}

void ClassAcceptor::closeThis()
{
    if (validatorName == AcceptorModel::CCTalk) {
        CCTalkAcceptor->closePort();
    }
}
