/* @file Реализация интерфейса работы с PC/SC API. */

#include "PCSCReader.h"

#include "Hardware/Protocols/Common/ProtocolUtils.h"
#include "PCSCReaderStatusCodes.h"
#include "SysUtils/ISysUtils.h"

#define CHECK_SCARD_ERROR(aFunctionName, ...)                                                      \
    handleResult(#aFunctionName, aFunctionName(__VA_ARGS__))

//--------------------------------------------------------------------------------
PCSCReader::PCSCReader() : m_Context(0), m_Card(0), m_ActiveProtocol(SCARD_PROTOCOL_UNDEFINED) {
    CHECK_SCARD_ERROR(SCardEstablishContext, SCARD_SCOPE_SYSTEM, NULL, NULL, &m_Context);
}

//--------------------------------------------------------------------------------
PCSCReader::~PCSCReader() {
    disconnect(false);

    if (m_Context) {
        CHECK_SCARD_ERROR(SCardReleaseContext, m_Context);
        m_Context = 0;
    }
}

//--------------------------------------------------------------------------------
bool PCSCReader::reset(QByteArray & /*aAnswer*/) {
    if (!m_Card) {
        return false;
    }

    return CHECK_SCARD_ERROR(SCardReconnect,
                             m_Card,
                             SCARD_SHARE_SHARED,
                             SCARD_PROTOCOL_Tx,
                             SCARD_UNPOWER_CARD,
                             (LPDWORD)&m_ActiveProtocol);
}

//--------------------------------------------------------------------------------
QStringList PCSCReader::getReaderList() {
    QStringList readers;
    wchar_t *mszReaders = nullptr;
    DWORD dwReaders = SCARD_AUTOALLOCATE;

    if (!CHECK_SCARD_ERROR(
            SCardListReadersW, m_Context, SCARD_ALL_READERS, (LPTSTR)&mszReaders, &dwReaders)) {
        return readers;
    }

    wchar_t *pReader = mszReaders;

    while ('\0' != *pReader) {
        readers << QString::from_WCharArray(pReader);
        // Advance to the next value.
        pReader = pReader + wcslen(pReader) + 1;
    }

    CHECK_SCARD_ERROR(SCardFreeMemory, m_Context, mszReaders);

    return readers;
}

//--------------------------------------------------------------------------------
bool PCSCReader::handleResult(const QString &aFunctionName, HRESULT aResultCode) {
    // Complete list of APDU responses
    // http://www.eftlab.co.uk/index.php/site-map/knowledge-base/118-apdu-response-list

    if (aResultCode == SCARD_S_SUCCESS) {
        return true;
    }

    SDeviceCodeSpecification specification = CPCSCReader::DeviceCodeSpecification[aResultCode];
    m_StatusCodes.insert(specification.statusCode);

    QString log = ISysUtils::getErrorMessage(aResultCode, false);

    if (log.isEmpty()) {
        log = specification.description;
    }

    toLog(LogLevel::Normal, aFunctionName + " returns " + log);

    return false;
}

//--------------------------------------------------------------------------------
TStatusCodes PCSCReader::getStatusCodes() {
    TStatusCodes statusCodes(m_StatusCodes);
    m_StatusCodes.clear();

    return statusCodes;
}

//--------------------------------------------------------------------------------
bool PCSCReader::connect(const QString &aReaderName) {
    // TOSO PORT_QT5 (const wchar_t *)aReaderName.utf16()???
    if (!CHECK_SCARD_ERROR(SCardConnect,
                           m_Context,
                           (const wchar_t *)aReaderName.utf16(),
                           SCARD_SHARE_SHARED,
                           SCARD_PROTOCOL_Tx,
                           &m_Card,
                           (LPDWORD)&m_ActiveProtocol)) {
        return false;
    }

    if (m_ActiveProtocol == SCARD_PROTOCOL_T0)
        m_PioSendPci = *SCARD_PCI_T0;
    if (m_ActiveProtocol == SCARD_PROTOCOL_T1)
        m_PioSendPci = *SCARD_PCI_T1;

    return true;
}

//--------------------------------------------------------------------------------
void PCSCReader::disconnect(bool aEject) {
    if (m_Card) {
        CHECK_SCARD_ERROR(SCardDisconnect, m_Card, aEject ? SCARD_EJECT_CARD : SCARD_LEAVE_CARD);
        m_Card = 0;
    }
}

//--------------------------------------------------------------------------------
bool PCSCReader::communicate(const QByteArray &aRequest, QByteArray &aResponse) {
    char outBuffer[4096] = {0};
    DWORD dwRecvLength = 4096; // достаточно ;)

    if (!CHECK_SCARD_ERROR(SCardTransmit,
                           m_Card,
                           &m_PioSendPci,
                           (const byte *)aRequest.constData(),
                           aRequest.size(),
                           NULL,
                           (LPBYTE)outBuffer,
                           &dwRecvLength)) {
        return false;
    }

    aResponse.clear();
    aResponse.append(outBuffer, dwRecvLength);

    return true;
}

//--------------------------------------------------------------------------------
bool PCSCReader::isConnected() const {
    return m_Card != 0;
}

//--------------------------------------------------------------------------------
