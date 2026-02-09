/* @file Базовый класс устройства приема денег на протоколе ccTalk. */

#include "CCTalkDeviceBase.h"

#include <QtCore/QRegularExpression>

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
template <class T>
CCTalkDeviceBase<T>::CCTalkDeviceBase()
    : m_EventIndex(0), m_FWVersion(0), m_Address(CCCTalk::Address::Unknown) {
    // данные устройства
    this->m_DeviceName = CCCTalk::DefaultDeviceName;
    this->m_ProtocolTypes = getProtocolTypes();
}

//---------------------------------------------------------------------------
template <class T> QStringList CCTalkDeviceBase<T>::getProtocolTypes() {
    return CCCTalk::ProtocolTypes;
}

//--------------------------------------------------------------------------------
template <class T> QDate CCTalkDeviceBase<T>::parseDate(const QByteArray &aData) {
    ushort data = qToBigEndian(aData.toHex().toUShort(nullptr, 16));

    return {int((data >> 9) & 0x3F) + this->m_BaseYear,
            int((data >> 5) & 0x0F),
            int((data >> 0) & 0x1F)};
}

//--------------------------------------------------------------------------------
template <class T>
TResult CCTalkDeviceBase<T>::execCommand(const QByteArray &aCommand,
                                         const QByteArray &aCommandData,
                                         QByteArray *aAnswer) {
    MutexLocker locker(&(this->m_ExternalMutex));

    char command = aCommand[0];
    CCCTalk::Command::SData commandData = CCCTalk::Command::Description[command];

    if (m_ModelData.unsupported.contains(command)) {
        this->toLog(LogLevel::Warning,
                    this->m_DeviceName + ": does not support command " + commandData.description);
        return CommandResult::Driver;
    }

    this->m_Protocol.setLog(this->m_Log);
    this->m_Protocol.setPort(this->m_IOPort);
    this->m_Protocol.setAddress(this->m_Address);

    if (!this->isAutoDetecting()) {
        QString protocolType = this->getConfigParameter(CHardware::ProtocolType).toString();
        this->m_Protocol.setType(protocolType);
    }

    QByteArray answer;
    TResult result = this->m_Protocol.processCommand(aCommand + aCommandData, answer);

    if (!result) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": Failed to " + commandData.description);
        return result;
    }

    bool ack = (answer.size() >= 1) && (answer == QByteArray(answer.size(), CCCTalk::ACK));

    if ((commandData.type == CCCTalk::Command::EAnswerType::ACK) && !ack) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName +
                        QString(": Failed to check answer = {%1}, need ack").arg(commandData.size));

        return CommandResult::Answer;
    }

    if (((commandData.type == CCCTalk::Command::EAnswerType::Data) ||
         (commandData.type == CCCTalk::Command::EAnswerType::Date)) &&
        (commandData.size >= answer.size())) {
        this->toLog(LogLevel::Error,
                    this->m_DeviceName +
                        QString(": Failed to check answer size = %1, need minimum = %2")
                            .arg(answer.size())
                            .arg(commandData.size));
        return CommandResult::Answer;
    }

    if (aAnswer) {
        *aAnswer = answer.mid(1);
    }

    return CommandResult::OK;
}

//--------------------------------------------------------------------------------
template <class T> bool CCTalkDeviceBase<T>::isConnected() {
    if (!this->isAutoDetecting()) {
        return checkConnection();
    }

    foreach (auto protocolType, this->m_ProtocolTypes) {
        this->m_Protocol.setType(protocolType);
        this->setConfigParameter(CHardware::ProtocolType, protocolType);

        if (checkConnection()) {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------
template <class T> bool CCTalkDeviceBase<T>::checkConnection() {
    SleepHelper::msleep(CCCTalk::IdentificationPause);

    QByteArray answer;

    if (!this->processCommand(CCCTalk::Command::DeviceTypeID, &answer)) {
        return false;
    } else {
        QString answerData = ProtocolUtils::clean(answer).replace(ASCII::Space, "").toLower();
        QString data = CCCTalk::DeviceTypeIds[this->m_Address];
        if (!answerData.contains(data)) {
            this->toLog(
                LogLevel::Error,
                this->m_DeviceName +
                    QString(": wrong device type = %1, need like %2").arg(answer.data()).arg(data));
            return false;
        }
    }

    QByteArray vendorID;

    if (!this->processCommand(CCCTalk::Command::VendorID, &vendorID)) {
        return false;
    }

    if (CCCTalk::VendorData.data().contains(vendorID.toUpper())) {
        this->setDeviceParameter(CDeviceData::Vendor, CCCTalk::VendorData.getName(vendorID));
    }

    QByteArray productCode;

    if (!this->processCommand(CCCTalk::Command::ProductCode, &productCode)) {
        return false;
    }

    if (!productCode.isEmpty()) {
        this->setDeviceParameter(CDeviceData::ProductCode, productCode.simplified());
    }

    if (!m_AllModelData) {
        this->toLog(LogLevel::Error, this->m_DeviceName + ": No model data");
        return false;
    }

    this->m_DeviceName = this->m_AllModelData->getData(vendorID, productCode, this->m_ModelData);
    this->m_Verified = this->m_ModelData.verified;
    this->m_ModelCompatibility = this->m_Models.contains(this->m_DeviceName);

    return true;
}

//--------------------------------------------------------------------------------
template <class T> void CCTalkDeviceBase<T>::processDeviceData() {
    QByteArray answer;

    if (this->processCommand(CCCTalk::Command::BuildCode, &answer)) {
        this->setDeviceParameter(CDeviceData::Build, answer);
    }

    if (this->processCommand(CCCTalk::Command::ProtocolID, &answer)) {
        this->setDeviceParameter(
            CDeviceData::ProtocolVersion,
            QString("%1.%2.%3").arg(uchar(answer[0])).arg(uchar(answer[1])).arg(uchar(answer[2])));
    }

    if (this->processCommand(CCCTalk::Command::Serial, &answer)) {
        this->setDeviceParameter(CDeviceData::SerialNumber,
                                 0x10000 * uchar(answer[2]) + 0x100 * uchar(answer[1]) +
                                     uchar(answer[0]));
    }

    if (this->processCommand(CCCTalk::Command::DBVersion, &answer)) {
        this->setDeviceParameter(CDeviceData::CashAcceptors::Database, uchar(answer[0]));
    }

    this->removeDeviceParameter(CDeviceData::Firmware);

    if (this->processCommand(CCCTalk::Command::SoftVersion, &answer)) {
        this->setDeviceParameter(CDeviceData::Firmware, answer);
        double FWVersion = this->parseFWVersion(answer);

        if (FWVersion) {
            this->m_FWVersion = FWVersion;

            if (answer.simplified().toDouble() != FWVersion) {
                this->setDeviceParameter(
                    CDeviceData::Version, this->m_FWVersion, CDeviceData::Firmware);
            }
        }
    }

    if (this->processCommand(CCCTalk::Command::BaseYear, &answer)) {
        this->m_BaseYear = answer.toInt();
    }

    if (this->processCommand(CCCTalk::Command::CreationDate, &answer)) {
        this->setDeviceParameter(CDeviceData::Date,
                                 this->parseDate(answer).toString("dd.MM.yyyy"),
                                 CDeviceData::Firmware);
    }

    if (this->processCommand(CCCTalk::Command::SoftLastDate, &answer)) {
        this->setDeviceParameter(CDeviceData::CashAcceptors::LastUpdate,
                                 this->parseDate(answer).toString("dd.MM.yyyy"),
                                 CDeviceData::Firmware);
    }
}

//--------------------------------------------------------------------------------
template <class T> double CCTalkDeviceBase<T>::parseFWVersion(const QByteArray &aAnswer) {
    double result = 0.0;

    // 1. Используем QStringLiteral для оптимизации создания регулярного выражения.
    // Это предотвращает аллокации в куче при каждом вызове метода.
    QRegularExpression regex(QStringLiteral("\\d+\\.\\d+"));

    // 2. Выполняем сопоставление. Метод match() автоматически
    // интерпретирует QByteArray как UTF-8 строку.
    QRegularExpressionMatch match = regex.match(QString::fromUtf8(aAnswer));

    // 3. Проверяем наличие совпадения через hasMatch() (стандарт Qt 6)
    if (match.hasMatch()) {
        // 4. Извлекаем группу 0 (все совпадение целиком) через captured(0).
        // Это быстрее, чем формирование QStringList через capturedTexts().
        result = match.captured(0).toDouble();
    }

    return result;
}

//--------------------------------------------------------------------------------
