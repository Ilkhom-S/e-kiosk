# Modules test aggregator

# Add module-specific test subdirectories here as modules are created/refactored
# Example:
# add_subdirectory(auth)
# add_subdirectory(db)

message(STATUS "Configuring module tests")
add_subdirectory(kiosk)
add_subdirectory(Connection)
add_subdirectory(NetworkTaskManager)
