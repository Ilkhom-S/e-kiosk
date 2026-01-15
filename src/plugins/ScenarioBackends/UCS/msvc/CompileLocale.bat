
call %QTDIR%\bin\qtvars.bat

lupdate "..\src" -ts "..\src\Locale\ucs_ru.ts" -ts "..\src\Locale\ucs_en.ts" -ts "..\src\Locale\ucs_tg.ts" -ts "..\src\Locale\ucs_uz.ts"
lrelease "..\src\Locale\ucs_ru.ts" -qm "%~1\%~2_ru.qm"
lrelease "..\src\Locale\ucs_en.ts" -qm "%~1\%~2_en.qm"
lrelease "..\src\Locale\ucs_tg.ts" -qm "%~1\%~2_tg.qm"
lrelease "..\src\Locale\ucs_uz.ts" -qm "%~1\%~2_uz.qm"
