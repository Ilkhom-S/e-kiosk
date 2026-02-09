/* @file Инициализация констант. */

#include <QtCore/QEvent>

#include <Common/PluginConstants.h>

#include <SDK/Drivers/ICardReader.h>
#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/Drivers/IDispenser.h>
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/IHID.h>
#include <SDK/Drivers/IWatchdog.h>

// PPVersion definition
const char CPluginParameters::PPVersion[] = "e_kiosk_version";

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
extern const char RequiredDevice[] = "required_device";
extern const char DeviceData[] = "device_data";
extern const char ModelName[] = "model_name";
extern const char ProtocolName[] = "protocol_name";
extern const char DetectingPriority[] = "detect_priority";
extern const char Mask[] = "mask";
extern const char SystemName[] = "system_name";
extern const char SearchingType[] = "searching_type";
extern const char RequiredResource[] = "required_resource";
extern const char Existence[] = "existence";
extern const char InteractionType[] = "interaction_type";
extern const char WaitUpdatingTimeout[] = "wait_updating_timeout";
extern const char OperatorPresence[] = "operator_presence";
extern const char FiscalServerPresence[] = "fiscal_server_presence";
extern const char OptionalPortSettings[] = "optional_port_settings";
extern const char OptionalPortSettingsEnable[] = "optional_port_settings_enable";
extern const char SerialNumber[] = "serial_number";
extern const char CanOnline[] = "can_online";
extern const char LibraryVersion[] = "library_version";

// Значения настроек.
namespace Values {
extern const char Use[] = "use";
extern const char NotUse[] = "not_use";
extern const char Auto[] = "auto";
} // namespace Values

// Типы поиска устройств.
namespace SearchingTypes {
extern const char Loading[] = "loading";
extern const char AutoDetecting[] = "auto_detecting";
} // namespace SearchingTypes

// Варианты нахождения устройств.
namespace ExistenceTypes {
extern const char Unique[] = "unique";
extern const char Multiple[] = "multiple";
} // namespace ExistenceTypes

// Константы устройств приема денег.
namespace CashAcceptor {
extern const char SystemCurrencyId[] = "system_currency_id";
} // namespace CashAcceptor

// Константы фискального регистратора.
namespace FR {
extern const char FSSerialNumber[] = "fs_serial_number";
extern const char TaxSystems[] = "tax_systems";
extern const char AgentFlags[] = "agent_flags";
extern const char AgentFlagsData[] = "agent_flags_data";
extern const char SectionNames[] = "section_names";
extern const char DealerTaxSystem[] = "dealer_tax_system";
extern const char DealerAgentFlag[] = "dealer_agent_flag";
extern const char DealerVAT[] = "dealer_vat";
extern const char DealerSupportPhone[] = "dealer_support_phone";
extern const char UserPhone[] = "user_phone";
extern const char UserMail[] = "user_mail";
extern const char ZReportTime[] = "z_report_time";
extern const char FiscalFieldData[] = "fiscal_field_data";
extern const char WithoutPrinting[] = "without_printing";
extern const char CanWithoutPrinting[] = "can_without_printing";
extern const char NullingSumInCash[] = "nulling_sum_in_cash";
} // namespace FR

// Константы принтера.
namespace Printer {
extern const char LineSize[] = "line_size";
extern const char ReceiptTemplate[] = "receipt_template";
extern const char PrintingMode[] = "printing_mode";
extern const char ServiceOperation[] = "service_operation";
extern const char BlockTerminalOnError[] = "block_terminal_on_error";
extern const char OFDNotSentError[] = "ofd_not_sent_error";
} // namespace Printer

// Константы HID-устройств.
namespace HID {
extern const char Text[] = "text";                         // QString
extern const char Image[] = "image";                       // QImage
extern const char FaceDetected[] = "face_detected";        // bool
extern const char ImageWithFaceArea[] = "image_with_face"; // QImage
} // namespace HID

// Константы порта.
namespace Port {
namespace TCP {
extern const char IP[] = "ip";
extern const char Number[] = "port_number";
} // namespace TCP
} // namespace Port

// Fiscal Fields definitions
namespace FiscalFields {

// Basic fields
extern const char FDName[] = "fd_name";
extern const char UserContact[] = "user_contact";
extern const char PayOffAddress[] = "payoff_address";
extern const char FDDateTime[] = "fd_date_time";
extern const char SerialFRNumber[] = "serial_fr_number";
extern const char OFDINN[] = "ofd_inn";
extern const char INN[] = "inn";
extern const char PayOffAmount[] = "payoff_amount";
extern const char Cashier[] = "cashier";
extern const char UnitName[] = "unit_name";
extern const char AutomaticNumber[] = "automatic_number";
extern const char RNM[] = "rnm";
extern const char SessionNumber[] = "session_number";
extern const char FDNumber[] = "fd_number";
extern const char SerialFSNumber[] = "serial_fs_number";
extern const char DocumentNumber[] = "document_number";
extern const char OFDName[] = "ofd_name";
extern const char LegalOwner[] = "legal_owner";
extern const char PayOffType[] = "payoff_type";
extern const char TaxSystem[] = "tax_system";
extern const char FTSURL[] = "fts_url";
extern const char TaxSystemsReg[] = "tax_systems_reg";
extern const char ProcessingPhone[] = "processing_phone";
extern const char FDSign[] = "fd_sign";
extern const char OFDNotSentFDQuantity[] = "ofd_not_sent_fd_quantity";
extern const char OFDNotSentFDDateTime[] = "ofd_not_sent_fd_date_time";
extern const char ReregistrationCause[] = "reregistration_cause";
extern const char FDForSessionTotal[] = "fd_for_session_total";
extern const char SenderMail[] = "sender_mail";
extern const char FiscalsForSessionTotal[] = "fiscals_for_session_total";
extern const char PayOffPlace[] = "payoff_place";
extern const char ModelVersion[] = "model_version";
extern const char FFDFR[] = "ffd_fr";
extern const char FFDFS[] = "ffd_fs";
extern const char VATRate[] = "vat_rate";
extern const char CashierINN[] = "cashier_inn";
extern const char OFDURL[] = "ofd_url";
extern const char FFD[] = "ffd";

// Transfer operator data
extern const char TransferOperatorAddress[] = "transfer_operator_address";
extern const char TransferOperatorINN[] = "transfer_operator_inn";
extern const char TransferOperatorName[] = "transfer_operator_name";
extern const char TransferOperatorPhone[] = "transfer_operator_phone";

// Provider data
extern const char ProviderPhone[] = "provider_phone";
extern const char ProviderINN[] = "provider_inn";

// Payment agent data
extern const char AgentOperation[] = "agent_operation";
extern const char AgentFlagsReg[] = "agent_flags_reg";
extern const char AgentPhone[] = "agent_phone";
extern const char AgentFlag[] = "agent_flag";

// Statuses
extern const char FSExpiredStatus[] = "fs_expired_status";
extern const char FSNeedChangeStatus[] = "fs_need_change_status";
extern const char FSMemoryEnd[] = "fs_memory_end_status";
extern const char OFDNoConnection[] = "ofd_no_connection_status";

// Operating modes
extern const char AutomaticMode[] = "automatic_mode";
extern const char AutonomousMode[] = "autonomous_mode";
extern const char EncryptionMode[] = "encryption_mode";
extern const char InternetMode[] = "internet_mode";
extern const char ServiceAreaMode[] = "service_area_mode";
extern const char FixedReportingMode[] = "fixed_reporting_mode";
extern const char LotteryMode[] = "lottery_mode";
extern const char GamblingMode[] = "gambling_mode";
extern const char ExcisableUnitMode[] = "excisable_unit_mode";
extern const char InAutomateMode[] = "in_automate_mode";

// Payoff subject (per item)
extern const char PayOffSubjectQuantity[] = "payoff_subject_quantity";
extern const char PayOffSubjectAmount[] = "payoff_subject_amount";
extern const char PayOffSubject[] = "payoff_subject";
extern const char PayOffSubjectUnitPrice[] = "payoff_subject_unit_price";
extern const char PayOffSubjectTaxAmount[] = "payoff_subject_tax_amount";
extern const char PayOffSubjectType[] = "payoff_subject_type";
extern const char PayOffSubjectMethodType[] = "payoff_subject_method_type";

// Fiscal totals by payment method (per receipt)
extern const char CashFiscalTotal[] = "cash_fiscal_total";
extern const char CardFiscalTotal[] = "card_fiscal_total";
extern const char PrePaymentFiscalTotal[] = "prepayment_fiscal_total";
extern const char PostPaymentFiscalTotal[] = "postpayment_fiscal_total";
extern const char CounterOfferFiscalTotal[] = "counter_offer_fiscal_total";

// Taxes (per receipt)
extern const char TaxAmount02[] = "tax_amount_02";
extern const char TaxAmount03[] = "tax_amount_03";
extern const char TaxAmount04[] = "tax_amount_04";
extern const char TaxAmount05[] = "tax_amount_05";
extern const char TaxAmount06[] = "tax_amount_06";
extern const char TaxAmount07[] = "tax_amount_07";

// Values
namespace Values {
extern const char NoData[] = "none";
} // namespace Values

} // namespace FiscalFields

} // namespace CAllHardware

// Application component
// const char Application[] = "Common";  // Now defined in header

// Components definitions
// namespace CComponents {
// const char Driver[] = "Driver";
// const char IOPort[] = "IOPort";
// const char BillAcceptor[] = "BillAcceptor";
// const char CoinAcceptor[] = "CoinAcceptor";
// const char Dispenser[] = "Dispenser";
// const char Printer[] = "Printer";
// const char FiscalRegistrator[] = "FiscalRegistrator";
// const char DocumentPrinter[] = "DocumentPrinter";
// const char Watchdog[] = "Watchdog";
// const char Modem[] = "Modem";
// const char Scanner[] = "Scanner";
// const char CardReader[] = "CardReader";
// const char Health[] = "Health";
// const char Camera[] = "Camera";
// const char Token[] = "Token";
// } // namespace CComponents

//---------------------------------------------------------------------------
// Definitions for CInteractionTypes constants
namespace CInteractionTypes {
extern const char COM[] = "COM";
extern const char USB[] = "USB";
extern const char LibUSB[] = "LibUSB";
extern const char TCP[] = "TCP";
extern const char OPOS[] = "OPOS";
extern const char System[] = "System";
extern const char External[] = "External";
} // namespace CInteractionTypes

// Dummy function to ensure constants are not optimized away
void dummyReference() {
    volatile const char *dummy = nullptr;
    // Application and CComponents are now constexpr in header
    // dummy = Application;
    // CComponents
    // dummy = CComponents::Driver;
    // ... etc
    // CInteractionTypes
    dummy = CInteractionTypes::COM;
    dummy = CInteractionTypes::USB;
    dummy = CInteractionTypes::LibUSB;
    dummy = CInteractionTypes::TCP;
    dummy = CInteractionTypes::OPOS;
    dummy = CInteractionTypes::System;
    dummy = CInteractionTypes::External;
    // CAllHardware main constants
    dummy = CAllHardware::RequiredDevice;
    dummy = CAllHardware::DeviceData;
    dummy = CAllHardware::ModelName;
    dummy = CAllHardware::ProtocolName;
    dummy = CAllHardware::DetectingPriority;
    dummy = CAllHardware::Mask;
    dummy = CAllHardware::SystemName;
    dummy = CAllHardware::SearchingType;
    dummy = CAllHardware::RequiredResource;
    dummy = CAllHardware::Existence;
    dummy = CAllHardware::InteractionType;
    dummy = CAllHardware::WaitUpdatingTimeout;
    dummy = CAllHardware::OperatorPresence;
    dummy = CAllHardware::FiscalServerPresence;
    dummy = CAllHardware::OptionalPortSettings;
    dummy = CAllHardware::OptionalPortSettingsEnable;
    dummy = CAllHardware::SerialNumber;
    dummy = CAllHardware::CanOnline;
    dummy = CAllHardware::LibraryVersion;
    // CAllHardware::Values
    dummy = CAllHardware::Values::Use;
    dummy = CAllHardware::Values::NotUse;
    dummy = CAllHardware::Values::Auto;
    // CAllHardware::SearchingTypes
    dummy = CAllHardware::SearchingTypes::Loading;
    dummy = CAllHardware::SearchingTypes::AutoDetecting;
    // CAllHardware::ExistenceTypes
    dummy = CAllHardware::ExistenceTypes::Unique;
    dummy = CAllHardware::ExistenceTypes::Multiple;
    // CAllHardware::CashAcceptor
    dummy = CAllHardware::CashAcceptor::SystemCurrencyId;
    // CAllHardware::FR
    dummy = CAllHardware::FR::FSSerialNumber;
    dummy = CAllHardware::FR::TaxSystems;
    dummy = CAllHardware::FR::AgentFlags;
    dummy = CAllHardware::FR::AgentFlagsData;
    dummy = CAllHardware::FR::SectionNames;
    dummy = CAllHardware::FR::DealerTaxSystem;
    dummy = CAllHardware::FR::DealerAgentFlag;
    dummy = CAllHardware::FR::DealerVAT;
    dummy = CAllHardware::FR::DealerSupportPhone;
    dummy = CAllHardware::FR::UserPhone;
    dummy = CAllHardware::FR::UserMail;
    dummy = CAllHardware::FR::ZReportTime;
    dummy = CAllHardware::FR::FiscalFieldData;
    dummy = CAllHardware::FR::WithoutPrinting;
    dummy = CAllHardware::FR::CanWithoutPrinting;
    dummy = CAllHardware::FR::NullingSumInCash;
    // CAllHardware::Printer
    dummy = CAllHardware::Printer::LineSize;
    dummy = CAllHardware::Printer::ReceiptTemplate;
    dummy = CAllHardware::Printer::PrintingMode;
    dummy = CAllHardware::Printer::ServiceOperation;
    dummy = CAllHardware::Printer::BlockTerminalOnError;
    dummy = CAllHardware::Printer::OFDNotSentError;
    // CAllHardware::HID
    dummy = CAllHardware::HID::Text;
    dummy = CAllHardware::HID::Image;
    dummy = CAllHardware::HID::FaceDetected;
    dummy = CAllHardware::HID::ImageWithFaceArea;
    // CAllHardware::Port::TCP
    dummy = CAllHardware::Port::TCP::IP;
    dummy = CAllHardware::Port::TCP::Number;
    // CAllHardware::FiscalFields basic fields
    dummy = CAllHardware::FiscalFields::FDName;
    dummy = CAllHardware::FiscalFields::UserContact;
    dummy = CAllHardware::FiscalFields::PayOffAddress;
    dummy = CAllHardware::FiscalFields::FDDateTime;
    dummy = CAllHardware::FiscalFields::SerialFRNumber;
    dummy = CAllHardware::FiscalFields::OFDINN;
    dummy = CAllHardware::FiscalFields::INN;
    dummy = CAllHardware::FiscalFields::PayOffAmount;
    dummy = CAllHardware::FiscalFields::Cashier;
    dummy = CAllHardware::FiscalFields::UnitName;
    dummy = CAllHardware::FiscalFields::AutomaticNumber;
    dummy = CAllHardware::FiscalFields::RNM;
    dummy = CAllHardware::FiscalFields::SessionNumber;
    dummy = CAllHardware::FiscalFields::FDNumber;
    dummy = CAllHardware::FiscalFields::SerialFSNumber;
    dummy = CAllHardware::FiscalFields::DocumentNumber;
    dummy = CAllHardware::FiscalFields::OFDName;
    dummy = CAllHardware::FiscalFields::LegalOwner;
    dummy = CAllHardware::FiscalFields::PayOffType;
    dummy = CAllHardware::FiscalFields::TaxSystem;
    dummy = CAllHardware::FiscalFields::FTSURL;
    dummy = CAllHardware::FiscalFields::TaxSystemsReg;
    dummy = CAllHardware::FiscalFields::ProcessingPhone;
    dummy = CAllHardware::FiscalFields::FDSign;
    dummy = CAllHardware::FiscalFields::OFDNotSentFDQuantity;
    dummy = CAllHardware::FiscalFields::OFDNotSentFDDateTime;
    dummy = CAllHardware::FiscalFields::ReregistrationCause;
    dummy = CAllHardware::FiscalFields::FDForSessionTotal;
    dummy = CAllHardware::FiscalFields::SenderMail;
    dummy = CAllHardware::FiscalFields::FiscalsForSessionTotal;
    dummy = CAllHardware::FiscalFields::PayOffPlace;
    dummy = CAllHardware::FiscalFields::ModelVersion;
    dummy = CAllHardware::FiscalFields::FFDFR;
    dummy = CAllHardware::FiscalFields::FFDFS;
    dummy = CAllHardware::FiscalFields::VATRate;
    dummy = CAllHardware::FiscalFields::CashierINN;
    dummy = CAllHardware::FiscalFields::OFDURL;
    dummy = CAllHardware::FiscalFields::FFD;
    // CAllHardware::FiscalFields transfer operator data
    dummy = CAllHardware::FiscalFields::TransferOperatorAddress;
    dummy = CAllHardware::FiscalFields::TransferOperatorINN;
    dummy = CAllHardware::FiscalFields::TransferOperatorName;
    dummy = CAllHardware::FiscalFields::TransferOperatorPhone;
    // CAllHardware::FiscalFields provider data
    dummy = CAllHardware::FiscalFields::ProviderPhone;
    dummy = CAllHardware::FiscalFields::ProviderINN;
    // CAllHardware::FiscalFields payment agent data
    dummy = CAllHardware::FiscalFields::AgentOperation;
    dummy = CAllHardware::FiscalFields::AgentFlagsReg;
    dummy = CAllHardware::FiscalFields::AgentPhone;
    dummy = CAllHardware::FiscalFields::AgentFlag;
    // CAllHardware::FiscalFields statuses
    dummy = CAllHardware::FiscalFields::FSExpiredStatus;
    dummy = CAllHardware::FiscalFields::FSNeedChangeStatus;
    dummy = CAllHardware::FiscalFields::FSMemoryEnd;
    dummy = CAllHardware::FiscalFields::OFDNoConnection;
    // CAllHardware::FiscalFields operating modes
    dummy = CAllHardware::FiscalFields::AutomaticMode;
    dummy = CAllHardware::FiscalFields::AutonomousMode;
    dummy = CAllHardware::FiscalFields::EncryptionMode;
    dummy = CAllHardware::FiscalFields::InternetMode;
    dummy = CAllHardware::FiscalFields::ServiceAreaMode;
    dummy = CAllHardware::FiscalFields::FixedReportingMode;
    dummy = CAllHardware::FiscalFields::LotteryMode;
    dummy = CAllHardware::FiscalFields::GamblingMode;
    dummy = CAllHardware::FiscalFields::ExcisableUnitMode;
    dummy = CAllHardware::FiscalFields::InAutomateMode;
    // CAllHardware::FiscalFields payoff subject
    dummy = CAllHardware::FiscalFields::PayOffSubjectQuantity;
    dummy = CAllHardware::FiscalFields::PayOffSubjectAmount;
    dummy = CAllHardware::FiscalFields::PayOffSubject;
    dummy = CAllHardware::FiscalFields::PayOffSubjectUnitPrice;
    dummy = CAllHardware::FiscalFields::PayOffSubjectTaxAmount;
    dummy = CAllHardware::FiscalFields::PayOffSubjectType;
    dummy = CAllHardware::FiscalFields::PayOffSubjectMethodType;
    // CAllHardware::FiscalFields fiscal totals
    dummy = CAllHardware::FiscalFields::CashFiscalTotal;
    dummy = CAllHardware::FiscalFields::CardFiscalTotal;
    dummy = CAllHardware::FiscalFields::PrePaymentFiscalTotal;
    dummy = CAllHardware::FiscalFields::PostPaymentFiscalTotal;
    dummy = CAllHardware::FiscalFields::CounterOfferFiscalTotal;
    // CAllHardware::FiscalFields taxes
    dummy = CAllHardware::FiscalFields::TaxAmount02;
    dummy = CAllHardware::FiscalFields::TaxAmount03;
    dummy = CAllHardware::FiscalFields::TaxAmount04;
    dummy = CAllHardware::FiscalFields::TaxAmount05;
    dummy = CAllHardware::FiscalFields::TaxAmount06;
    dummy = CAllHardware::FiscalFields::TaxAmount07;
    // CAllHardware::FiscalFields::Values
    dummy = CAllHardware::FiscalFields::Values::NoData;
    (void)dummy; // Suppress unused variable warning
}

// Call dummy function to ensure constants are linked
static int dummy_call = (dummyReference(), 0);

} // namespace Driver
} // namespace SDK
