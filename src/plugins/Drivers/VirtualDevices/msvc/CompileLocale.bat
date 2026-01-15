
call %QTDIR%\bin\qtvars.bat

lrelease "..\src\locale\virtual_devices_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\virtual_devices_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\virtual_devices_tg.ts" -qm "%~1\%~2_tg.qm"
lrelease "..\src\locale\virtual_devices_uz.ts" -qm "%~1\%~2_uz.qm"
