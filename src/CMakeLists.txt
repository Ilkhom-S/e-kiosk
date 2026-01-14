# CMakeLists for src/ libraries

# Module layout: modules/common holds shared utilities for apps
# Moved base application to `modules/common/application` as a single BasicApplication module
add_subdirectory(modules/Common/Application)
add_subdirectory(modules/Common/log)
add_subdirectory(modules/DebugUtils)
add_subdirectory(modules/Connection)
add_subdirectory(modules/NetworkTaskManager)
add_subdirectory(modules/SysUtils)
add_subdirectory(modules/SettingsManager)
add_subdirectory(modules/ScenarioEngine)
add_subdirectory(modules/WatchServiceClient)
