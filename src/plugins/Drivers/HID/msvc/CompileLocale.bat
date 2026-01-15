rem call %QTDIR%\bin\qtenv2.bat

lrelease "..\src\locale\scanners_ru.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\scanners_en.ts" "%TC_LIB_DIR%\Hardware\common_ru.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\scanners_tg.ts" "%TC_LIB_DIR%\Hardware\common_tg.ts" -qm "%~1\%~2_tg.qm"
lrelease "..\src\locale\scanners_uz.ts" "%TC_LIB_DIR%\Hardware\common_uz.ts" -qm "%~1\%~2_uz.qm"
