/* @file Инициализация констант. */

#include <QtCore/QEvent>

#include <SDK/Drivers/ICardReader.h>
#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/Drivers/IDispenser.h>
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/IHID.h>
#include <SDK/Drivers/IWatchdog.h>

namespace SDK {
namespace Driver {

const char *IDevice::StatusSignal =
    SIGNAL(status(SDK::Driver::EWarningLevel::Enum, const QString &, int));
const char *IDevice::InitializedSignal = SIGNAL(initialized());
const char *IDevice::UpdatedSignal = SIGNAL(updated(bool));
const char *IDevice::ConfigurationChangedSignal = SIGNAL(configurationChanged());

const char *ICashAcceptor::EscrowSignal = SIGNAL(escrow(SDK::Driver::SPar));
const char *ICashAcceptor::StackedSignal = SIGNAL(stacked(SDK::Driver::TParList));

const char *IHID::DataSignal = SIGNAL(data(const QVariantMap &));

const char *IFiscalPrinter::FRSessionClosedSignal = SIGNAL(FRSessionClosed(const QVariantMap &));

const char *ICardReader::InsertedSignal =
    SIGNAL(inserted(SDK::Driver::ECardType::Enum, const QVariantMap &));
const char *ICardReader::EjectedSignal = SIGNAL(ejected());

const char *IDispenser::DispensedSignal = SIGNAL(dispensed(int, int));
const char *IDispenser::RejectedSignal = SIGNAL(rejected(int, int));
const char *IDispenser::UnitEmptySignal = SIGNAL(unitEmpty(int));
const char *IDispenser::UnitsDefinedSignal = SIGNAL(unitsDefined());

const char *IWatchdog::KeyRegisteredSignal = SIGNAL(keyRegistered(bool));

/// Регистрация своих типов.

int Type1 = qRegisterMetaType<SDK::Driver::EWarningLevel::Enum>();
int Type2 = qRegisterMetaType<SDK::Driver::ECashAcceptorStatus::Enum>();
int Type4 = qRegisterMetaType<SDK::Driver::SPar>();
int Type5 = qRegisterMetaType<SDK::Driver::TParList>();
int Type6 = qRegisterMetaType<SDK::Driver::ECardType::Enum>();

/// Hardware constants definitions
namespace CAllHardware {

// Общие константы.
const char RequiredDevice[] = "required_device";
const char DeviceData[] = "device_data";
const char ModelName[] = "model_name";
const char ProtocolName[] = "protocol_name";
const char DetectingPriority[] = "detect_priority";
const char Mask[] = "mask";
const char SystemName[] = "system_name";
const char SearchingType[] = "searching_type";
const char RequiredResource[] = "required_resource";
const char Existence[] = "existence";
const char InteractionType[] = "interaction_type";
const char WaitUpdatingTimeout[] = "wait_updating_timeout";
const char OperatorPresence[] = "operator_presence";
const char FiscalServerPresence[] = "fiscal_server_presence";
const char OptionalPortSettings[] = "optional_port_settings";
const char OptionalPortSettingsEnable[] = "optional_port_settings_enable";
const char SerialNumber[] = "serial_number";
const char CanOnline[] = "can_online";
const char LibraryVersion[] = "library_version";

// Значения настроек.
namespace Values {
const char Use[] = "use";
const char NotUse[] = "not_use";
const char Auto[] = "auto";
} // namespace Values

// Типы поиска устройств.
namespace SearchingTypes {
const char Loading[] = "loading";
const char AutoDetecting[] = "auto_detecting";
} // namespace SearchingTypes

// Варианты нахождения устройств.
namespace ExistenceTypes {
const char Unique[] = "unique";
const char Multiple[] = "multiple";
} // namespace ExistenceTypes

// Константы устройств приема денег.
namespace CashAcceptor {
const char SystemCurrencyId[] = "system_currency_id";
} // namespace CashAcceptor

// Константы фискального регистратора.
namespace FR {
const char FSSerialNumber[] = "fs_serial_number";
const char TaxSystems[] = "tax_systems";
const char AgentFlags[] = "agent_flags";
const char AgentFlagsData[] = "agent_flags_data";
const char SectionNames[] = "section_names";
const char DealerTaxSystem[] = "dealer_tax_system";
const char DealerAgentFlag[] = "dealer_agent_flag";
const char DealerVAT[] = "dealer_vat";
const char DealerSupportPhone[] = "dealer_support_phone";
const char UserPhone[] = "user_phone";
const char UserMail[] = "user_mail";
const char ZReportTime[] = "z_report_time";
const char FiscalFieldData[] = "fiscal_field_data";
const char WithoutPrinting[] = "without_printing";
const char CanWithoutPrinting[] = "can_without_printing";
const char NullingSumInCash[] = "nulling_sum_in_cash";
} // namespace FR

// Константы принтера.
namespace Printer {
const char LineSize[] = "line_size";
const char ReceiptTemplate[] = "receipt_template";
const char PrintingMode[] = "printing_mode";
const char ServiceOperation[] = "service_operation";
const char BlockTerminalOnError[] = "block_terminal_on_error";
const char OFDNotSentError[] = "ofd_not_sent_error";
} // namespace Printer

// Константы HID-устройств.
namespace HID {
const char Text[] = "text";                         // QString
const char Image[] = "image";                       // QImage
const char FaceDetected[] = "face_detected";        // bool
const char ImageWithFaceArea[] = "image_with_face"; // QImage
} // namespace HID

// Константы порта.
namespace Port {
namespace TCP {
const char IP[] = "ip";
const char Number[] = "port_number";
} // namespace TCP
} // namespace Port

// Fiscal Fields definitions
namespace FiscalFields {

// Basic fields
const char FDName[] = "fd_name";
const char UserContact[] = "user_contact";
const char PayOffAddress[] = "payoff_address";
const char FDDateTime[] = "fd_date_time";
const char SerialFRNumber[] = "serial_fr_number";
const char OFDINN[] = "ofd_inn";
const char INN[] = "inn";
const char PayOffAmount[] = "payoff_amount";
const char Cashier[] = "cashier";
const char UnitName[] = "unit_name";
const char AutomaticNumber[] = "automatic_number";
const char RNM[] = "rnm";
const char SessionNumber[] = "session_number";
const char FDNumber[] = "fd_number";
const char SerialFSNumber[] = "serial_fs_number";
const char DocumentNumber[] = "document_number";
const char OFDName[] = "ofd_name";
const char LegalOwner[] = "legal_owner";
const char PayOffType[] = "payoff_type";
const char TaxSystem[] = "tax_system";
const char FTSURL[] = "fts_url";
const char TaxSystemsReg[] = "tax_systems_reg";
const char ProcessingPhone[] = "processing_phone";
const char FDSign[] = "fd_sign";
const char OFDNotSentFDQuantity[] = "ofd_not_sent_fd_quantity";
const char OFDNotSentFDDateTime[] = "ofd_not_sent_fd_date_time";
const char ReregistrationCause[] = "reregistration_cause";
const char FDForSessionTotal[] = "fd_for_session_total";
const char SenderMail[] = "sender_mail";
const char FiscalsForSessionTotal[] = "fiscals_for_session_total";
const char PayOffPlace[] = "payoff_place";
const char ModelVersion[] = "model_version";
const char FFDFR[] = "ffd_fr";
const char FFDFS[] = "ffd_fs";
const char VATRate[] = "vat_rate";
const char CashierINN[] = "cashier_inn";
const char OFDURL[] = "ofd_url";
const char FFD[] = "ffd";

// Transfer operator data
const char TransferOperatorAddress[] = "transfer_operator_address";
const char TransferOperatorINN[] = "transfer_operator_inn";
const char TransferOperatorName[] = "transfer_operator_name";
const char TransferOperatorPhone[] = "transfer_operator_phone";

// Provider data
const char ProviderPhone[] = "provider_phone";
const char ProviderINN[] = "provider_inn";

// Payment agent data
const char AgentOperation[] = "agent_operation";
const char AgentFlagsReg[] = "agent_flags_reg";
const char AgentPhone[] = "agent_phone";
const char AgentFlag[] = "agent_flag";

// Statuses
const char FSExpiredStatus[] = "fs_expired_status";
const char FSNeedChangeStatus[] = "fs_need_change_status";
const char FSMemoryEnd[] = "fs_memory_end_status";
const char OFDNoConnection[] = "ofd_no_connection_status";

// Operating modes
const char AutomaticMode[] = "automatic_mode";
const char AutonomousMode[] = "autonomous_mode";
const char EncryptionMode[] = "encryption_mode";
const char InternetMode[] = "internet_mode";
const char ServiceAreaMode[] = "service_area_mode";
const char FixedReportingMode[] = "fixed_reporting_mode";
const char LotteryMode[] = "lottery_mode";
const char GamblingMode[] = "gambling_mode";
const char ExcisableUnitMode[] = "excisable_unit_mode";
const char InAutomateMode[] = "in_automate_mode";

// Payoff subject (per item)
const char PayOffSubjectQuantity[] = "payoff_subject_quantity";
const char PayOffSubjectAmount[] = "payoff_subject_amount";
const char PayOffSubject[] = "payoff_subject";
const char PayOffSubjectUnitPrice[] = "payoff_subject_unit_price";
const char PayOffSubjectTaxAmount[] = "payoff_subject_tax_amount";
const char PayOffSubjectType[] = "payoff_subject_type";
const char PayOffSubjectMethodType[] = "payoff_subject_method_type";

// Fiscal totals by payment method (per receipt)
const char CashFiscalTotal[] = "cash_fiscal_total";
const char CardFiscalTotal[] = "card_fiscal_total";
const char PrePaymentFiscalTotal[] = "prepayment_fiscal_total";
const char PostPaymentFiscalTotal[] = "postpayment_fiscal_total";
const char CounterOfferFiscalTotal[] = "counter_offer_fiscal_total";

// Taxes (per receipt)
const char TaxAmount02[] = "tax_amount_02";
const char TaxAmount03[] = "tax_amount_03";
const char TaxAmount04[] = "tax_amount_04";
const char TaxAmount05[] = "tax_amount_05";
const char TaxAmount06[] = "tax_amount_06";
const char TaxAmount07[] = "tax_amount_07";

// Values
namespace Values {
const char NoData[] = "none";
} // namespace Values

} // namespace FiscalFields

} // namespace CAllHardware

// Application component
const char Application[] = "Common";

// Components definitions
namespace CComponents {
const char Driver[] = "Driver";
const char IOPort[] = "IOPort";
const char BillAcceptor[] = "BillAcceptor";
const char CoinAcceptor[] = "CoinAcceptor";
const char Dispenser[] = "Dispenser";
const char Printer[] = "Printer";
const char FiscalRegistrator[] = "FiscalRegistrator";
const char DocumentPrinter[] = "DocumentPrinter";
const char Watchdog[] = "Watchdog";
const char Modem[] = "Modem";
const char Scanner[] = "Scanner";
const char CardReader[] = "CardReader";
const char Health[] = "Health";
const char Camera[] = "Camera";
const char Token[] = "Token";
} // namespace CComponents

//---------------------------------------------------------------------------
// Definitions for CInteractionTypes constants
namespace CInteractionTypes {
const char COM[] = "COM";
const char USB[] = "USB";
const char LibUSB[] = "LibUSB";
const char TCP[] = "TCP";
const char OPOS[] = "OPOS";
const char System[] = "System";
const char External[] = "External";
} // namespace CInteractionTypes

} // namespace Driver
} // namespace SDK
