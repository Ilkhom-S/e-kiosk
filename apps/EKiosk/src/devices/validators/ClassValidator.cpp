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

void ClassValidator::setPortListInfo(QStringList port_list) {
    portList = port_list;
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
    bool pollState_in = false;

    if (validatorName == ValidatorModel::CashCodeCCNET) {
        if (!CCNetValidator->stopPoll) {
            pollState_in = true;
        }
    } else if (validatorName == ValidatorModel::MeiEBDS) {
        if (!EBDSValidator->stopPoll) {
            pollState_in = true;
        }
    }

    return pollState_in;
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
            this->msleep(20);
            CCNetValidator->CmdStopPoll();
        } else if (validatorName == ValidatorModel::MeiEBDS) {
            this->msleep(20);
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
    return;
}

bool ClassValidator::isItYou(QStringList &com_List,
                             QString &validator_name,
                             QString &com_str,
                             QString &validator_coment) {
    if ((validator_name != "") && (com_str != "") && (com_str.contains("COM"))) {
        this->setValidator(validator_name);
        this->setPortName(com_str);

        if (validator_name == ValidatorModel::CashCodeCCNET) {
            if (CCNetValidator->isItYou()) {
                nowValidatorName = validator_name;
                nowPortName = com_str;
                nowComent = validator_coment = CCNetValidator->PartNumber;
                this->vPartNumber = CCNetValidator->PartNumber;
                this->vSerialNumber = CCNetValidator->SerialNumber;
                CCNetValidator->closePort();
                return true;
            }
        }

        if (validator_name == ValidatorModel::MeiEBDS) {
            if (EBDSValidator->isItYou()) {
                nowValidatorName = validator_name;
                nowPortName = com_str;
                nowComent = validator_coment = EBDSValidator->partNumber;
                this->vPartNumber = EBDSValidator->partNumber;
                this->vSerialNumber = EBDSValidator->serialNumber;
                EBDSValidator->closePort();
                return true;
            }
        }
    }

    for (int dev_count = 0; dev_count < Validator_List.count(); dev_count++) {

        this->setValidator(Validator_List.at(dev_count));

        for (int com_count = 0; com_count < com_List.count(); com_count++) {
            this->setPortName(com_List.at(com_count));

            if (validatorName == ValidatorModel::CashCodeCCNET) {
                if (CCNetValidator->isItYou()) {
                    nowValidatorName = validator_name = Validator_List.at(dev_count);
                    nowPortName = com_str = com_List.at(com_count);
                    nowComent = validator_coment = CCNetValidator->PartNumber;
                    CCNetValidator->closePort();
                    return true;
                }
            }

            if (validatorName == ValidatorModel::MeiEBDS) {
                if (EBDSValidator->isItYou()) {
                    nowValidatorName = validator_name = Validator_List.at(dev_count);
                    nowPortName = com_str = com_List.at(com_count);
                    nowComent = validator_coment = EBDSValidator->partNumber;
                    EBDSValidator->closePort();
                    return true;
                }
            }
        }
    }

    return false;
}

bool ClassValidator::CIsItYou(QString &validat_name) {
    if (validat_name == ValidatorModel::CashCodeCCNET) {
        if (CCNetValidator->isItYou()) {
            this->vPartNumber = CCNetValidator->PartNumber;
            this->vSerialNumber = CCNetValidator->SerialNumber;
            CCNetValidator->closePort();
            return true;
        } else {
            return false;
        }
    }

    if (validat_name == ValidatorModel::MeiEBDS) {
        if (EBDSValidator->isItYou()) {
            this->vPartNumber = EBDSValidator->partNumber;
            this->vSerialNumber = EBDSValidator->serialNumber;
            EBDSValidator->closePort();
            return true;
        } else {
            return false;
        }
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
