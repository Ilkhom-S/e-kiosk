#include "ClassAcceptor.h"

QStringList acceptorList;

ClassAcceptor::ClassAcceptor(QObject *parent) : QThread(parent) {
    acceptorList << AcceptorModel::CCTalk;
}

void ClassAcceptor::termanatedThread() {
    // qDebug() << "******************************************";
    // qDebug() << "Validator command terminated";
    // qDebug() << "******************************************";
}

void ClassAcceptor::setValidator(QString name) {
    validatorName = name;

    if (validatorName == AcceptorModel::CCTalk) {
        CCTalkAcceptor = new CCTalk();
        connect(CCTalkAcceptor, SIGNAL(emitNominal(int)), this, SIGNAL(eNominal(int)));
        connect(CCTalkAcceptor,
                SIGNAL(emitNominalDuplicate(int)),
                this,
                SIGNAL(eNominalDuplicate(int)));
        connect(CCTalkAcceptor,
                SIGNAL(emitAnimateStatus(bool)),
                this,
                SIGNAL(showHideDialogAnimate(bool)));
        connect(CCTalkAcceptor,
                SIGNAL(emitStatus(int, QString)),
                this,
                SLOT(getStatusFrom_Acceptor(int, QString)));
        connect(CCTalkAcceptor,
                SIGNAL(emitLoging(int, QString, QString)),
                SIGNAL(emitLoging(int, QString, QString)));
        connect(
            CCTalkAcceptor, SIGNAL(emitBillTable(QString)), this, SIGNAL(emitBillTable(QString)));
    }
}

void ClassAcceptor::getStatusFrom_Acceptor(int sts, QString comment) {
    status = sts;

    emit emitStatusCoinAcceptor(status, comment);
}

void ClassAcceptor::setPortName(QString portName) {
    com_Port = portName;

    if (validatorName == AcceptorModel::CCTalk) {
        CCTalkAcceptor->setPortName(com_Port);
    }
}

void ClassAcceptor::setPartNumber(QString partNumber) {

    if (validatorName == AcceptorModel::CCTalk) {
        CCTalkAcceptor->setPartNumber(partNumber);
    }
}

void ClassAcceptor::setPortListInfo(QStringList portList) {
    portList = portList;
}

bool ClassAcceptor::openPort() {
    bool result = false;

    if (validatorName == AcceptorModel::CCTalk) {
        result = CCTalkAcceptor->OpenPort();
    }

    return result;
}

void ClassAcceptor::execCommand(int cmd) {
    cmdExec = cmd;

    if (cmdExec == AcceptorCommands::StopPolling) {
        if (validatorName == AcceptorModel::CCTalk) {
            CCTalkAcceptor->stopPoll = true;
            return;
        }
    }

    this->start();
}

bool ClassAcceptor::pollState() {
    bool pollStateIn = false;

    if (validatorName == AcceptorModel::CCTalk) {
        if (!CCTalkAcceptor->stopPoll) {
            pollStateIn = true;
        }
    }

    return pollStateIn;
}

void ClassAcceptor::run() {
    switch (cmdExec) {
    case AcceptorCommands::Restart:
        if (validatorName == AcceptorModel::CCTalk) {
            CCTalk::CmdRestart();
        }
        break;

    case AcceptorCommands::StartPolling:
        if (validatorName == AcceptorModel::CCTalk) {
            CCTalkAcceptor->CmdStartPoll();
        }
        break;

    case AcceptorCommands::StopPolling:
        if (validatorName == AcceptorModel::CCTalk) {
            ClassAcceptor::msleep(100);
            CCTalkAcceptor->CmdStopPoll();
        }
        break;

    case AcceptorCommands::Poll:
        if (validatorName == AcceptorModel::CCTalk) {
            CCTalkAcceptor->CmdGetStatus();
        }
        break;

    case AcceptorCommands::SetNominalTable:
        if (validatorName == AcceptorModel::CCTalk) {
            CCTalkAcceptor->CmdInit();
        }
        break;

    case AcceptorCommands::Com_Clear:
        break;
    }
}

bool ClassAcceptor::isItYou(QStringList &comList,
                            QString &validatorName,
                            QString &comStr,
                            QString &validatorComent) {
    if ((validatorName != "") && (comStr != "") && (comStr.contains("COM"))) {
        this->setValidator(validatorName);
        this->setPortName(comStr);

        if (validatorName == AcceptorModel::CCTalk) {
            if (CCTalkAcceptor->isItYou()) {
                nowValidatorName = validatorName;
                nowPortName = comStr;
                nowComent = validatorComent = CCTalkAcceptor->PartNumber;
                this->v_PartNumber = CCTalkAcceptor->PartNumber;
                this->v_SerialNumber = CCTalkAcceptor->SerialNumber;
                CCTalkAcceptor->closePort();
                return true;
            }
        }
    }

    for (int devCount = 0; devCount < acceptorList.count(); devCount++) {

        this->setValidator(acceptorList.at(devCount));

        for (int comCount = 0; comCount < comList.count(); comCount++) {
            this->setPortName(comList.at(comCount));

            if (validatorName == AcceptorModel::CCTalk) {
                if (CCTalkAcceptor->isItYou()) {
                    nowValidatorName = validatorName = acceptorList.at(devCount);
                    nowPortName = comStr = comList.at(comCount);
                    nowComent = validatorComent = CCTalkAcceptor->PartNumber;
                    this->v_PartNumber = CCTalkAcceptor->PartNumber;
                    this->v_SerialNumber = CCTalkAcceptor->SerialNumber;
                    CCTalkAcceptor->closePort();
                    return true;
                }
            }
        }
    }

    return false;
}

bool ClassAcceptor::CIsItYou(QString &validatName) {
    if (validatName == AcceptorModel::CCTalk) {
        if (CCTalkAcceptor->isItYou()) {
            this->v_PartNumber = CCTalkAcceptor->PartNumber;
            this->v_SerialNumber = CCTalkAcceptor->SerialNumber;
            CCTalkAcceptor->closePort();
            return true;
        }
        return false;
    }

    return false;
}

void ClassAcceptor::closeThis() {
    if (validatorName == AcceptorModel::CCTalk) {
        CCTalkAcceptor->closePort();
    }
}
