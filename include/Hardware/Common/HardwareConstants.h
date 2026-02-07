/* @file Константы драйверов. */

#pragma once

#include <SDK/Drivers/HardwareConstants.h>

//---------------------------------------------------------------------------
namespace CHardware {
/// Общие константы.
extern const char OPOSName[];
extern const char Codepage[];
extern const char CallingType[];
extern const char AutomaticStatus[];
extern const char UpdatingFilenameExtension[];
extern const char PluginParameterNames[];
extern const char RequiredResourceNames[];
extern const char PluginPath[];
extern const char ConfigData[];
extern const char CanSoftReboot[];
extern const char ProtocolType[];

/// Типы вызова функционала драйвера.
namespace CallingTypes {
extern const char Internal[];
extern const char External[];
} // namespace CallingTypes

/// Типы устройств.
namespace Types {
extern const char CashAcceptor[];
extern const char BillAcceptor[];
extern const char Dispenser[];
extern const char CoinAcceptor[];
extern const char DualCashAcceptor[];
} // namespace Types

/// Кодировки.
namespace Codepages {
extern const char CP850[];
extern const char CP866[];
extern const char Win1250[];
extern const char Win1251[];
extern const char Win1252[];
extern const char Base[];
extern const char SPARK[];
extern const char ATOL[];
extern const char CustomKZT[];
} // namespace Codepages

/// Константы порта.
namespace Port {
extern const char IOLogging[];
extern const char DeviceModelName[];
extern const char MaxReadingSize[];
extern const char OpeningTimeout[];
extern const char OpeningContext[];
extern const char Suspended[];
extern const char JustConnected[];

/// Константы COM-порта.
namespace COM {
extern const char BaudRate[];
extern const char Parity[];
extern const char ByteSize[];
extern const char StopBits[];
extern const char RTS[];
extern const char DTR[];
extern const char WaitResult[];
extern const char ControlRemoving[];
} // namespace COM

/// Константы USB-порта.
namespace USB {
extern const char VID[];
extern const char PID[];
} // namespace USB
} // namespace Port

/// Константы кардридера.
namespace CardReader {
extern const char Track1[];
extern const char Track2[];
extern const char Track3[];
} // namespace CardReader

/// Константы устройства работы с деньгами.
namespace CashDevice {
/// Типы протокола CCTalk.
namespace CCTalkTypes {
extern const char CRC8[];
extern const char CRC16[];
extern const char CRC16Encrypted[];
} // namespace CCTalkTypes
} // namespace CashDevice

/// Константы устройства приема денег.
namespace CashAcceptor {
extern const char SecurityLevel[];
extern const char ParTable[];
extern const char Enabled[];
extern const char OnlyDefferedDisable[];
extern const char DisablingTimeout[];
extern const char InitializeTimeout[];
extern const char ProcessEnabling[];
extern const char ProcessDisabling[];
extern const char StackedFilter[];
} // namespace CashAcceptor

/// Константы виртуального устройства приема денег.
namespace VirtualCashAcceptor {
extern const char NotesPerEscrow[];
} // namespace VirtualCashAcceptor

/// Константы диспенсера.
namespace Dispenser {
extern const char Units[];
extern const char JammedItem[];
extern const char NearEndCount[];
} // namespace Dispenser

/// Константы сканера.
namespace Scanner {
extern const char Prefix[];
} // namespace Scanner

/// Константы принтера.
namespace Printer {
extern const char FeedingAmount[];
extern const char NeedCutting[];
extern const char NeedSeparating[];
extern const char ByteString[];
extern const char Receipt[];
extern const char PresenterEnable[];
extern const char RetractorEnable[];
extern const char PresenterStatusEnable[];
extern const char VerticalMountMode[];
extern const char AutoRetractionTimeout[];
extern const char BlackMark[];
extern const char PowerOnReaction[];
extern const char OutCall[];
extern const char LineSize[];
extern const char PrintingMode[];

/// Команды.
namespace Commands {
extern const char Cutting[];
extern const char Presentation[];
extern const char Pushing[];
extern const char Retraction[];
} // namespace Commands

/// Настройки для плагина.
namespace Values {
extern const char Cut[];
extern const char Retract[];
extern const char Push[];
extern const char Present[];
} // namespace Values

/// Настройки для плагина.
namespace Settings {
extern const char NotTakenReceipt[];
extern const char PreviousReceipt[];
extern const char PreviousAndNotTakenReceipts[];
extern const char LeftReceiptTimeout[];
extern const char FontSize[];
extern const char LineSpacing[];
extern const char FeedingFactor[];
extern const char PresentationLength[];
extern const char Loop[];
extern const char Hold[];
extern const char Ejector[];
extern const char RemotePaperSensor[];
extern const char PaperJamSensor[];
extern const char PaperWeightSensors[];
extern const char DocumentCap[];
extern const char BackFeed[];
extern const char PrintPageNumber[];
extern const char LeftMargin[];
extern const char RightMargin[];
} // namespace Settings

/// Параметры обработки чека после отрезки.
namespace EjectorMode {
extern const char Presenting[];
extern const char Printing[];
extern const char Action[];
} // namespace EjectorMode
} // namespace Printer

/// Константы сторожевого таймера.
namespace Watchdog {
extern const char CanRegisterKey[];
extern const char CanWakeUpPC[];
extern const char PCWakingUpTime[];

namespace Sensor {
extern const char Safe[];            /// Сейф.
extern const char UpperUnit[]; /// Верхний модуль.
extern const char LowerUnit[]; /// Верхний модуль.

/// Настройки срабатывания датчиков.
namespace Action {
extern const char Safe[];            /// Сейф.
extern const char UpperUnit[]; /// Верхний модуль.
extern const char LowerUnit[]; /// Верхний модуль.
} // namespace Action

/// Действия при срабатывании датчика.
namespace ActionValue {
extern const char EnterServiceMenu[]; /// Войти в сервисное меню.
extern const char LockTerminal[];          /// Заблокировать терминал.
} // namespace ActionValue
} // namespace Sensor
} // namespace Watchdog

/// Константы фискального регистратора.
namespace FR {
extern const char EjectorParameters[];
extern const char FiscalMode[];
extern const char CanAutoCloseSession[];
extern const char FiscalChequeCreation[];
extern const char SessionOpeningTime[];
extern const char Amount[];
extern const char StartZReportNumber[];
extern const char ZReportNumber[];
extern const char EKLZRequestType[];
extern const char EKLZData[];
extern const char EKLZStatus[];
extern const char CVCNumber[];
extern const char ForcePerformZReport[];
extern const char PrinterModel[];
extern const char CanZReportWithoutPrinting[];

/// Команды.
namespace Commands {
extern const char PrintingDeferredZReports[];
} // namespace Commands

/// Варианты использования настроек.
namespace Values {
extern const char Adaptive[];
extern const char Discrete[];
extern const char LoopAndPushNotTakenOnTimeout[];
extern const char NoLoopAndPushNotTakenOnTimeout[];
extern const char NoLoopAndRetractNotTakenOnTimeout[];
} // namespace Values

namespace Strings {
extern const char Payment[];
extern const char Depositing[];
extern const char INN[];
extern const char SerialNumber[];
extern const char DocumentNumber[];
extern const char Amount[];
extern const char Date[];
extern const char Time[];
extern const char Session[];
extern const char Cashier[];
extern const char ReceiptNumber[];
extern const char Total[];
extern const char WithoutTaxes[];
} // namespace Strings

namespace DocumentCapData {
extern const char DealerName[];
extern const char DealerAddress[];
extern const char DealerSupportPhone[];
extern const char PointAddress[];
} // namespace DocumentCapData
} // namespace FR

/// Константы веб-камеры.
namespace WebCamera {
extern const char FaceDetection[];
extern const char FPS[];
} // namespace WebCamera
} // namespace CHardware

//---------------------------------------------------------------------------
