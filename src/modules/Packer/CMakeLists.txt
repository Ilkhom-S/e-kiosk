# Packer module

file(GLOB PACKER_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(Packer
    SOURCES ${PACKER_SOURCES}
    QT_MODULES Core
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link to zlib if available, otherwise use Qt's bundled zlib
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    target_link_libraries(Packer PRIVATE ZLIB::ZLIB)
else()
    # Try to use Qt's internal zlib path (for compatibility with Qt5/Qt6)
    # Compute the Qt core CMake dir variable name for the active Qt major version
    set(_qt_core_dir_var "Qt${QT_VERSION_MAJOR}Core_DIR")
    if(DEFINED ${_qt_core_dir_var})
        set(_qt_core_dir ${${_qt_core_dir_var}})
        set(_qt_qtzlib_path "${_qt_core_dir}/../../../include/QtZlib")
        if(EXISTS "${_qt_qtzlib_path}")
            target_include_directories(Packer PRIVATE "${_qt_qtzlib_path}")
        endif()
    endif()
endif()
