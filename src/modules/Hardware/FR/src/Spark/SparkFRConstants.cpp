/* @file Константы протокола ФР Spark. */

#include "SparkFRConstants.h"

namespace CSparkFR {
namespace Commands {
const char ENQT[] = "\x1A";
const char GetFWVersion[] = "SV";
const char KKMInfo[] = "S1";
const char EKLZInfo[] = "S\"";
const char GetEKLZError[] = "S!";
const char ZBufferSpace[] = "SA";
const char ZBufferCount[] = "SAA";
const char PrintLine[] = "PP";
const char Cut[] = "Pp0";
const char TaxesAndFlags[] = "S3";
const char SetFlag[] = "PJ";
const char Payin[] = "91";
const char ClosePayIO[] = "t";
const char Payout[] = "90";
const char Sale0[] = " ";
const char Sale1[] = "!";
const char Sale2[] = "\"";
const char Sale3[] = "#";
const char Sale4[] = "$";
const char SetTaxes[] = "PT";
const char AcceptTaxes[] = "Pt";
const char CloseFiscal[] = "1";
const char CancelFiscal[] = "7";
const char Reports[] = "I";
const char PrintZBuffer[] = "PALL";
const char EnterPassword[] = "W  ";
const char GetTotal[] = "Sg";
const char GetCashAcceptorTotal[] = "S|bf3";
const char Push[] = "S|-1";
const char Retract[] = "S|-0";
const char OpenKKM[] = "o";
const char CloseKKM[] = "zz";
const char SetCashier[] = "5";
const char GetSensorState[] = "S|R";
const char SetTextProperty[] = "P@";
const char SetTextPropertyName[] = "PL";
const char GetTextPropertyName[] = "SY";
} // namespace Commands
} // namespace CSparkFR
