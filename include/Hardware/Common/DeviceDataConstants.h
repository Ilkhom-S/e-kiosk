/* @file Константы данных устройств. */

#pragma once

//---------------------------------------------------------------------------
namespace CDeviceData {
/// Общие константы.
extern const char ModelName[];
extern const char Port[];
extern const char Name[];
extern const char Driver[];
extern const char ModelKey[];
extern const char ModelNumber[];
extern const char Revision[];
extern const char Firmware[];
extern const char BootFirmware[];
extern const char CheckSum[];
extern const char SerialNumber[];
extern const char BoardVersion[];
extern const char Build[];
extern const char Switches[];
extern const char Memory[];
extern const char Version[];
extern const char Type[];
extern const char Address[];
extern const char Date[];
extern const char FirmwareUpdatable[];
extern const char ProjectNumber[];
extern const char ProductCode[];
extern const char Vendor[];
extern const char ProtocolVersion[];
extern const char SDCard[];
extern const char Error[];
extern const char NotConnected[];
extern const char Identity[];
extern const char Token[];
extern const char InternalFirmware[];
extern const char InternalHardware[];
extern const char ControllerBuild[];
extern const char Count[];
extern const char Number[];

/// Общие значения.
namespace Values {
extern const char Yes[];
extern const char No[];
extern const char Opened[];
extern const char Closed[];
extern const char Disabled[];
extern const char Enabled[];
extern const char Absent[];
} // namespace Values

/// Порты.
namespace Ports {
extern const char Mine[];
extern const char Other[];

namespace USB {
extern const char ConfigAmount[];
extern const char BusNumber[];
extern const char Address[];
extern const char PortNumber[];
extern const char ConfigData[];
extern const char Specification[];
extern const char FirmwareVersion[];
extern const char Code[];
extern const char Description[];
extern const char EP0PacketSize[];
extern const char Vendor[];
extern const char Product[];
extern const char BOSData[];

namespace BOS {
extern const char Capability[];

namespace Capability2_0 {
extern const char Attributes[];
} // namespace Capability2_0

namespace Capability3_0 {
extern const char Attributes[];
extern const char SpeedSupported[];
extern const char FunctionalitySupport[];
extern const char U1ExitLatency[];
extern const char U2ExitLatency[];
} // namespace Capability3_0
} // namespace BOS

namespace Config {
extern const char InterfaceAmount[];
extern const char InterfaceData[];
extern const char Index[];
extern const char Value[];
extern const char Attributes[];
extern const char MaxPower[];

namespace Interface {
extern const char EndpointAmount[];
extern const char EndpointData[];
extern const char Number[];
extern const char Index[];
extern const char AlternateSetting[];
extern const char Code[];
extern const char Description[];

namespace Endpoint {
extern const char TransferType[];
extern const char IsoSyncType[];
extern const char IsoUsageType[];
extern const char CompanionAmount[];
extern const char CompanionData[];
extern const char Address[];
extern const char Attributes[];
extern const char MaxPacketSize[];
extern const char PollingInterval[];
extern const char SyncRefreshRate[];
extern const char SynchAddress[];

namespace Companion {
extern const char MaxBurstPacketAmount[];
extern const char Attributes[];
extern const char BytesPerInterval[];
} // namespace Companion
} // namespace Endpoint
} // namespace Interface
} // namespace Config
} // namespace USB
} // namespace Ports

/// OPOS-устройства.
namespace OPOS {
extern const char Description[];
extern const char ControlObject[];
extern const char ServiceObject[];
} // namespace OPOS

/// Купюроприемники и монетоприемники.
namespace CashAcceptors {
extern const char AssetNumber[];
extern const char BillSet[];
extern const char Alias[];
extern const char Interface[];
extern const char CountryCode[];
extern const char StackerType[];
extern const char Database[];
extern const char ModificationNumber[];
extern const char LastUpdate[];
} // namespace CashAcceptors

/// Модемы
namespace Modems {
extern const char IMEI[];
extern const char IMSI[];
extern const char SIMNumber[];
extern const char SIMId[];
extern const char GSMCells[];
} // namespace Modems

/// Принтеры
namespace Printers {
extern const char Location[];
extern const char Comment[];
extern const char Server[];
extern const char Share[];
extern const char Unicode[];
extern const char Cutter[];
extern const char LabelPrinting[];
extern const char BMSensor[];
extern const char Font[];
extern const char Presenter[];
extern const char PaperSupply[];
extern const char Codes[];
extern const char PNESensor[];
} // namespace Printers

/// Фискальные регистраторы
namespace FR {
extern const char OnlineMode[];
extern const char INN[];
extern const char RNM[];
extern const char AgentFlags[];
extern const char TaxSystems[];
extern const char FFDFR[];
extern const char FFDFS[];
extern const char OFDServer[];
extern const char TotalPaySum[];
extern const char Session[];
extern const char OwnerId[];
extern const char ReregistrationNumber[];
extern const char FreeReregistrations[];
extern const char LastRegistrationDate[];
extern const char Activated[];
extern const char Language[];
extern const char CurrentDate[];
extern const char FutureClosingDate[];
extern const char LastClosingDate[];
extern const char OpeningDate[];
extern const char FiscalDocuments[];
extern const char NonFiscalDocuments[];
extern const char Printer[];
extern const char EKLZ[];
extern const char OperationModes[];
extern const char AutomaticNumber[];
extern const char DTDBuild[]; // data transfer device
extern const char CanProcessZBuffer[];
extern const char Taxes[];
extern const char Taxes2019Applied[];
} // namespace FR

/// ЭКЛЗ.
namespace EKLZ {
extern const char Serial[];
extern const char ActivizationDate[];
extern const char FreeActivizations[];
} // namespace EKLZ

/// ФП.
namespace FM {
extern const char FreeSessions[];
extern const char Firmware[];
} // namespace FM

/// ФН.
namespace FS {
extern const char SerialNumber[];
extern const char ValidityData[];
extern const char DifferenceDT[];
extern const char Version[];
extern const char Expiration[];
extern const char FFD[];
extern const char Provider[];
extern const char Revision[];
} // namespace FS

/// Сторожевые таймеры.
namespace Watchdogs {
extern const char Key[];
extern const char CanWakeUpPC[];
extern const char PowerControlLogic[];
extern const char AdvancedPowerLogic[];

namespace Sub {
extern const char All[];
extern const char CrossUSBCard[];
extern const char PowerSupply[];
} // namespace Sub
} // namespace Watchdogs

/// PC health.
namespace Health {
extern const char HandleCount[];
extern const char HandleCountAll[];
extern const char CPUTemperature[];
extern const char Antivirus[];
extern const char Firewall[];
extern const char Motherboard[];
extern const char CPU[];
extern const char HDD[];
extern const char TimeZone[];
} // namespace Health
} // namespace CDeviceData

//---------------------------------------------------------------------------
