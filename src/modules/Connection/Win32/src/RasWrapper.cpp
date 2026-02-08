/* @file Обёртка RAS API WIN32 */

#define _SCL_SECURE_NO_WARNINGS

#include "RasWrapper.h"

#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

namespace RasApi {

//------------------------------------------------------------------------------
EOSVersion::Enum getOsVersion() {
    static EOSVersion::Enum version = EOSVersion::Unknown;

    if (version == EOSVersion::Unknown) {
        OSVERSIONINFO info;
        info.dwOSVersionInfoSize = sizeof(info);
        if (GetVersionEx(&info)) {
            DWORD winver = (info.dwMajorVersion << 8) | info.dwMinorVersion;

            if (winver == 0x0500) {
                version = EOSVersion::Windows2000;
            } else if (winver == 0x0501) {
                version = EOSVersion::WindowsXP;
            } else if (winver == 0x0502) {
                version = EOSVersion::Windows2003;
            } else if (winver == 0x0600) {
                version = EOSVersion::WindowsVista;
            } else if (winver == 0x0601) {
                version = EOSVersion::Windows7;
            } else if (winver == 0x0602) {
                version = EOSVersion::Windows8;
            } else if (winver == 0x0603) {
                version = EOSVersion::Windows81;
            } else if (winver >= 0xa000) {
                version = EOSVersion::Windows10;
            }
        } else {
            std::cerr << "RasApi: GetVersionEx() failed, will use WindowsXP as default version."
                      << std::endl;
        }
    }

    return version;
}

//------------------------------------------------------------------------------
std::wstring EDeviceType::ToString(EDeviceType::Enum type) {
    switch (type) {
    case Modem:
        return RASDT_Modem;
    case Isdn:
        return RASDT_Isdn;
    case X25:
        return RASDT_X25;
    case Vpn:
        return RASDT_Vpn;
    case Pad:
        return RASDT_Pad;
    case Generic:
        return RASDT_Generic;
    case Serial:
        return RASDT_Serial;
    case FrameRelay:
        return RASDT_FrameRelay;
    case Atm:
        return RASDT_Atm;
    case Sonet:
        return RASDT_Sonet;
    case SW56:
        return RASDT_SW56;
    case Irda:
        return RASDT_Irda;
    case Parallel:
        return RASDT_Parallel;
    case PPPoE:
        return RASDT_PPPoE;
    }

    return L"Unknown device type";
}

//------------------------------------------------------------------------------
EDeviceType::Enum EDeviceType::ToEnum(const std::wstring &type) {
    if (!_wcsicmp(type.c_str(), RASDT_Modem)) {
        return Modem;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Isdn)) {
        return Isdn;
    }
    if (!_wcsicmp(type.c_str(), RASDT_X25)) {
        return X25;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Vpn)) {
        return Vpn;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Pad)) {
        return Pad;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Generic)) {
        return Generic;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Serial)) {
        return Serial;
    }
    if (!_wcsicmp(type.c_str(), RASDT_FrameRelay)) {
        return FrameRelay;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Atm)) {
        return Atm;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Sonet)) {
        return Sonet;
    }
    if (!_wcsicmp(type.c_str(), RASDT_SW56)) {
        return SW56;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Irda)) {
        return Irda;
    }
    if (!_wcsicmp(type.c_str(), RASDT_Parallel)) {
        return Parallel;
    }
    if (!_wcsicmp(type.c_str(), RASDT_PPPoE)) {
        return PPPoE;
    }

    return Unknown;
}

//------------------------------------------------------------------------------
std::wstring EErrorCode::toString(DWORD aCode) {
    wchar_t buf[256];

    if (RasGetErrorString(aCode, buf, sizeof(buf)) == EErrorCode::NoError) {
        return buf;
    }
    return L"Unknown RAS error";
}

//------------------------------------------------------------------------------
IpAddress::IpAddress() {
    std::memset(&m_Address, 0, sizeof(m_Address));
}

//--------------------------------------------------------------------------------
IpAddress::IpAddress(const RASIPADDR &aIpAddress) {
    m_Address = aIpAddress;
}

//--------------------------------------------------------------------------------
IpAddress::operator RASIPADDR *() {
    return &m_Address;
}

//--------------------------------------------------------------------------------
IpAddress::operator const RASIPADDR *() const {
    return &m_Address;
}

//--------------------------------------------------------------------------------
char IpAddress::byte(size_t index) {
    switch (index) {
    case 0:
        return m_Address.a;
    case 1:
        return m_Address.b;
    case 2:
        return m_Address.c;
    case 3:
        return m_Address.d;
    }

    return 0;
}

//--------------------------------------------------------------------------------
void IpAddress::setByte(size_t index, char byte) {
    switch (index) {
    case 0:
        m_Address.a = byte;
    case 1:
        m_Address.b = byte;
    case 2:
        m_Address.c = byte;
    case 3:
        m_Address.d = byte;
    }
}

//------------------------------------------------------------------------------
PhonebookEntryName::PhonebookEntryName() {
    std::memset(&m_Entry, 0, sizeof(m_Entry));
    m_Entry.dwSize = sizeof(m_Entry);
}

//--------------------------------------------------------------------------------
PhonebookEntryName::PhonebookEntryName(const RASENTRYNAME &aEntry) {
    m_Entry = aEntry;
}

//--------------------------------------------------------------------------------
PhonebookEntryName::operator RASENTRYNAME *() {
    return &m_Entry;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntryName::name() const {
    return m_Entry.szEntryName;
}

//--------------------------------------------------------------------------------
void PhonebookEntryName::setName(const std::wstring &aName) {
    std::memset(m_Entry.szEntryName, 0, sizeof(m_Entry.szEntryName));
    aName.copy(m_Entry.szEntryName, aName.size());
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntryName::phonebookPath() const {
    return m_Entry.szPhonebookPath;
}

//--------------------------------------------------------------------------------
void PhonebookEntryName::setPhonebookPath(const std::wstring &aPath) {
    std::memset(m_Entry.szPhonebookPath, 0, sizeof(m_Entry.szPhonebookPath));
    aPath.copy(m_Entry.szPhonebookPath, aPath.size());
}

//--------------------------------------------------------------------------------
bool PhonebookEntryName::isSystem() const {
    return m_Entry.dwFlags == EPhonebookEntry::AllUsers;
}

//--------------------------------------------------------------------------------
void PhonebookEntryName::setIsSystem(bool aIsSystem) {
    m_Entry.dwFlags = aIsSystem ? EPhonebookEntry::AllUsers : EPhonebookEntry::Private;
}

//------------------------------------------------------------------------------
PhonebookEntry::PhonebookEntry() {
    m_Entry = 0;
    DWORD size = 0;

    m_LastError = RasGetEntryProperties(0, 0, 0, &size, 0, 0);

    if (m_LastError == ERROR_BUFFER_TOO_SMALL) {
        m_Entry = reinterpret_cast<LPRASENTRY>(new unsigned char[size]);
        std::memset(m_Entry, 0, size);
        m_Entry->dwSize = sizeof(RASENTRY);
        m_LastError = ERROR_SUCCESS;
    }
}

//--------------------------------------------------------------------------------
/*
PhonebookEntry::PhonebookEntry(const RASENTRY & aEntry)
{
        m_Entry = aEntry;
}
*/
//--------------------------------------------------------------------------------
PhonebookEntry::~PhonebookEntry() {
    delete[] m_Entry;
}

//--------------------------------------------------------------------------------
PhonebookEntry::operator RASENTRY *() {
    return m_Entry;
}

//--------------------------------------------------------------------------------
EConnectionOption::OptionSet PhonebookEntry::options() const {
    return m_Entry->dwfOptions;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setOptions(EConnectionOption::OptionSet aOptions) {
    m_Entry->dwfOptions = aOptions;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::countryId() const {
    return m_Entry->dwCountryID;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setCountryId(size_t aId) {
    m_Entry->dwCountryID = aId;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::countryCode() const {
    return m_Entry->dwCountryCode;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setCountryCode(size_t aCode) {
    m_Entry->dwCountryCode = aCode;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::areaCode() const {
    return m_Entry->szAreaCode;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setAreaCode(const std::wstring &aCode) {
    std::memset(m_Entry->szAreaCode, 0, sizeof(m_Entry->szAreaCode));
    aCode.copy(m_Entry->szAreaCode, aCode.size());
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::localPhoneNumber() const {
    return m_Entry->szLocalPhoneNumber;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setLocalPhoneNumber(const std::wstring &aNumber) {
    std::memset(m_Entry->szLocalPhoneNumber, 0, sizeof(m_Entry->szLocalPhoneNumber));
    aNumber.copy(m_Entry->szLocalPhoneNumber, aNumber.size());
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::ip() const {
    return m_Entry->ipaddr;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setIp(const IpAddress &aIp) {
    m_Entry->ipaddr = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::dnsIp() const {
    return m_Entry->ipaddrDns;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDnsIp(const IpAddress &aIp) {
    m_Entry->ipaddrDns = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::dnsAltIp() const {
    return m_Entry->ipaddrDnsAlt;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDnsAltIp(const IpAddress &aIp) {
    m_Entry->ipaddrDnsAlt = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::winsIp() const {
    return m_Entry->ipaddrWins;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setWinsIp(const IpAddress &aIp) {
    m_Entry->ipaddrWins = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::winsAltIp() const {
    return m_Entry->ipaddrWinsAlt;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setWinsAltIp(const IpAddress &aIp) {
    m_Entry->ipaddrWinsAlt = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::frameSize() const {
    return m_Entry->dwFrameSize;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setFrameSize(size_t aSize) {
    m_Entry->dwFrameSize = aSize;
}

//--------------------------------------------------------------------------------
ENetworkProtocol::ProtocolSet PhonebookEntry::netProtocols() const {
    return m_Entry->dwfNetProtocols;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setNetProtocols(ENetworkProtocol::ProtocolSet aProtocols) {
    m_Entry->dwfNetProtocols = aProtocols;
}

//--------------------------------------------------------------------------------
EFramingProtocol::Enum PhonebookEntry::framingProtocol() const {
    return EFramingProtocol::Enum(m_Entry->dwFramingProtocol);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setFramingProtocol(EFramingProtocol::Enum aProtocol) {
    m_Entry->dwFramingProtocol = aProtocol;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::script() const {
    return m_Entry->szScript;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setScript(const std::wstring &aScript) {
    std::memset(m_Entry->szScript, 0, sizeof(m_Entry->szScript));
    aScript.copy(m_Entry->szScript, aScript.size());
}

//--------------------------------------------------------------------------------
EDeviceType::Enum PhonebookEntry::deviceType() {
    return EDeviceType::ToEnum(m_Entry->szDeviceType);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDeviceType(EDeviceType::Enum aType) {
    std::wstring type = EDeviceType::ToString(aType);
    std::memset(m_Entry->szDeviceType, 0, sizeof(m_Entry->szDeviceType));
    type.copy(m_Entry->szDeviceType, type.size());
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::deviceName() const {
    return m_Entry->szDeviceName;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDeviceName(const std::wstring &aName) {
    std::memset(m_Entry->szDeviceName, 0, sizeof(m_Entry->szDeviceName));
    aName.copy(m_Entry->szDeviceName, aName.size());
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::subEntries() const {
    return m_Entry->dwSubEntries;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setSubEntries(size_t aEntries) {
    m_Entry->dwSubEntries = aEntries;
}

//--------------------------------------------------------------------------------
EDialMode::Enum PhonebookEntry::dialMode() const {
    return EDialMode::Enum(m_Entry->dwDialMode);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDialMode(EDialMode::Enum aMode) {
    m_Entry->dwDialMode = aMode;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::dialExtraPercent() const {
    return m_Entry->dwDialExtraPercent;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDialExtraPercent(size_t aPercent) {
    m_Entry->dwDialExtraPercent = aPercent;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::dialExtraSampleSeconds() const {
    return m_Entry->dwDialExtraSampleSeconds;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDialExtraSampleSeconds(size_t aSeconds) {
    m_Entry->dwDialExtraSampleSeconds = aSeconds;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::hangUpExtraPercent() const {
    return m_Entry->dwHangUpExtraPercent;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setHangUpExtraPercent(size_t aPercent) {
    m_Entry->dwHangUpExtraPercent = aPercent;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::hangUpExtraSampleSeconds() const {
    return m_Entry->dwHangUpExtraSampleSeconds;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setHangUpExtraSampleSeconds(size_t aSeconds) {
    m_Entry->dwHangUpExtraSampleSeconds = aSeconds;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::idleDisconnectSeconds() const {
    return m_Entry->dwIdleDisconnectSeconds;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setIdleDisconnectSeconds(size_t aSeconds) {
    m_Entry->dwIdleDisconnectSeconds = aSeconds;
}

//--------------------------------------------------------------------------------
EPhonebookEntry::TypeEnum PhonebookEntry::phonebookEntryType() const {
    return EPhonebookEntry::TypeEnum(m_Entry->dwType);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setPhonebookEntryType(EPhonebookEntry::TypeEnum aType) {
    m_Entry->dwType = aType;
}

//--------------------------------------------------------------------------------
EEncryptionType::Enum PhonebookEntry::encryptionType() const {
    return EEncryptionType::Enum(m_Entry->dwEncryptionType);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setEncryptionType(EEncryptionType::Enum aType) {
    m_Entry->dwEncryptionType = aType;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::custom_AuthKey() const {
    return m_Entry->dwCustom_AuthKey;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setCustom_AuthKey(size_t aKey) {
    m_Entry->dwCustom_AuthKey = aKey;
}

//--------------------------------------------------------------------------------
GUID PhonebookEntry::bookEntryGuid() const {
    return m_Entry->guidId;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setBookEntryGuid(const GUID &aGuid) {
    m_Entry->guidId = aGuid;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::custom_DialDll() const {
    return m_Entry->szCustom_DialDll;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setCustom_DialDll(const std::wstring &aDll) {
    std::memset(m_Entry->szCustom_DialDll, 0, sizeof(m_Entry->szCustom_DialDll));
    aDll.copy(m_Entry->szCustom_DialDll, aDll.size());
}

//--------------------------------------------------------------------------------
EVpnStrategy::Enum PhonebookEntry::vpnStrategy() const {
    return EVpnStrategy::Enum(m_Entry->dwVpnStrategy);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setVpnStrategy(EVpnStrategy::Enum aStrategy) {
    m_Entry->dwVpnStrategy = aStrategy;
}

//--------------------------------------------------------------------------------
EConnectionOption2::OptionSet PhonebookEntry::options2() const {
    return EConnectionOption2::OptionSet(m_Entry->dwfOptions2);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setOptions2(EConnectionOption2::OptionSet aOptions) {
    m_Entry->dwfOptions2 = aOptions;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::dnsSuffix() const {
    return m_Entry->szDnsSuffix;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDnsSuffix(const std::wstring &aSuffix) {
    std::memset(m_Entry->szDnsSuffix, 0, sizeof(m_Entry->szDnsSuffix));
    aSuffix.copy(m_Entry->szDnsSuffix, aSuffix.size());
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::tcpWindowSize() const {
    return m_Entry->dwTcpWindowSize;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setTcpWindowSize(size_t aSize) {
    m_Entry->dwTcpWindowSize = aSize;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::prerequisitePhonebook() const {
    return m_Entry->szPrerequisitePbk;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setPrerequisitePhonebook(const std::wstring &aPhonebook) {
    std::memset(m_Entry->szPrerequisitePbk, 0, sizeof(m_Entry->szPrerequisitePbk));
    aPhonebook.copy(m_Entry->szPrerequisitePbk, aPhonebook.size());
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::prerequisiteEntry() const {
    return m_Entry->szPrerequisiteEntry;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setPrerequisiteEntry(const std::wstring &aEntry) {
    std::memset(m_Entry->szPrerequisiteEntry, 0, sizeof(m_Entry->szPrerequisiteEntry));
    aEntry.copy(m_Entry->szPrerequisiteEntry, aEntry.size());
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::redialCount() const {
    return m_Entry->dwRedialCount;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setRedialCount(size_t aCount) {
    m_Entry->dwRedialCount = aCount;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::redialPause() const {
    return m_Entry->dwRedialPause;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setRedialPause(size_t aPause) {
    m_Entry->dwRedialPause = aPause;
}

//------------------------------------------------------------------------------
Connection::Connection() {
    std::memset(&m_Connection, 0, sizeof(m_Connection));
    m_Connection.dwSize = sizeof(m_Connection);
}

//--------------------------------------------------------------------------------
Connection::Connection(const RASCONN &aConnection) {
    m_Connection = aConnection;
}

//--------------------------------------------------------------------------------
void Connection::reset(const RASCONN &aConnection) {
    m_Connection = aConnection;
}

//--------------------------------------------------------------------------------
Connection::operator RASCONN *() {
    return &m_Connection;
}

//--------------------------------------------------------------------------------
HRASCONN Connection::handle() const {
    return m_Connection.hrasconn;
}

//--------------------------------------------------------------------------------
void Connection::setHandle(HRASCONN aHandle) {
    m_Connection.hrasconn = aHandle;
}

//--------------------------------------------------------------------------------
std::wstring Connection::entryName() const {
    return m_Connection.szEntryName;
}

//--------------------------------------------------------------------------------
void Connection::setEntryName(const std::wstring &aName) {
    std::memset(m_Connection.szEntryName, 0, sizeof(m_Connection.szEntryName));
    aName.copy(m_Connection.szEntryName, aName.size());
}

//--------------------------------------------------------------------------------
EDeviceType::Enum Connection::deviceType() {
    return EDeviceType::ToEnum(m_Connection.szDeviceType);
}

//--------------------------------------------------------------------------------
void Connection::setDeviceType(EDeviceType::Enum aType) {
    std::wstring typeStr = EDeviceType::ToString(aType);
    std::memset(m_Connection.szDeviceType, 0, sizeof(m_Connection.szDeviceType));
    typeStr.copy(m_Connection.szDeviceType, typeStr.size());
}

//--------------------------------------------------------------------------------
std::wstring Connection::deviceName() const {
    return m_Connection.szDeviceName;
}

//--------------------------------------------------------------------------------
void Connection::setDeviceName(const std::wstring &aName) {
    std::memset(m_Connection.szDeviceName, 0, sizeof(m_Connection.szDeviceName));
    aName.copy(m_Connection.szDeviceName, aName.size());
}

//--------------------------------------------------------------------------------
std::wstring Connection::phonebookPath() const {
    return m_Connection.szPhonebook;
}

//--------------------------------------------------------------------------------
void Connection::setPhonebookPath(const std::wstring &aPath) {
    std::memset(m_Connection.szPhonebook, 0, sizeof(m_Connection.szPhonebook));
    aPath.copy(m_Connection.szPhonebook, aPath.size());
}

//--------------------------------------------------------------------------------
size_t Connection::subEntryIndex() const {
    return m_Connection.dwSubEntry;
}

//--------------------------------------------------------------------------------
void Connection::setSubEntryIndex(size_t aIndex) {
    m_Connection.dwSubEntry = aIndex;
}

//--------------------------------------------------------------------------------
GUID Connection::entryGuid() const {
    return m_Connection.guidEntry;
}

//--------------------------------------------------------------------------------
void Connection::setEntryGuid(const GUID &aGuid) {
    m_Connection.guidEntry = aGuid;
}

//--------------------------------------------------------------------------------
bool Connection::isSystem() const {
    return m_Connection.dwFlags & EConnectionFlag::AllUsers;
}

//--------------------------------------------------------------------------------
void Connection::setIsSystem(bool aIsSystem) {
    if (aIsSystem) {
        m_Connection.dwFlags |= EConnectionFlag::AllUsers;
    } else {
        m_Connection.dwFlags &= ~EConnectionFlag::AllUsers;
    }
}

//--------------------------------------------------------------------------------
bool Connection::isGlobalCredsUsed() const {
    return (m_Connection.dwFlags & EConnectionFlag::GlobalCreds) != 0;
}

//--------------------------------------------------------------------------------
void Connection::setIsGlobalCredsUsed(bool aIsGlobalCredsUsed) {
    if (aIsGlobalCredsUsed) {
        m_Connection.dwFlags |= EConnectionFlag::GlobalCreds;
    } else {
        m_Connection.dwFlags &= ~EConnectionFlag::GlobalCreds;
    }
}

//--------------------------------------------------------------------------------
LUID Connection::localSessionId() const {
    return m_Connection.luid;
}

//--------------------------------------------------------------------------------
void Connection::setLocalSessionId(const LUID &aId) {
    m_Connection.luid = aId;
}

//------------------------------------------------------------------------------
Device::Device() {
    std::memset(&m_Device, 0, sizeof(m_Device));
    m_Device.dwSize = sizeof(m_Device);
}

//--------------------------------------------------------------------------------
Device::Device(const RASDEVINFO &aDevice) {
    m_Device = aDevice;
}

//--------------------------------------------------------------------------------
Device::operator RASDEVINFO *() {
    return &m_Device;
}

//--------------------------------------------------------------------------------
std::wstring Device::type() const {
    return m_Device.szDeviceType;
}

//--------------------------------------------------------------------------------
void Device::setType(const std::wstring &aType) {
    std::memset(m_Device.szDeviceType, 0, sizeof(m_Device.szDeviceType));
    aType.copy(m_Device.szDeviceType, aType.size());
}

//--------------------------------------------------------------------------------
std::wstring Device::name() const {
    return m_Device.szDeviceName;
}

//--------------------------------------------------------------------------------
void Device::setName(const std::wstring &aName) {
    std::memset(m_Device.szDeviceName, 0, sizeof(m_Device.szDeviceName));
    aName.copy(m_Device.szDeviceName, aName.size());
}

//------------------------------------------------------------------------------
DialParams::DialParams() : m_HasSavedPassword(false), m_RemovePassword(false) {
    std::memset(&m_Params, 0, sizeof(m_Params));
    m_Params.dwSize = sizeof(m_Params);
}

//--------------------------------------------------------------------------------
DialParams::DialParams(const RASDIALPARAMS &aParams)
    : m_HasSavedPassword(false), m_RemovePassword(false) {
    m_Params = aParams;
}

//--------------------------------------------------------------------------------
DialParams::operator RASDIALPARAMS *() {
    return &m_Params;
}

//--------------------------------------------------------------------------------
std::wstring DialParams::entryName() const {
    return m_Params.szEntryName;
}

//--------------------------------------------------------------------------------
void DialParams::setEntryName(const std::wstring &aName) {
    std::memset(m_Params.szEntryName, 0, sizeof(m_Params.szEntryName));
    aName.copy(m_Params.szEntryName, sizeof(m_Params.szEntryName));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::phoneNumber() const {
    return m_Params.szPhoneNumber;
}

//--------------------------------------------------------------------------------
void DialParams::setPhoneNumber(const std::wstring &aNumber) {
    std::memset(m_Params.szPhoneNumber, 0, sizeof(m_Params.szPhoneNumber));
    aNumber.copy(m_Params.szPhoneNumber, sizeof(m_Params.szPhoneNumber));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::callbackNumber() const {
    return m_Params.szCallbackNumber;
}

//--------------------------------------------------------------------------------
void DialParams::setCallbackNumber(const std::wstring &aNumber) {
    std::memset(m_Params.szCallbackNumber, 0, sizeof(m_Params.szCallbackNumber));
    aNumber.copy(m_Params.szCallbackNumber, sizeof(m_Params.szCallbackNumber));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::userName() const {
    return m_Params.szUserName;
}

//--------------------------------------------------------------------------------
void DialParams::setUserName(const std::wstring &aName) {
    std::memset(m_Params.szUserName, 0, sizeof(m_Params.szUserName));
    aName.copy(m_Params.szUserName, sizeof(m_Params.szUserName));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::password() const {
    return m_Params.szPassword;
}

//--------------------------------------------------------------------------------
void DialParams::setPassword(const std::wstring &aPassword) {
    std::memset(m_Params.szPassword, 0, sizeof(m_Params.szPassword));
    aPassword.copy(m_Params.szPassword, sizeof(m_Params.szPassword));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::domain() const {
    return m_Params.szDomain;
}

//--------------------------------------------------------------------------------
void DialParams::setDomain(const std::wstring &aDomain) {
    std::memset(m_Params.szDomain, 0, sizeof(m_Params.szDomain));
    aDomain.copy(m_Params.szDomain, sizeof(m_Params.szDomain));
}

//--------------------------------------------------------------------------------
unsigned int DialParams::subEntry() const {
    return m_Params.dwSubEntry;
}

//--------------------------------------------------------------------------------
void DialParams::setSubEntry(unsigned int aIndex) {
    m_Params.dwSubEntry = aIndex;
}

//--------------------------------------------------------------------------------
unsigned long DialParams::callbackId() const {
    return m_Params.dwCallbackId;
}

//--------------------------------------------------------------------------------
void DialParams::setCallbackId(unsigned long aValue) {
    m_Params.dwCallbackId = aValue;
}

//--------------------------------------------------------------------------------
bool DialParams::hasSavedPassword() const {
    return m_HasSavedPassword;
}

//--------------------------------------------------------------------------------
void DialParams::setHasSavedPassword(bool aHasPassword) {
    m_HasSavedPassword = aHasPassword;
}

//--------------------------------------------------------------------------------
bool DialParams::removePassword() const {
    return m_RemovePassword;
}

//--------------------------------------------------------------------------------
void DialParams::setRemovePassword(bool aRemove) {
    m_RemovePassword = aRemove;
}

//------------------------------------------------------------------------------
PhonebookEntryEnumerator::PhonebookEntryEnumerator(const std::wstring &aPhonebookPath)
    : m_RequestedBufSize(0), m_Entries(0) {
    reset(aPhonebookPath);
}

//------------------------------------------------------------------------------
PhonebookEntryEnumerator::~PhonebookEntryEnumerator() {
    delete[] m_Entries;
}

//------------------------------------------------------------------------------
bool PhonebookEntryEnumerator::getEntry(PhonebookEntryName &aEntry) {
    return m_CurrentIndex < m_EntryCount;
}

//------------------------------------------------------------------------------
void PhonebookEntryEnumerator::reset(const std::wstring &aPhonebookPath) {
    m_CurrentIndex = 0;
    m_EntryCount = 0;
    m_RequestedBufSize = 0;
    RASENTRYNAME dummy;
    dummy.dwSize = sizeof(dummy);

    m_LastError = RasEnum_Entries(
        0,
        0,
        GetOSVersion() < EOSVersion::WindowsVista ? &dummy : 0, // различие(баг) в апи
        &m_RequestedBufSize,
        &m_EntryCount);

    if (m_LastError == ERROR_BUFFER_TOO_SMALL) {
        delete[] m_Entries;
        m_Entries = reinterpret_cast<LPRASENTRYNAME>(new char[m_RequestedBufSize]);
        m_Entries[0].dwSize = sizeof(m_Entries[0]);
        m_LastError = RasEnum_Entries(0,
                                      aPhonebookPath.empty() ? 0 : aPhonebookPath.data(),
                                      m_Entries,
                                      &m_RequestedBufSize,
                                      &m_EntryCount);
    }
}

//------------------------------------------------------------------------------
ConnectionEnumerator::ConnectionEnumerator() : m_RequestedBufSize(0), m_Connections(0) {
    reset();
}

//--------------------------------------------------------------------------------
ConnectionEnumerator::~ConnectionEnumerator() {
    delete[] m_Connections;
}

//--------------------------------------------------------------------------------
bool ConnectionEnumerator::getConnection(Connection &aConnection) {
    if (m_CurrentIndex < m_ConnectionCount) {
        aConnection.reset(m_Connections[m_CurrentIndex++]);
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------
void ConnectionEnumerator::reset() {
    m_CurrentIndex = 0;
    m_ConnectionCount = 0;
    DWORD connectionsCount = 0;
    DWORD reqBytesCount = 0;

    RASCONN dummy;
    dummy.dwSize = sizeof(dummy);

    m_LastError = RasEnum_Connections(
        GetOSVersion() < EOSVersion::WindowsVista ? &dummy : 0, // различие(баг) в апи
        &reqBytesCount,
        &connectionsCount);

    m_RequestedBufSize = reqBytesCount;
    m_ConnectionCount = connectionsCount;

    if (m_LastError == ERROR_BUFFER_TOO_SMALL) {
        delete[] m_Connections;
        m_Connections = reinterpret_cast<LPRASCONN>(new char[m_RequestedBufSize]);
        m_Connections[0].dwSize = sizeof(m_Connections[0]);
        m_LastError = RasEnum_Connections(m_Connections, &m_RequestedBufSize, &m_ConnectionCount);
    }
}

//------------------------------------------------------------------------------
DeviceEnumerator::DeviceEnumerator() : m_RequestedBufSize(0), m_Devices(0) {
    reset();
}

//--------------------------------------------------------------------------------
DeviceEnumerator::~DeviceEnumerator() {
    delete[] m_Devices;
}

//--------------------------------------------------------------------------------
bool DeviceEnumerator::getDevice(Device &aDevice) {
    return m_CurrentIndex < m_DeviceCount;
}

//--------------------------------------------------------------------------------
void DeviceEnumerator::reset() {
    m_CurrentIndex = 0;
    m_DeviceCount = 0;

    m_LastError = RasEnum_Devices(0, &m_RequestedBufSize, &m_DeviceCount);

    if (m_LastError == ERROR_BUFFER_TOO_SMALL && m_DeviceCount) {
        delete[] m_Devices;
        m_Devices = new RASDEVINFO[m_RequestedBufSize / m_DeviceCount];
        m_Devices[0].dwSize = sizeof(m_Devices[0]);
        m_LastError = RasEnum_Devices(m_Devices, &m_RequestedBufSize, &m_DeviceCount);
    }
}

//------------------------------------------------------------------------------
DWORD validatePhonebookEntryName(PhonebookEntryName &aEntry) {
    DWORD error = RasValidateEntryName(
        aEntry.phonebookPath().empty() ? 0 : aEntry.phonebookPath().data(), aEntry.name().data());

    return EErrorCode::Enum(error);
}

//------------------------------------------------------------------------------
DWORD createNewPhonebookEntry(const PhonebookEntryName &aEntryName, PhonebookEntry &aEntry) {
    DWORD size = 0;

    DWORD error = RasGetEntryProperties(0, 0, 0, &size, 0, 0);

    if (error == ERROR_BUFFER_TOO_SMALL) {
        error = RasSetEntryProperties(
            aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(),
            aEntryName.name().data(),
            (RASENTRY *)aEntry,
            size,
            0,
            0);
    }

    return error;
}

//------------------------------------------------------------------------------
DWORD removePhonebookEntry(const PhonebookEntryName &aEntryName) {
    DWORD size = 0;

    DWORD error = RasGetEntryProperties(0, 0, 0, &size, 0, 0);

    if (error == ERROR_BUFFER_TOO_SMALL) {
        error = RasDeleteEntry(
            aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(),
            aEntryName.name().data());
    }

    return error;
}

//------------------------------------------------------------------------------
DWORD getConnectionStatus(const std::wstring &aConnectionName, EConnectionStatus::Enum &aStatus) {
    DWORD error = ERROR_SUCCESS;
    Connection conn;
    ConnectionEnumerator enumerator;

    if (enumerator.isValid()) {
        bool found = false;

        while (RasApi::ConnectionEnumerator::getConnection(conn)) {
            if (conn.entryName() == aConnectionName) {
                found = true;
                break;
            }
        }

        if (found) {
            RASCONNSTATUS rasstatus;
            rasstatus.dwSize = sizeof(RASCONNSTATUS);
            error = RasGetConnectStatus(conn.handle(), &rasstatus);

            if (error == ERROR_SUCCESS && rasstatus.rasconnstate == RASCS_Connected) {
                aStatus = EConnectionStatus::Connected;
            }
        } else {
            aStatus = EConnectionStatus::Disconnected;
        }
    } else {
        error = enumerator.getLastError();
    }

    return error;
}

//------------------------------------------------------------------------------
DWORD getEntryProperties(const PhonebookEntryName &aEntryName, PhonebookEntry &aEntry) {
    DWORD size = sizeof(*(RASENTRY *)aEntry);

    DWORD error = RasGetEntryProperties(
        aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(),
        aEntryName.name().data(),
        (RASENTRY *)aEntry,
        &size,
        0,
        0);

    return error;
}

//------------------------------------------------------------------------------
DWORD getEntryDialParams(const PhonebookEntryName &aEntryName, DialParams &aParameters) {
    RasApi::DialParams::setEntryName(aEntryName.name());

    BOOL hasSavedPassword = FALSE;

    DWORD error = RasGetEntryDialParams(
        aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(),
        static_cast<RASDIALPARAMS *>(aParameters),
        &hasSavedPassword);

    if (error == ERROR_SUCCESS) {
        aParameters.setHasSavedPassword(hasSavedPassword == TRUE);
    }

    return error;
}

//------------------------------------------------------------------------------
DWORD setEntryDialParams(const PhonebookEntryName &aEntryName, DialParams &aParams) {
    RasApi::DialParams::setEntryName(aEntryName.name());

    DWORD error = RasSetEntryDialParams(
        aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(),
        static_cast<RASDIALPARAMS *>(aParams),
        aParams.removePassword());

    return error;
}

//------------------------------------------------------------------------------
DWORD dial(const PhonebookEntryName &aEntryName) {
    DWORD error = ERROR_SUCCESS;

    // Проверяем наличие соединения
    PhonebookEntryName pbentry;
    PhonebookEntryEnumerator pbenum(aEntryName.phonebookPath());

    if (pbenum.isValid()) {
        bool found = false;

        while (RasApi::PhonebookEntryEnumerator::getEntry(pbentry)) {
            if (pbentry.name() == aEntryName.name()) {
                found = true;
                break;
            }
        }

        if (found) {
            // Загружаем сохранённый пароль
            DialParams dialParams;
            error = GetEntryDialParams(pbentry, dialParams);

            if (error == ERROR_SUCCESS) {
                // Устанавливаем номер для дозвона
                PhonebookEntry entry;
                error = GetEntryProperties(pbentry, entry);

                if (error == ERROR_SUCCESS) {
                    RasApi::DialParams::setPhoneNumber(entry.localPhoneNumber());

                    HRASCONN handle = 0;

                    // Устанавливаем соединение
                    error = RasDial(
                        0,
                        pbentry.phonebookPath().empty() ? 0 : pbentry.phonebookPath().data(),
                        static_cast<RASDIALPARAMS *>(dialParams),
                        0,
                        0,
                        &handle);

                    if (error != ERROR_SUCCESS && handle) {
                        RasHangUp(handle);
                    }
                }
            }
        } else {
            error = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        }
    } else {
        error = pbenum.getLastError();
    }

    return error;
}

//------------------------------------------------------------------------------
DWORD hangUp(const std::wstring &aConnectionName) {
    DWORD error;
    Connection conn;
    ConnectionEnumerator enumerator;

    if (enumerator.isValid()) {
        bool found = false;

        while (RasApi::ConnectionEnumerator::getConnection(conn)) {
            if (conn.entryName() == aConnectionName) {
                found = true;
                break;
            }
        }

        if (found) {
            EConnectionStatus::Enum status;
            error = GetConnectionStatus(aConnectionName, status);

            if (error == ERROR_SUCCESS && status == EConnectionStatus::Connected) {
                error = RasHangUp(conn.handle());
            }
        } else {
            error = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        }
    } else {
        error = enumerator.getLastError();
    }

    return error;
}

std::wstring getRegValue(HKEY aKey, const std::wstring &aValueName) {
    wchar_t value[MAX_PATH] = {'\0'};
    DWORD len = sizeof(value);

    DWORD type;
    if (RegQueryValueEx(aKey, aValueName.c_str(), 0, &type, (LPBYTE)value, &len) == ERROR_SUCCESS) {
        switch (type) {
        case REG_SZ:
            return std::wstring(value);
        }
    }

    return std::wstring();
}

std::wstring getAttachedTo(const std::wstring &aDeviceName) {
    std::wstring result;

    for (int index = 0; index < 20 && result.empty(); ++index) {
        wchar_t regPath[MAX_PATH] = {'\0'};
        swprintf(regPath,
                 MAX_PATH,
                 L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E96D-E325-11CE-BFC1-"
                 L"08002BE10318}\\%.4d",
                 index);

        HKEY modem;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, NULL, KEY_READ, &modem) == ERROR_SUCCESS) {
            if (getRegValue(modem, L"FriendlyName") == aDeviceName ||
                getRegValue(modem, L"DriverDesc") == aDeviceName ||
                getRegValue(modem, L"Model") == aDeviceName) {
                result = getRegValue(modem, L"AttachedTo");
            }

            RegCloseKey(modem);
        }
    }

    return result;
}

} // namespace RasApi
