
cmake_minimum_required(VERSION 3.16)

project(EKioskRoot VERSION 1.0.0 LANGUAGES CXX C)

# Global Qt detection for all modules/apps/tests
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)



# Only reference subdirectories for modular apps/libraries
add_subdirectory(apps)
add_subdirectory(tests)
