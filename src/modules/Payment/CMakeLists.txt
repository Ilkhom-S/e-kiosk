# Payment module (PaymentBase)

file(GLOB PAYMENT_SOURCES
    src/*.cpp
    src/*.h
)

# Qt Script is deprecated in Qt6 and optional in Qt5
# Check if QtScript is available
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 COMPONENTS Script QUIET)
    if(Qt5Script_FOUND)
        ek_add_library(PaymentBase
            SOURCES ${PAYMENT_SOURCES}
            QT_MODULES Core Qml Script
            INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ek_boost
            COMPILE_DEFINITIONS _UNICODE UNICODE
        )
    else()
        message(STATUS "PaymentBase: Building without QtScript support")
        ek_add_library(PaymentBase
            SOURCES ${PAYMENT_SOURCES}
            QT_MODULES Core Qml
            INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ek_boost
            COMPILE_DEFINITIONS _UNICODE UNICODE NO_QTSCRIPT
        )
    endif()
else()
    # Qt6 doesn't have QtScript, may need alternative
    ek_add_library(PaymentBase
        SOURCES ${PAYMENT_SOURCES}
        QT_MODULES Core Qml
        INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS boost
        COMPILE_DEFINITIONS _UNICODE UNICODE QT6_NO_SCRIPT NO_QTSCRIPT
    )
endif()

# Note: This module depends on boost from thirdparty
