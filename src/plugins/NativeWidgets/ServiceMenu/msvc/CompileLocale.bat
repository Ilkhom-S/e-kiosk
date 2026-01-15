rem call %QTDIR%\bin\qtenv2.bat

lrelease "..\src\Locale\service_menu_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\Locale\service_menu_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\Locale\service_menu_tg.ts" -qm "%~1\%~2_tg.qm"
lrelease "..\src\Locale\service_menu_uz.ts" -qm "%~1\%~2_uz.qm"
