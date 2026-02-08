#include "ClassValidator.h"

QStringList Validator_List;

ClassValidator::ClassValidator(QObject *parent) : QThread(parent) {
    Validator_List << ValidatorModel::CashCodeCCNET << ValidatorModel::MeiEBDS;
}

void ClassValidator::setValidator(QString name) {
    validatorName = name;

    if (validatorName == ValidatorModel::CashCodeCCNET) {
        CCNetValidator = new CCNetSm();
        connect(CCNetValidator, SIGNAL(emitNominal(int)), this, SIGNAL(eNominal(int)));
        connect(CCNetValidator,
                SIGNAL(emitNominalDuplicate(int)),
                this,
                SIGNAL(eNominalDuplicate(int)));
        connect(CCNetValidator,
                SIGNAL(emitAnimateStatus(bool)),
                this,
                SIGNAL(showHideDialogAnimate(bool)));
        connect(CCNetValidator,
                SIGNAL(emitReturnNominalStatus(bool)),
                this,
                SIGNAL(showHideDialogReturnNominal(bool)));
        connect(CCNetValidator,
                SIGNAL(emitStatus(int, QString)),
                this,
                SLOT(getStatusFrom_Validator(int, QString)));
        connect(CCNetValidator,
                SIGNAL(emitLog(int, QString, QString)),
                SIGNAL(emitLog(int, QString, QString)));
        connect(CCNetValidator,
                SIGNAL(emitValidatorLog(int, QByteArray, QString)),
                SIGNAL(emitValidatorLog(int, QByteArray, QString)));
        connect(CCNetValidator,
                SIGNAL(emitFirmwareUpdate(QString)),
                SIGNAL(emitFirmwareUpdate(QString)));
    }

    if (validatorName == ValidatorModel::MeiEBDS) {
        EBDSValidator = new EBDS();
        connect(EBDSValidator, SIGNAL(emitNominal(int)), this, SIGNAL(eNominal(int)));
        connect(
            EBDSValidator, SIGNAL(emitNominalDuplicate(int)), this, SIGNAL(eNominalDuplicate(int)));
        connect(EBDSValidator,
                SIGNAL(emitAnimateStatus(bool)),
                this,
                SIGNAL(showHideDialogAnimate(bool)));
        connect(EBDSValidator,
                SIGNAL(emitReturnNominalStatus(bool)),
                this,
                SIGNAL(showHideDialogReturnNominal(bool)));
        connect(EBDSValidator,
                SIGNAL(emitStatus(int, QString)),
                this,
                SLOT(getStatusFrom_Validator(int, QString)));
        connect(EBDSValidator,
                SIGNAL(emitLog(int, QString, QString)),
                SIGNAL(emitLog(int, QString, QString)));
        connect(EBDSValidator,
                SIGNAL(emitValidatorLog(int, QByteArray, QString)),
                SIGNAL(emitValidatorLog(int, QByteArray, QString)));
    }
}

void ClassValidator::getStatusFrom_Validator(int sts, QString comment) {
    status = sts;
    emit emitStatusValidator(status, comment);
}

void ClassValidator::setPortName(QString portName) {
    com_Port = portName;

    if (validatorName == ValidatorModel::CashCodeCCNET) {
        CCNetValidator->setPortName(com_Port);
    } else if (validatorName == ValidatorModel::MeiEBDS) {
        EBDSValidator->setPortName(com_Port);
    }
}

void ClassValidator::setPartNumber(QString partNumber) {

    if (validatorName == ValidatorModel::CashCodeCCNET) {
        CCNetValidator->setPartNumber(partNumber);
    } else if (validatorName == ValidatorModel::MeiEBDS) {
        EBDSValidator->setPartNumber(partNumber);
    }
}

void ClassValidator::setPortListInfo(QStringList portList) {
    portList = portList;
}

bool ClassValidator::openPort() {
    bool result = false;

    if (validatorName == ValidatorModel::CashCodeCCNET) {
        result = CCNetValidator->OpenPort();
    } else if (validatorName == ValidatorModel::MeiEBDS) {
        result = EBDSValidator->OpenPort();
    }

    return result;
}

void ClassValidator::execCommand(int cmd) {
    cmdExec = cmd;

    if (cmdExec == ValidatorCommands::StopPolling) {
        if (validatorName == ValidatorModel::CashCodeCCNET) {
            CCNetValidator->stopPoll = true;
            return;
        }

        if (validatorName == ValidatorModel::MeiEBDS) {
            EBDSValidator->stopPoll = true;
            return;
        }
    }

    if (cmd == ValidatorCommands::FirmwareUpdate) {
        msleep(1000);
    }

    this->start();
}

bool ClassValidator::pollState() {
    bool pollStateIn = false;

    if (validatorName == ValidatorModel::CashCodeCCNET) {
        if (!CCNetValidator->stopPoll) {
            pollStateIn = true;
        }
    } else if (validatorName == ValidatorModel::MeiEBDS) {
        if (!EBDSValidator->stopPoll) {
            pollStateIn = true;
        }
    }

    return pollStateIn;
}

void ClassValidator::run() {

    switch (cmdExec) {
    case ValidatorCommands::Restart:
        if (validatorName == ValidatorModel::CashCodeCCNET) {
            CCNetValidator->CmdRestart();
        } else if (validatorName == ValidatorModel::MeiEBDS) {
            EBDSValidator->CmdRestart();
        }
        break;

    case ValidatorCommands::StartPolling:
        if (validatorName == ValidatorModel::CashCodeCCNET) {
            CCNetValidator->maxSum_Reject = maxSum.value("max_sum_reject").toBool();
            CCNetValidator->maxSum = maxSum.value("max_sum").toInt();
            CCNetValidator->CmdStartPoll();
        } else if (validatorName == ValidatorModel::MeiEBDS) {
            EBDSValidator->maxSum_Reject = maxSum.value("max_sum_reject").toBool();
            EBDSValidator->maxSum = maxSum.value("max_sum").toInt();
            EBDSValidator->CmdStartPoll();
        }
        break;

    case ValidatorCommands::StopPolling:
        if (validatorName == ValidatorModel::CashCodeCCNET) {
            ClassValidator::msleep(20);
            CCNetValidator->CmdStopPoll();
        } else if (validatorName == ValidatorModel::MeiEBDS) {
            ClassValidator::msleep(20);
            EBDSValidator->CmdStopPoll();
        }
        break;

    case ValidatorCommands::Poll:
        if (validatorName == ValidatorModel::CashCodeCCNET) {
            CCNetValidator->CmdGetStatus();
        } else if (validatorName == ValidatorModel::MeiEBDS) {
            EBDSValidator->CmdGetStatus();
        }
        break;

    case ValidatorCommands::FirmwareUpdate:
        if (validatorName == ValidatorModel::CashCodeCCNET) {
            CCNetValidator->CmdFirmwareUpdate(firmwareVersion);
        }
        break;
    }
}

bool ClassValidator::isItYou(QStringList &comList,
                             QString &validatorName,
                             QString &comStr,
                             QString &validatorComent) {
    if ((validatorName != "") && (comStr != "") && (comStr.contains("COM"))) {
        this->setValidator(validatorName);
        this->setPortName(comStr);

        if (validatorName == ValidatorModel::CashCodeCCNET) {
            if (CCNetValidator->isItYou()) {
                nowValidatorName = validatorName;
                nowPortName = comStr;
                nowComent = validatorComent = CCNetValidator->PartNumber;
                this->vPartNumber = CCNetValidator->PartNumber;
                this->vSerialNumber = CCNetValidator->SerialNumber;
                CCNetValidator->closePort();
                return true;
            }
        }

        if (validatorName == ValidatorModel::MeiEBDS) {
            if (EBDSValidator->isItYou()) {
                nowValidatorName = validatorName;
                nowPortName = comStr;
                nowComent = validatorComent = EBDSValidator->partNumber;
                this->vPartNumber = EBDSValidator->partNumber;
                this->vSerialNumber = EBDSValidator->serialNumber;
                EBDSValidator->closePort();
                return true;
            }
        }
    }

    for (int devCount = 0; devCount < Validator_List.count(); devCount++) {

        this->setValidator(Validator_List.at(devCount));

        for (int comCount = 0; comCount < comList.count(); comCount++) {
            this->setPortName(comList.at(comCount));

            if (validatorName == ValidatorModel::CashCodeCCNET) {
                if (CCNetValidator->isItYou()) {
                    nowValidatorName = validatorName = Validator_List.at(devCount);
                    nowPortName = comStr = comList.at(comCount);
                    nowComent = validatorComent = CCNetValidator->PartNumber;
                    CCNetValidator->closePort();
                    return true;
                }
            }

            if (validatorName == ValidatorModel::MeiEBDS) {
                if (EBDSValidator->isItYou()) {
                    nowValidatorName = validatorName = Validator_List.at(devCount);
                    nowPortName = comStr = comList.at(comCount);
                    nowComent = validatorComent = EBDSValidator->partNumber;
                    EBDSValidator->closePort();
                    return true;
                }
            }
        }
    }

    return false;
}

bool ClassValidator::CIsItYou(QString &validatName) {
    if (validatName == ValidatorModel::CashCodeCCNET) {
        if (CCNetValidator->isItYou()) {
            this->vPartNumber = CCNetValidator->PartNumber;
            this->vSerialNumber = CCNetValidator->SerialNumber;
            CCNetValidator->closePort();
            return true;
        }
        return false;
    }

    if (validatName == ValidatorModel::MeiEBDS) {
        if (EBDSValidator->isItYou()) {
            this->vPartNumber = EBDSValidator->partNumber;
            this->vSerialNumber = EBDSValidator->serialNumber;
            EBDSValidator->closePort();
            return true;
        }
        return false;
    }

    return false;
}

void ClassValidator::setDBError(bool error) {
    if (validatorName == ValidatorModel::CashCodeCCNET) {
        CCNetValidator->hasDBError = error;
    } else if (validatorName == ValidatorModel::MeiEBDS) {
        EBDSValidator->hasDBError = error;
    }
}

void ClassValidator::closeThis() {
    if (validatorName == ValidatorModel::CashCodeCCNET) {
        CCNetValidator->closePort();
    } else if (validatorName == ValidatorModel::MeiEBDS) {
        EBDSValidator->closePort();
    }
}
