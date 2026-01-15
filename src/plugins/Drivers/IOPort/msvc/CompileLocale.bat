rem Setting Qt variables to use QLinguist commands
rem call %QTDIR%\bin\qtenv2.bat

lrelease "..\src\locale\ioports_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\locale\ioports_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\locale\ioports_tg.ts" -qm "%~1\%~2_tg.qm"
lrelease "..\src\locale\ioports_uz.ts" -qm "%~1\%~2_uz.qm"
