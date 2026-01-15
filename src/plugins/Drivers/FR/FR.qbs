import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "fr"

		Depends { name: "HardwareProtocols" }
		Depends { name: "HardwareCommon" }
		Depends { name: "HardwarePrinters" }
		Depends { name: "HardwareFR" }
		Depends { name: "OPOSSDK" }
		Depends { name: "SysUtils" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.cpp",
			"src/*.h",
			"src/Parameters/*.cpp",
			"src/Parameters/*.h",
			"../Parameters/PrinterPluginParameters.*",
			"../../../includes/Hardware/FR/FRStatusesDescriptions.h"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "fr_ru"
		files: [
			"src/locale/fr_ru.ts",
			"../Printer/src/locale/printers_ru.ts",
			"../../../modules/Hardware/Common/src/locale/common_ru.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "fr_en"
		files: [
			"src/locale/fr_en.ts",
			"../Printer/src/locale/printers_en.ts",
			"../../../modules/Hardware/Common/src/locale/common_en.ts"
		]
	}

	   Product {
		   Depends { name: "MultiLocale" }
		   name: "fr_tg"
		   files: [
			   "src/locale/fr_tg.ts",
			   "../Printer/src/locale/printers_tg.ts",
			   "../../../modules/Hardware/Common/src/locale/common_tg.ts"
		   ]
	   }

	   Product {
		   Depends { name: "MultiLocale" }
		   name: "fr_uz"
		   files: [
			   "src/locale/fr_uz.ts",
			   "../Printer/src/locale/printers_uz.ts",
			   "../../../modules/Hardware/Common/src/locale/common_uz.ts"
		   ]
	   }
}
