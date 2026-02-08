/* @file ФР семейства АТОЛ на COM-порту. */

#include "AtolSerialFR.h"

#include "AtolFRBaseConstants.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
AtolSerialFR::AtolSerialFR() {
    /// Команды получения информации об устройстве.
    using namespace CAtolFRBase::Commands;

    // регистры
    m_RegisterData.add(CAtolFR::Registers::NonNullableAmount, '\x0D', 7);
    m_RegisterData.add(CAtolFR::Registers::SerialNumber, '\x16', 4);
    m_RegisterData.add(CAtolFRBase::Registers::FreeReregistrations, '\x0F', 1);
    m_RegisterData.add(CAtolFRBase::Registers::FreeFMSessions, '\x10', 2);
    m_RegisterData.add(CAtolFRBase::Registers::EKLZActivizationResources, '\x1A', 2);
    m_RegisterData.add(CAtolFRBase::Registers::EKLZRegistrationData, '\x1B', 16);
    m_RegisterData.add(CAtolFRBase::Registers::EKLZActivizationData, '\x1C', 10);

    // команды
    m_CommandData.add(EKLZRequest, 6 * 1000, true);
    m_CommandData.add(GetSoftEKLZStatus, 6 * 1000, true);
    m_CommandData.add(PrintDeferredZReports, 120 * 1000);
    m_CommandData.add(EnterDeferredZReportMode, true, true);

    setConfigParameter(CHardware::FR::Commands::PrintingDeferredZReports, PrintDeferredZReports);

    // ошибки
    m_ErrorData = PErrorData(new CAtolFRBase::Errors::Data());
}

//--------------------------------------------------------------------------------
bool AtolSerialFR::checkTax(TVAT aVAT, CFR::Taxes::SData &aData) {
    if (!checkTaxValue(aVAT, aData, CAtolFR::FRParameters::Tax, true)) {
        return false;
    }

    if ((m_DeviceName == CAtolFR::Models::FPrint11PTK) ||
        (m_DeviceName == CAtolFR::Models::FPrint22K) ||
        (m_DeviceName == CAtolFR::Models::FPrint55K)) {
        CAtolFR::FRParameters::SData taxFRParameterData =
            CAtolFR::FRParameters::TaxDescription(aData.group);
        QByteArray rawDescription;

        if (getFRParameter(taxFRParameterData, rawDescription)) {
            QString description = aData.description;
            QString FRDescription =
                m_Codec->toUnicode(rawDescription.replace(ASCII::NUL, "").simplified());

            if ((description != FRDescription) &&
                !setFRParameter(
                    taxFRParameterData,
                    description.leftJustified(m_ModelData.maxStringSize, QChar(ASCII::Space)))) {
                toLog(LogLevel::Error,
                      m_DeviceName +
                          QString(": Failed to set description for tax value %1% (%2 tax group)")
                              .arg(aVAT, 5, 'f', 2, ASCII::Zero)
                              .arg(aData.group));
            }
        } else {
            toLog(LogLevel::Error,
                  m_DeviceName +
                      QString(": Failed to get tax value for %1 tax group").arg(aData.group));
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool AtolSerialFR::getCommonStatus(TStatusCodes &aStatusCodes) {
    if (!AtolFRBase::getCommonStatus(aStatusCodes)) {
        return false;
    }

    getEKLZStatus(aStatusCodes);

    return true;
}

//--------------------------------------------------------------------------------
bool AtolSerialFR::getShortStatus(TStatusCodes &aStatusCodes) {
    if (!AtolFRBase::getShortStatus(aStatusCodes)) {
        return false;
    }

    m_EKLZError = m_EKLZError || ((m_Mode == CAtolFR::InnerModes::ExtraCommand) &&
                                (m_Submode == CAtolFR::InnerSubmodes::EKLZError));
    m_FMError = m_FMError || ((m_Mode == CAtolFR::InnerModes::ExtraCommand) &&
                            ((m_Submode == CAtolFR::InnerSubmodes::EnterDate) ||
                             (m_Submode == CAtolFR::InnerSubmodes::EnterTime) ||
                             (m_Submode == CAtolFR::InnerSubmodes::FMDataTimeError) ||
                             (m_Submode == CAtolFR::InnerSubmodes::FMDataTimeConfirm)));

    return true;
}

//--------------------------------------------------------------------------------
void AtolSerialFR::processDeviceData() {
    AtolFRBase::processDeviceData();

    CAtolFR::SSoftInfo softInfo;
    removeDeviceParameter(CDeviceData::FM::Firmware);

    if (getSoftVersion(CAtolFR::FRSubSystems::FM, softInfo)) {
        setDeviceParameter(CDeviceData::Version, softInfo.version, CDeviceData::FM::Firmware);
        setDeviceParameter(CDeviceData::Build, softInfo.build, CDeviceData::FM::Firmware);
    }

    QByteArray data;

    m_NonNullableAmount = 0;

    if (getRegister(CAtolFR::Registers::NonNullableAmount, data)) {
        m_NonNullableAmount = qlonglong(data.toHex().toULongLong()) / 100.0;
    }

    if (getRegister(CAtolFRBase::Registers::EKLZRegistrationData, data)) {
        m_INN = CFR::INNToString(data.mid(0, 6).toHex());
        m_RNM = CFR::RNMToString(data.mid(6, 5).toHex());
        m_Fiscalized = m_INN.toULongLong() && m_RNM.toULongLong();
    }

    bool EKLZExists = m_ModelData.FRType == EFRType::EKLZ;
    setDeviceParameter(CDeviceData::FR::EKLZ, EKLZExists);

    if (EKLZExists) {
        // Вытаскиваем инфо из системых регистров
        setDeviceParameter(CDeviceData::FR::Activated, m_Fiscalized, CDeviceData::FR::EKLZ);

        if (getRegister(CAtolFRBase::Registers::FreeReregistrations, data)) {
            setDeviceParameter(CDeviceData::FR::FreeReregistrations, uchar(data[0]));
        }

        if (getRegister(CAtolFRBase::Registers::EKLZActivizationResources, data)) {
            setDeviceParameter(CDeviceData::EKLZ::FreeActivizations, uchar(data[1]));
        }

        if ((m_ModelData.FRType == EFRType::EKLZ) && m_Fiscalized &&
            getRegister(CAtolFRBase::Registers::EKLZActivizationData, data)) {
            qlonglong EKLZNumber = data.left(5).toHex().toULongLong();
            QDate EKLZActivizationDate =
                QDate::fromString(data.mid(5, 3).toHex().insert(4, "20"), CFR::DateFormat);

            setDeviceParameter(CDeviceData::EKLZ::Serial, EKLZNumber);
            setDeviceParameter(CDeviceData::EKLZ::ActivizationDate,
                               EKLZActivizationDate.toString(CFR::DateLogFormat));
        }

        QByteArray commandData(1, CAtolFRBase::EKLZRequests::GetRegNumber);
        QByteArray answer;

        if (canPerformEKLZRequest() &&
            processCommand(CAtolFRBase::Commands::EKLZRequest, commandData, &answer)) {
            qlonglong EKLZNumber = answer.mid(3).toHex().toULongLong();
            setDeviceParameter(CDeviceData::EKLZ::Serial, EKLZNumber);
        }
    }

    if (getRegister(CAtolFRBase::Registers::FreeFMSessions, data)) {
        setDeviceParameter(CDeviceData::FM::FreeSessions, data.toHex().toUShort());
    }
}

//--------------------------------------------------------------------------------
bool AtolSerialFR::enterExtendedMode() {
    if (!m_CanProcessZBuffer) {
        return true;
    }

    char mode = m_Mode;

    if (!enterInnerMode(CAtolFR::InnerModes::Cancel)) {
        toLog(
            LogLevel::Error,
            "AtolFR: Failed to enter to extended Z-report mode due to cannot enter to cancel mode");
        return false;
    }

    QByteArray answer;
    bool result = processCommand(CAtolFRBase::Commands::EnterDeferredZReportMode, &answer);

    if (result) {
        m_WhiteSpaceZBuffer = uchar(answer[2]);
        checkZBufferState();

        toLog(LogLevel::Normal,
              QString("AtolFR: %1 Z-reports left in buffer").arg(m_WhiteSpaceZBuffer));
    } else {
        toLog(LogLevel::Error, "AtolFR: Failed to enter to extended Z-report mode");
    }

    enterInnerMode(mode);

    return result;
}

//--------------------------------------------------------------------------------
CAtolFR::TModelKey AtolSerialFR::getModelKey(const QByteArray &aAnswer) {
    QString deviceName = aAnswer.mid(11).simplified();
    char modelPostfix = deviceName[deviceName.size() - 1].cell();
    EFRType::Enum type =
        CAtolFR::EKLZPostfixes.contains(modelPostfix) ? EFRType::EKLZ : EFRType::NoEKLZ;
    int modelNumber = uchar(aAnswer[3]);

    toLog(LogLevel::Normal,
          QString("AtolFR: model number = %1, EKLZ is %2available")
              .arg(modelNumber)
              .arg((type == EFRType::EKLZ) ? "" : "not "));

    return CAtolFR::TModelKey(modelNumber, type);
}

//--------------------------------------------------------------------------------
TResult
AtolSerialFR::performCommand(const QByteArray &aCommandData, QByteArray &aAnswer, int aTimeout) {
    TResult result = AtolFRBase::performCommand(aCommandData, aAnswer, aTimeout);

    if (result == CommandResult::Transport) // m_PollingActive??
    {
        m_EKLZError = false;
    }

    return result;
}

//--------------------------------------------------------------------------------
bool AtolSerialFR::processAnswer(const QByteArray &aCommand, char aError) {
    if (isEKLZErrorCritical(aError)) {
        // ФР тяжело переживает отказ ЭКЛЗ и может некоторое время не захотеть общаться вообще
        SleepHelper::msleep(CAtolFRBase::Timeouts::EKLZErrorPause);
    }

    if ((aCommand[0] == CAtolFRBase::Commands::PrintDeferredZReports) &&
        (aError == CAtolFR::Errors::BadModeForCommand) && (m_Mode != CAtolFR::InnerModes::Cancel)) {
        return enterInnerMode(CAtolFR::InnerModes::Cancel);
    }

    return AtolFRBase::processAnswer(aCommand, aError);
}

//--------------------------------------------------------------------------------
void AtolSerialFR::getEKLZStatus(TStatusCodes &aStatusCodes) {
    if (!canPerformEKLZRequest()) {
        return;
    }

    QByteArray commandData(1, CAtolFRBase::EKLZRequests::GetStatus);
    commandData.append(CAtolFRBase::SessionResume);
    QByteArray answer;

    if (processCommand(CAtolFRBase::Commands::EKLZRequest, commandData, &answer)) {
        if (isEKLZErrorCritical(answer[2], true)) {
            aStatusCodes.insert(FRStatusCode::Error::EKLZ);
        }

        if (answer[2] == CAtolFRBase::Errors::EKLZ::NearEnd) {
            aStatusCodes.insert(FRStatusCode::Warning::EKLZNearEnd);
        }
    } else if (getError(CAtolFRBase::Commands::EKLZRequest, answer) != CAtolFR::Errors::NoPaper) {
        aStatusCodes.insert(FRStatusCode::Error::EKLZ);
    }
}

//--------------------------------------------------------------------------------
void AtolSerialFR::setErrorFlags(const QByteArray &aCommand, char aError) {
    AtolFRBase::setErrorFlags(aCommand, aError);

    if (isEKLZErrorCritical(aError)) {
        m_EKLZError = true;
    }
}

//--------------------------------------------------------------------------------
bool AtolSerialFR::canPerformEKLZRequest() {
    if (m_StatusCollection.isEmpty() ||
        m_StatusCollection.contains(DeviceStatusCode::Error::NotAvailable) || m_BadAnswerCounter) {
        processCommand(CAtolFRBase::Commands::GetSoftEKLZStatus);
    }

    return (m_ModelData.FRType == EFRType::EKLZ) && (m_Mode == CAtolFR::InnerModes::Choice) &&
           !m_EKLZError;
}

//--------------------------------------------------------------------------------
bool AtolSerialFR::isEKLZErrorCritical(const char aError, bool aIsDirectRequest) const {
    return aIsDirectRequest ? (aError == CAtolFRBase::Errors::EKLZ::Error) ||
                                  (aError == CAtolFRBase::Errors::EKLZ::CCError) ||
                                  (aError == CAtolFRBase::Errors::EKLZ::End)
                            :
                            //------------------------------------------
               (aError == CAtolFRBase::Errors::I2CInterface) ||
                   (aError == CAtolFRBase::Errors::EKLZOverflow);
}

//--------------------------------------------------------------------------------
