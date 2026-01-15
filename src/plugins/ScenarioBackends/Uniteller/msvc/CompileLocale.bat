
call %QTDIR%\bin\qtvars.bat

lupdate "..\src" -ts "..\src\Locale\uniteller_ru.ts" -ts "..\src\Locale\uniteller_en.ts" -ts "..\src\Locale\uniteller_tg.ts" -ts "..\src\Locale\uniteller_uz.ts"
lrelease "..\src\Locale\uniteller_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\Locale\uniteller_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\Locale\uniteller_tg.ts" -qm "%~1\%~2_tg.qm"
lrelease "..\src\Locale\uniteller_uz.ts" -qm "%~1\%~2_uz.qm"
