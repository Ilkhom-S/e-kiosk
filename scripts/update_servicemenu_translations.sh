#!/bin/bash
# EKiosk ServiceMenu translation update script

# Update .ts files from all .cpp, .h, and .ui files
lupdate src/plugins/NativeWidgets/ServiceMenu/**/*.cpp src/plugins/NativeWidgets/ServiceMenu/**/*.h src/plugins/NativeWidgets/ServiceMenu/**/*.ui \
    -ts src/plugins/NativeWidgets/ServiceMenu/src/Locale/service_menu_ru.ts \
        src/plugins/NativeWidgets/ServiceMenu/src/Locale/service_menu_en.ts \
        src/plugins/NativeWidgets/ServiceMenu/src/Locale/service_menu_uz.ts \
        src/plugins/NativeWidgets/ServiceMenu/src/Locale/service_menu_tg.ts

# Release .qm files from updated .ts files
lrelease src/plugins/NativeWidgets/ServiceMenu/src/Locale/service_menu_ru.ts \
         src/plugins/NativeWidgets/ServiceMenu/src/Locale/service_menu_en.ts \
         src/plugins/NativeWidgets/ServiceMenu/src/Locale/service_menu_uz.ts \
         src/plugins/NativeWidgets/ServiceMenu/src/Locale/service_menu_tg.ts

echo "✅ Translation update and release complete."
