/* @file Главные константы драйверов, используются верхней логике, могут использоваться также в
 * драйверах и плагинах. */

#pragma once

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
namespace CAllHardware {
/// Общие константы.
extern const char RequiredDevice[];
extern const char DeviceData[];
extern const char ModelName[];
extern const char ProtocolName[];
extern const char DetectingPriority[];
extern const char Mask[];
extern const char System_Name[];
extern const char SearchingType[];
extern const char RequiredResource[];
extern const char Existence[];
extern const char InteractionType[];
extern const char WaitUpdatingTimeout[];
extern const char OperatorPresence[];
extern const char FiscalServerPresence[];
extern const char OptionalPortSettings[];
extern const char OptionalPortSettingsEnable[];
extern const char SerialNumber[];
extern const char CanOnline[];
extern const char LibraryVersion[];

/// Значения настроек.
namespace Values {
extern const char Use[];
extern const char NotUse[];
extern const char Auto[];
} // namespace Values

/// Типы поиска устройств.
namespace SearchingTypes {
extern const char Loading[];
extern const char AutoDetecting[];
} // namespace SearchingTypes

/// Варианты нахождения устройств.
namespace ExistenceTypes {
extern const char Unique[];
extern const char Multiple[];
} // namespace ExistenceTypes

/// Константы устройств приема денег.
namespace CashAcceptor {
extern const char System_CurrencyId[];
} // namespace CashAcceptor

/// Константы фискального регистратора.
namespace FR {
extern const char FSSerialNumber[];
extern const char TaxSystems[];
extern const char AgentFlags[];
extern const char AgentFlagsData[];
extern const char SectionNames[];
extern const char DealerTaxSystem[];
extern const char DealerAgentFlag[];
extern const char DealerVAT[];
extern const char DealerSupportPhone[];
extern const char UserPhone[];
extern const char UserMail[];
extern const char ZReportTime[];
extern const char FiscalFieldData[];
extern const char WithoutPrinting[];
extern const char CanWithoutPrinting[];
extern const char NullingSum_InCash[];
} // namespace FR

/// Константы принтера.
namespace Printer {
extern const char LineSize[];
extern const char ReceiptTemplate[];
extern const char PrintingMode[];
extern const char ServiceOperation[];
extern const char BlockTerminalOnError[];
extern const char OFDNotSentError[];
} // namespace Printer

/// Константы HID-устройств.
namespace HID {
/// Аттрибуты, передаваемые в сигнале о новых введённых данных
extern const char Text[];              // QString
extern const char Image[];             // QImage
extern const char FaceDetected[];      // bool
extern const char ImageWithFaceArea[]; // QImage
} // namespace HID

/// Константы порта.
namespace Port {
/// Константы TCP-порта.
namespace TCP {
extern const char IP[];
extern const char Number[];
} // namespace TCP
} // namespace Port
} // namespace CAllHardware

} // namespace Driver
} // namespace SDK

namespace CHardwareSDK = SDK::Driver::CAllHardware;

//---------------------------------------------------------------------------
