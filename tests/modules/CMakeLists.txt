# Modules test aggregator

# Add module-specific test subdirectories here as modules are created/refactored
# Example:
# add_subdirectory(auth)
# add_subdirectory(db)

message(STATUS "Configuring module tests")
add_subdirectory(Common)
add_subdirectory(Connection)
add_subdirectory(DebugUtils)
add_subdirectory(NetworkTaskManager)
add_subdirectory(SettingsManager)
add_subdirectory(WatchServiceClient)
add_subdirectory(WatchServiceController)
