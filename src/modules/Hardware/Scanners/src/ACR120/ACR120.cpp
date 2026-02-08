/* @file Кардридер ACS ACR120. */

// ACR120 SDK

#include <ACR120U/include/ACR120U.h>
#include <Hardware/Scanners/ACR120.h>

ACR120::ACR120() : m_Handle(0), m_CardPresent(false) {
    m_PollingInterval = 500;
}

//--------------------------------------------------------------------------------
bool ACR120::isConnected() {
    for (qint16 i = ACR120_USB1; i <= ACR120_USB8; ++i) {
        m_Handle = ACR120_Open(i);

        if (m_Handle > 0) {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------
bool ACR120::release() {
    bool result = TPollingHID::release();

    if (m_Handle) {
        ACR120_Close(m_Handle);

        m_Handle = 0;
    }

    return result;
}

//--------------------------------------------------------------------------------
bool ACR120::getStatus(TStatusCodes & /*aStatusCodes*/) {
    if (m_Handle <= 0) {
        return false;
    }

    quint8 tagFound;
    quint8 tagType[4];
    quint8 tagLength[4];
    quint8 tagSerialNumber[4][10];

    qint16 status = ACR120_ListTags(m_Handle, &tagFound, tagType, tagLength, tagSerialNumber);

    if (status == SUCCESS_READER_OP && tagFound && !m_CardPresent) {
        m_CardPresent = true;
        QByteArray rawData((const char *)(&tagSerialNumber[0][0]), tagLength[0]);

        QString receivedData = rawData.toHex().toUpper();

        if (!rawData.isEmpty()) {
            toLog(LogLevel::Normal,
                  QString("Scanner ACR120: data received: %1 (%2)")
                      .arg(QString(rawData))
                      .arg(receivedData));

            QVariantMap result;
            result[CHardwareSDK::HID::Text] = receivedData;

            emit data(result);
        }
    } else if (!tagFound && m_CardPresent) {
        m_CardPresent = false;
    }

    return true;
}

//--------------------------------------------------------------------------------
