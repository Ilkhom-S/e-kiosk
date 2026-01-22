# Payment module (PaymentBase)

file(GLOB PAYMENT_SOURCES
    src/*.cpp
    src/*.h
)


# QtQml is required for JavaScript evaluation in PaymentBase (QJSEngine)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Qml REQUIRED)
set(PAYMENT_QT_MODULES Core Qml)
set(PAYMENT_DEPENDS ek_boost)

ek_add_library(PaymentBase
    FOLDER "modules"
    SOURCES ${PAYMENT_SOURCES}
    QT_MODULES ${PAYMENT_QT_MODULES}
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${PAYMENT_DEPENDS}
    COMPILE_DEFINITIONS _UNICODE UNICODE
)
