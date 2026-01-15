
call %QTDIR%\bin\qtvars.bat

lrelease "..\src\Locale\migrator3000_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\Locale\migrator3000_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\Locale\migrator3000_tg.ts" -qm "%~1\%~2_tg.qm"
lrelease "..\src\Locale\migrator3000_uz.ts" -qm "%~1\%~2_uz.qm"
