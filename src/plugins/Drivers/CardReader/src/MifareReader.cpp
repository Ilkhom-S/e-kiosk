/* @file Интерфейс драйвера PC/SC ридера. */

#include "MifareReader.h"

#include "Hardware/CardReaders/CardReaderStatusesDescriptions.h"
#include "Hardware/Common/USBDeviceModelData.h"
#include "MifareReaderModelData.h"

using namespace SDK::Driver;

//------------------------------------------------------------------------------
namespace CMifareReader {
const char SAM2Header[] = "\x04\x01\x01";
} // namespace CMifareReader

//------------------------------------------------------------------------------
MifareReader::MifareReader() : m_Ready(false) {
    // данные USB-функционала
    m_DetectingData->set(CUSBVendors::ACS, CMifareReader::DetectingData().getProductData());

    m_PDODetecting = true;
    m_PortUsing = false;

    // данные устройства
    m_DeviceName = CMifareReader::UnknownModel;
    m_Reader.setLog(m_Log);
    m_StatusCodesSpecification =
        DeviceStatusCode::PSpecifications(new CardReaderStatusCode::CSpecifications());

    m_PollingInterval = 500;
}

//--------------------------------------------------------------------------------
QStringList MifareReader::getModelList() {
    return CMifareReader::DetectingData().getModelList(CUSBVendors::ACS);
}

//------------------------------------------------------------------------------
bool MifareReader::release() {
    m_Reader.disconnect(true);

    return TMifareReader::release();
}

//------------------------------------------------------------------------------
bool MifareReader::getStatus(TStatusCodes &aStatusCodes) {
    QStringList readerList = m_Reader.getReaderList();
    m_Ready =
        std::find_if(readerList.begin(), readerList.end(), [&](const QString &system_Name) -> bool {
            return m_Reader.connect(system_Name);
        }) != readerList.end();

    if (!m_Ready) {
        return false;
    }

    QByteArray answer;

    if (!m_Reader.communicate(CMifareReader::GetVersionRequest, answer) || (answer.size() < 30) ||
        !answer.startsWith(CMifareReader::SAM2Header)) {
        aStatusCodes.insert(CardReaderStatusCode::Error::SAM);
    }

    aStatusCodes.unite(m_Reader.getStatusCodes());
    m_Ready = std::find_if(aStatusCodes.begin(), aStatusCodes.end(), [&](int aStatusCode) -> bool {
                 return m_StatusCodesSpecification->value(aStatusCode).warningLevel ==
                        EWarningLevel::Error;
             }) == aStatusCodes.end();

    return true;
}

//------------------------------------------------------------------------------
bool MifareReader::isDeviceReady() const {
    return m_Ready;
}

//------------------------------------------------------------------------------
void MifareReader::eject() {
    m_Reader.disconnect(true);
}

//------------------------------------------------------------------------------
bool MifareReader::reset(QByteArray &aAnswer) {
    return m_Reader.reset(aAnswer);
}

//------------------------------------------------------------------------------
bool MifareReader::communicate(const QByteArray &aSendMessage, QByteArray &aReceiveMessage) {
    return m_Reader.communicate(aSendMessage, aReceiveMessage);
}

//------------------------------------------------------------------------------
