rem call %QTDIR%\bin\qtenv2.bat

lrelease "..\src\locale\printers_ru.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\printers_en.ts" "%TC_LIB_DIR%\Hardware\common_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\printers_tg.ts" "%TC_LIB_DIR%\Hardware\common_tg.ts" -qm "%~1\%~2_tg.qm"
lrelease "..\src\locale\printers_uz.ts" "%TC_LIB_DIR%\Hardware\common_uz.ts" -qm "%~1\%~2_uz.qm"
