import qbs 1.0
import "../driverTemplate.qbs" as DriverTemplate

Project {

	DriverTemplate {
		name: "printers"

		Depends { name: "Qt"; submodules: ["axcontainer", "gui", "printsupport"] }

		Depends { name: "DriversSDK" }
		Depends { name: "HardwareCommon" }
		Depends { name: "HardwarePrinters" }
		Depends { name: "SysUtils" }
		Depends { name: "HardwareIOPorts" }

		files: [
			"../../../includes/Common/CoreVersion.rc",
			"src/*.h",
			"src/*.cpp",
			"../Parameters/PrinterPluginParameters.h",
			"../Parameters/PrinterPluginParameterTranslations.h",
			"../Parameters/PrinterPluginParameters.cpp",
			"../../../includes/Hardware/Printers/PrinterStatusesDescriptions.h"
		]

		Properties {
			condition: qbs.targetOS == "windows"
			cpp.staticLibraries: ["Advapi32", "User32", "Winspool"]
		}
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "printers_ru"
		files: [
			"src/locale/printers_ru.ts",
			"../../../modules/Hardware/Common/src/locale/common_ru.ts"
		]
	}

	Product {
		Depends { name: "MultiLocale" }
		name: "printers_en"
		files: [
			"src/locale/printers_en.ts",
			"../../../modules/Hardware/Common/src/locale/common_en.ts"
		]
	}

	   Product {
		   Depends { name: "MultiLocale" }
		   name: "printers_tg"
		   files: [
			   "src/locale/printers_tg.ts",
			   "../../../modules/Hardware/Common/src/locale/common_tg.ts"
		   ]
	   }

	   Product {
		   Depends { name: "MultiLocale" }
		   name: "printers_uz"
		   files: [
			   "src/locale/printers_uz.ts",
			   "../../../modules/Hardware/Common/src/locale/common_uz.ts"
		   ]
	   }
}
