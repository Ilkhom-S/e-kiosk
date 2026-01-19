# Payment module (PaymentBase)

file(GLOB PAYMENT_SOURCES
    src/*.cpp
    src/*.h
)


# QtQml is required for JavaScript evaluation in PaymentBase (QJSEngine)
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 COMPONENTS Core Qml REQUIRED)
    set(PAYMENT_QT_MODULES Core Qml)
    set(PAYMENT_DEPENDS ek_boost)
elseif(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 COMPONENTS Core Qml REQUIRED)
    set(PAYMENT_QT_MODULES Core Qml)
    set(PAYMENT_DEPENDS boost)
endif()

ek_add_library(PaymentBase
    FOLDER "modules"
    SOURCES ${PAYMENT_SOURCES}
    QT_MODULES ${PAYMENT_QT_MODULES}
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${PAYMENT_DEPENDS}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)

# Note: This module depends on boost from thirdparty
