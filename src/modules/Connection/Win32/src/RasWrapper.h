/* @file Обёртка RAS API WIN32 */

#pragma once

#include <string>

// Windows
// Минимальная версия необходимая для работы с RAS API - WindowsXP
#define OLD_WINVER WINVER
#if defined(WINVER)
#undef WINVER
#endif
#define WINVER 0x501
#define NOMINMAX // HACK for QDateTime Qt 5.0.0
#include <ras.h>
#include <raserror.h>
#include <windows.h>
#include <wininet.h>
#undef WINVER
#define WINVER OLD_WINVER
#undef OLD_WINVER

namespace RasApi {

//--------------------------------------------------------------------------------
/// Версии Windows для управления различиями в API.
namespace EOSVersion {
enum Enum {
    Unknown = 0,
    Windows2000,
    WindowsXP,
    Windows2003,
    WindowsVista,
    Windows7,
    Windows8,
    Windows81,
    Windows10,
    Windows11
};
} // namespace EOSVersion

//--------------------------------------------------------------------------------
/// Свойства соединения для RASENTRY::dwfOptions.
namespace EConnectionOption {
enum Enum {
    UseCountryAndAreaCodes = RASEO_UseCountryAndAreaCodes,
    SpecificIpAddr = RASEO_SpecificIpAddr,
    SpecificNameServers = RASEO_SpecificNameServers,
    IpHeaderCompression = RASEO_IpHeaderCompression,
    RemoteDefaultGateway = RASEO_RemoteDefaultGateway,
    DisableLcpExtensions = RASEO_DisableLcpExtensions,
    TerminalBeforeDial = RASEO_TerminalBeforeDial,
    TerminalAfterDial = RASEO_TerminalAfterDial,
    Modem_Lights = RASEO_Modem_Lights,
    SwCompression = RASEO_SwCompression,
    RequireEncryptedPw = RASEO_RequireEncryptedPw,
    RequireMsEncryptedPw = RASEO_RequireMsEncryptedPw,
    RequireDataEncryption = RASEO_RequireDataEncryption,
    NetworkLogon = RASEO_NetworkLogon,
    UseLogonCredentials = RASEO_UseLogonCredentials,
    PromoteAlternates = RASEO_PromoteAlternates,
    SecureLocalFiles = RASEO_SecureLocalFiles,
    RequireEAP = RASEO_RequireEAP,
    RequirePAP = RASEO_RequirePAP,
    RequireSPAP = RASEO_RequireSPAP,
    Custom = RASEO_Custom,
    PreviewPhoneNumber = RASEO_PreviewPhoneNumber,
    SharedPhoneNumbers = RASEO_SharedPhoneNumbers,
    PreviewUserPw = RASEO_PreviewUserPw,
    PreviewDomain = RASEO_PreviewDomain,
    ShowDialingProgress = RASEO_ShowDialingProgress,
    RequireCHAP = RASEO_RequireCHAP,
    RequireMsCHAP = RASEO_RequireMsCHAP,
    RequireMsCHAP2 = RASEO_RequireMsCHAP2,
    RequireW95MSCHAP = RASEO_RequireW95MSCHAP,
    Custom_Script = RASEO_Custom_Script
};

typedef DWORD OptionSet;
} // namespace EConnectionOption

//--------------------------------------------------------------------------------
/// Свойства соединения для RASENTRY::dwfOptions2.
namespace EConnectionOption2 {
enum Enum {
    SecureFileAndPrint = RASEO2_SecureFileAndPrint,
    SecureClientForMSNet = RASEO2_SecureClientForMSNet,
    DontNegotiateMultilink = RASEO2_DontNegotiateMultilink,
    DontUseRasCredentials = RASEO2_DontUseRasCredentials,
    UsePreSharedKey = RASEO2_UsePreSharedKey,
    Internet = RASEO2_Internet,
    DisableNbtOverIP = RASEO2_DisableNbtOverIP,
    UseGlobalDeviceSettings = RASEO2_UseGlobalDeviceSettings,
    ReconnectIfDropped = RASEO2_ReconnectIfDropped,
    SharePhoneNumbers = RASEO2_SharePhoneNumbers,
    /* эти значения только для Висты и выше
    SecureRoutingCompartment    = RASEO2_SecureRoutingCompartment,
    IPv6SpecificNameServer      = RASEO2_IPv6SpecificNameServers,
    IPv6RemoteDefaultGateway    = RASEO2_IPv6RemoteDefaultGateway,
    RegisterIpWithDNS           = RASEO2_RegisterIpWithDNS,
    UseDNSSuffixForRegistration = RASEO2_UseDNSSuffixForRegistration,
    IPv4ExplicitMetric          = RASEO2_IPv4ExplicitMetric,
    IPv6ExplicitMetric          = RASEO2_IPv6ExplicitMetric,
    DisableIKENameEkuCheck      = RASEO2_DisableIKENameEkuCheck
    */
    /* эти значения только для семёрки и выше
    DisableClassBasedStaticRoute = RASEO2_DisableClassBasedStaticRoute,
    SpecificIPv6Addr             = RASEO2_SpecificIPv6Addr,
    DisableMobility              = RASEO2_DisableMobility,
    RequireMachineCertificates   = RASEO2_RequireMachineCertificates
    */
};

typedef DWORD OptionSet;
} // namespace EConnectionOption2

//--------------------------------------------------------------------------------
/// Свойства соединения для RASCONN::dwFlags.
namespace EConnectionFlag {
enum Enum { AllUsers = RASCF_AllUsers, GlobalCreds = RASCF_GlobalCreds };

typedef size_t FlagSet;
} // namespace EConnectionFlag

//--------------------------------------------------------------------------------
/// Тип устройства для RASDEVINFO::szDeviceType.
namespace EDeviceType {
enum Enum {
    Unknown,
    Modem,
    Isdn,
    X25,
    Vpn,
    Pad,
    Generic,
    Serial,
    FrameRelay,
    Atm,
    Sonet,
    SW56,
    Irda,
    Parallel,
    PPPoE
};

std::wstring ToString(Enum type);
Enum ToEnum(const std::wstring &type);
} // namespace EDeviceType

//--------------------------------------------------------------------------------
/// Тип протокола для RASENTRY::dwProtocols.
namespace ENetworkProtocol {
enum Enum {
    Ipx = RASNP_Ipx,
    Ip = RASNP_Ip,
    /* эти значения только для Висты и выше
    IpV6    = RASNP_Ipv6
    */
};

typedef DWORD ProtocolSet;
} // namespace ENetworkProtocol

//--------------------------------------------------------------------------------
/// Тип framing protocol для RASENTRY::dwFramingProtocol.
namespace EFramingProtocol {
enum Enum { Ppp = RASFP_Ppp, Slip = RASFP_Slip };
} // namespace EFramingProtocol

//--------------------------------------------------------------------------------
namespace EPhonebookEntry {
/// Тип телефонной книги для RASENTRYNAME::dwFlags.
enum PhonebookTypeEnum { AllUsers = REN_AllUsers, Private = REN_User };

/// Тип записи телефонной книги для RASENTRY::dwType.
enum TypeEnum {
    Phone = RASET_Phone,
    Vpn = RASET_Vpn,
    Internet = RASET_Internet,
    Broadband = RASET_Broadband
};
} // namespace EPhonebookEntry

//--------------------------------------------------------------------------------
/// Способ набора multilink записей для RASENTRY::dwDialMode.
namespace EDialMode {
enum Enum { DialAll = RASEDM_DialAll, DialAsNeeded = RASEDM_DialAsNeeded };
} // namespace EDialMode

//--------------------------------------------------------------------------------
/// Тип шифрования для RASENTRY::dwEncryptionType.
namespace EEncryptionType {
enum Enum {
    None = ET_None,
    Require = ET_Require,
    RequireMax = ET_RequireMax,
    Optional = ET_Optional
};
} // namespace EEncryptionType

//--------------------------------------------------------------------------------
/// Свойства VPN для RASENTRY::dwVpnStrategy.
namespace EVpnStrategy {
enum Enum {
    Default = VS_Default,
    PptpOnly = VS_PptpOnly,
    PptpFirst = VS_PptpFirst,
    L2tpOnly = VS_L2tpOnly,
    L2tpFirst = VS_L2tpFirst,
    /* эти значения только для Висты и выше
    SstpOnly   = VS_SstpOnly,
    SstpFirst  = VS_SstpFirst,
    */
    /* эти значения только для семёрки и выше
    Ikev2Only  = VS_Ikev2Only,
    Ikev2First = VS_Ikev2First
    */
};
} // namespace EVpnStrategy

//--------------------------------------------------------------------------------
/// Коды ошибок, возвращаемые RAS API.
namespace EErrorCode {
enum Enum {
    NoError = ERROR_SUCCESS,
    InvalidName = ERROR_INVALID_NAME,
    InvalidBuffer = ERROR_BUFFER_INVALID,
    CannotOpenPhonebook = ERROR_CANNOT_OPEN_PHONEBOOK,
    InvalidParameter = ERROR_INVALID_PARAMETER,
    AlreadyExists = ERROR_ALREADY_EXISTS
};

std::wstring ToString(DWORD aCode);
} // namespace EErrorCode

//--------------------------------------------------------------------------------
/// Статус соединения для RASCONNSTATUS::rasconnstate.
namespace EConnectionStatus {
enum Enum { Connected = RASCS_Connected, Disconnected = RASCS_Disconnected };
} // namespace EConnectionStatus

//--------------------------------------------------------------------------------
/// Время простоя соединения перед разрывом для RASENTRY::dwIdleDisconnectSeconds.
namespace EIdleDisconnect {
enum Enum { Disabled = RASIDS_Disabled, UseGlobalValue = RASIDS_UseGlobalValue };
} // namespace EIdleDisconnect

//------------------------------------------------------------------------------
// База для классов, возвращающих результат операции
class RasBase {
public:
    DWORD getLastError() const { return m_LastError; }
    bool isValid() const { return m_LastError == ERROR_SUCCESS; }

protected:
    DWORD m_LastError;
};

//------------------------------------------------------------------------------
/// IP-адрес.
class IpAddress {
public:
    IpAddress();
    explicit IpAddress(const RASIPADDR &aIpAddress);

    operator RASIPADDR *();
    operator const RASIPADDR *() const;

    static char byte(size_t index);
    static void setByte(size_t index, char byte);

private:
    RASIPADDR m_Address{};
};

//------------------------------------------------------------------------------
/// Элемент телефонной книги.
class PhonebookEntryName {
public:
    PhonebookEntryName();
    explicit PhonebookEntryName(const RASENTRYNAME &aEntry);

    operator RASENTRYNAME *();

    std::wstring name() const;
    static void setName(const std::wstring &aName);

    std::wstring phonebookPath() const;
    static void setPhonebookPath(const std::wstring &aPath);

    bool isSystem() const;
    void setIsSystem(bool aIsSystem);

private:
    RASENTRYNAME m_Entry{};
};

//------------------------------------------------------------------------------
/// Параметры элемента телефонной книги.
class PhonebookEntry : public RasBase {
public:
    PhonebookEntry();
    ~PhonebookEntry();
    //	PhonebookEntry(const RASENTRY & aEntry);

    operator RASENTRY *();

    EConnectionOption::OptionSet options() const;
    void setOptions(EConnectionOption::OptionSet aOptions);

    // Location/phone number.
    size_t countryId() const;
    void setCountryId(size_t aId);

    size_t countryCode() const;
    void setCountryCode(size_t aCode);

    std::wstring areaCode() const;
    static void setAreaCode(const std::wstring &aCode);

    std::wstring localPhoneNumber() const;
    static void setLocalPhoneNumber(const std::wstring &aNumber);

    // PPP/Ip
    IpAddress ip() const;
    void setIp(const IpAddress &aIp);

    IpAddress dnsIp() const;
    void setDnsIp(const IpAddress &aIp);

    IpAddress dnsAltIp() const;
    void setDnsAltIp(const IpAddress &aIp);

    IpAddress winsIp() const;
    void setWinsIp(const IpAddress &aIp);

    IpAddress winsAltIp() const;
    void setWinsAltIp(const IpAddress &aIp);

    // Framing
    size_t frameSize() const;
    void setFrameSize(size_t aSize);

    ENetworkProtocol::ProtocolSet netProtocols() const;
    void setNetProtocols(ENetworkProtocol::ProtocolSet aProtocols);

    EFramingProtocol::Enum framingProtocol() const;
    void setFramingProtocol(EFramingProtocol::Enum aProtocol);

    // Scripting
    std::wstring script() const;
    static void setScript(const std::wstring &aScript);

    // Device
    static EDeviceType::Enum deviceType();
    static void setDeviceType(EDeviceType::Enum aType);

    std::wstring deviceName() const;
    static void setDeviceName(const std::wstring &aName);

    // Multilink and BAP
    size_t subEntries() const;
    void setSubEntries(size_t aEntries);

    EDialMode::Enum dialMode() const;
    void setDialMode(EDialMode::Enum aMode);

    size_t dialExtraPercent() const;
    void setDialExtraPercent(size_t aPercent);

    size_t dialExtraSampleSeconds() const;
    void setDialExtraSampleSeconds(size_t aSeconds);

    size_t hangUpExtraPercent() const;
    void setHangUpExtraPercent(size_t aPercent);

    size_t hangUpExtraSampleSeconds() const;
    void setHangUpExtraSampleSeconds(size_t aSeconds);

    // Idle time out
    size_t idleDisconnectSeconds() const;
    void setIdleDisconnectSeconds(size_t aSeconds);

    EPhonebookEntry::TypeEnum phonebookEntryType() const;
    void setPhonebookEntryType(EPhonebookEntry::TypeEnum aType);

    EEncryptionType::Enum encryptionType() const;
    void setEncryptionType(EEncryptionType::Enum aType);

    size_t custom_AuthKey() const;
    void setCustom_AuthKey(size_t aKey);

    GUID bookEntryGuid() const;
    void setBookEntryGuid(const GUID &aGuid);

    std::wstring custom_DialDll() const;
    static void setCustom_DialDll(const std::wstring &aDll);

    EVpnStrategy::Enum vpnStrategy() const;
    void setVpnStrategy(EVpnStrategy::Enum aStrategy);

    EConnectionOption2::OptionSet options2() const;
    void setOptions2(EConnectionOption2::OptionSet aOptions);

    std::wstring dnsSuffix() const;
    static void setDnsSuffix(const std::wstring &aSuffix);

    size_t tcpWindowSize() const;
    void setTcpWindowSize(size_t aSize);

    std::wstring prerequisitePhonebook() const;
    static void setPrerequisitePhonebook(const std::wstring &aPhonebook);

    std::wstring prerequisiteEntry() const;
    static void setPrerequisiteEntry(const std::wstring &aEntry);

    size_t redialCount() const;
    void setRedialCount(size_t aCount);

    size_t redialPause() const;
    void setRedialPause(size_t aPause);

private:
    LPRASENTRY m_Entry{};
};

//------------------------------------------------------------------------------
/// Dialup-соединение
class Connection {
public:
    Connection();
    explicit Connection(const RASCONN &aConnection);

    void reset(const RASCONN &aConnection);
    operator RASCONN *();

    HRASCONN handle() const;
    void setHandle(HRASCONN aHandle);

    std::wstring entryName() const;
    static void setEntryName(const std::wstring &aName);

    static EDeviceType::Enum deviceType();
    static void setDeviceType(EDeviceType::Enum aType);

    std::wstring deviceName() const;
    static void setDeviceName(const std::wstring &aName);

    std::wstring phonebookPath() const;
    static void setPhonebookPath(const std::wstring &aPath);

    size_t subEntryIndex() const;
    void setSubEntryIndex(size_t aIndex);

    GUID entryGuid() const;
    void setEntryGuid(const GUID &aGuid);

    bool isSystem() const;
    static void setIsSystem(bool aIsSystem);

    bool isGlobalCredsUsed() const;
    static void setIsGlobalCredsUsed(bool aIsGlobalCredsUsed);

    LUID localSessionId() const;
    void setLocalSessionId(const LUID &aId);

private:
    RASCONN m_Connection{};
};

//------------------------------------------------------------------------------
/// Сетевое устройство.
class Device {
public:
    Device();
    explicit Device(const RASDEVINFO &aDevice);

    operator RASDEVINFO *();

    std::wstring type() const;
    static void setType(const std::wstring &aType);

    std::wstring name() const;
    static void setName(const std::wstring &aName);

private:
    RASDEVINFO m_Device{};
};

//------------------------------------------------------------------------------
/// Параметры соединения.
class DialParams {
public:
    DialParams();
    DialParams(const RASDIALPARAMS &aParams);

    operator RASDIALPARAMS *();

    std::wstring entryName() const;
    static void setEntryName(const std::wstring &aName);

    std::wstring phoneNumber() const;
    static void setPhoneNumber(const std::wstring &aNumber);

    std::wstring callbackNumber() const;
    static void setCallbackNumber(const std::wstring &aNumber);

    std::wstring userName() const;
    static void setUserName(const std::wstring &aName);

    std::wstring password() const;
    static void setPassword(const std::wstring &aPassword);

    std::wstring domain() const;
    static void setDomain(const std::wstring &aDomain);

    unsigned int subEntry() const;
    void setSubEntry(unsigned int aIndex);

    unsigned long callbackId() const;
    void setCallbackId(unsigned long aValue);

    bool hasSavedPassword() const;
    void setHasSavedPassword(bool aHasPassword);

    bool removePassword() const;
    void setRemovePassword(bool aRemove);

private:
    RASDIALPARAMS m_Params{};
    bool m_HasSavedPassword;
    bool m_RemovePassword;
};

//------------------------------------------------------------------------------
/// Перечислитель элементов телефонной книги.
class PhonebookEntryEnumerator : public RasBase {
public:
    explicit PhonebookEntryEnumerator(const std::wstring &aPhonebookPath = L"");
    ~PhonebookEntryEnumerator();

    static bool getEntry(PhonebookEntryName &aEntry);
    void reset(const std::wstring &aPhonebookPath = L"");

private:
    DWORD m_EntryCount{};
    size_t m_CurrentIndex{};
    DWORD m_RequestedBufSize{};
    LPRASENTRYNAME m_Entries{};
};

//------------------------------------------------------------------------------
/// Перечислитель сетевых соединений в системе.
class ConnectionEnumerator : public RasBase {
public:
    ConnectionEnumerator();
    ~ConnectionEnumerator();

    static bool getConnection(Connection &aConnection);
    void reset();

private:
    DWORD m_ConnectionCount{};
    size_t m_CurrentIndex{};
    DWORD m_RequestedBufSize{};
    LPRASCONN m_Connections{};
};

//------------------------------------------------------------------------------
/// Перечислитель сетевых устройств в системе.
class DeviceEnumerator : public RasBase {
public:
    DeviceEnumerator();
    ~DeviceEnumerator();

    static bool getDevice(Device &aDevice);
    void reset();

private:
    DWORD m_DeviceCount{};
    size_t m_CurrentIndex{};
    DWORD m_RequestedBufSize{};
    LPRASDEVINFO m_Devices{};
};

//------------------------------------------------------------------------------
DWORD ValidatePhonebookEntryName(PhonebookEntryName &aEntry);

//------------------------------------------------------------------------------
DWORD CreateNewPhonebookEntry(const PhonebookEntryName &aEntryName, PhonebookEntry &aEntry);

//------------------------------------------------------------------------------
DWORD GetConnectionStatus(const std::wstring &aConnectionName, EConnectionStatus::Enum &aStatus);

//------------------------------------------------------------------------------
DWORD GetEntryProperties(const PhonebookEntryName &aEntryName, PhonebookEntry &aEntry);

//------------------------------------------------------------------------------
DWORD GetEntryDialParams(const PhonebookEntryName &aEntryName, DialParams &aParameters);

DWORD SetEntryDialParams(const PhonebookEntryName &aEntryName, DialParams &aParams);

//------------------------------------------------------------------------------
DWORD Dial(const PhonebookEntryName &aEntryName);

//------------------------------------------------------------------------------
DWORD HangUp(const std::wstring &aConnectionName);

//------------------------------------------------------------------------------
DWORD RemovePhonebookEntry(const PhonebookEntryName &aEntryName);

//------------------------------------------------------------------------------
std::wstring getAttachedTo(const std::wstring &aDeviceName);

//------------------------------------------------------------------------------

} // namespace RasApi

//------------------------------------------------------------------------------
