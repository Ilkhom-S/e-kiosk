cmake_minimum_required(VERSION 3.16)
project(EKiosk VERSION 1.0.0 LANGUAGES CXX C)

# Only reference subdirectories for modular apps/libraries
add_subdirectory(apps)
add_subdirectory(tests)
