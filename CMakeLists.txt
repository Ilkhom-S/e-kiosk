cmake_minimum_required(VERSION 3.16)

project(EKiosk VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt - support both Qt5 and Qt6
# Priority order: environment variables > CMAKE_PREFIX_PATH > default detection
if(DEFINED ENV{QT5_DIR})
    # Use QT5_DIR environment variable
    set(CMAKE_PREFIX_PATH $ENV{QT5_DIR})
    find_package(Qt5 QUIET COMPONENTS Core PATHS $ENV{QT5_DIR})
elseif(DEFINED ENV{QT6_DIR})
    # Use QT6_DIR environment variable
    set(CMAKE_PREFIX_PATH $ENV{QT6_DIR})
    find_package(Qt6 QUIET COMPONENTS Core PATHS $ENV{QT6_DIR})
elseif(DEFINED CMAKE_PREFIX_PATH)
    # If CMAKE_PREFIX_PATH is set (e.g., from presets), use it
    find_package(Qt5 QUIET COMPONENTS Core PATHS ${CMAKE_PREFIX_PATH})
    if(NOT Qt5_FOUND)
        find_package(Qt6 QUIET COMPONENTS Core PATHS ${CMAKE_PREFIX_PATH})
    endif()
else()
    # Default behavior: try Qt5 first, fall back to Qt6
    find_package(Qt5 QUIET COMPONENTS Core)
    if(NOT Qt5_FOUND)
        find_package(Qt6 QUIET COMPONENTS Core)
    endif()
endif()

if(Qt5_FOUND)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5")
elseif(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
else()
    message(FATAL_ERROR "Neither Qt5 nor Qt6 found. Please install Qt and set one of the following:\n"
        "  - Environment variable QT5_DIR or QT6_DIR (recommended for cross-platform development)\n"
        "  - CMAKE_PREFIX_PATH in presets or command line\n"
        "  - Add Qt to PATH or use default installation location\n"
        "Examples:\n"
        "  - QT5_DIR=C:/Qt/5.15.2/msvc2019_64\n"
        "  - QT6_DIR=C:/Qt/6.10.1/msvc2022_64\n"
        "  - CMAKE_PREFIX_PATH=C:/Qt/Qt5.6.3/5.6.3/msvc2015")
endif()

# Now find the actual Qt package with the detected version
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# Define common Qt components needed
set(QT_COMMON_COMPONENTS
    Core
    Widgets
    Sql
    PrintSupport
    Network
    Multimedia
    Xml
    SerialPort
)

# WebSockets is optional - check if available
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WebSockets QUIET)
if(Qt${QT_VERSION_MAJOR}WebSockets_FOUND)
    list(APPEND QT_COMMON_COMPONENTS WebSockets)
    message(STATUS "Qt WebSockets component found")
else()
    message(WARNING "Qt WebSockets component not found - WebSocket functionality will be limited")
endif()

# Qt5-specific components
if(QT_VERSION_MAJOR EQUAL 5)
    # Try WebKitWidgets first (Qt5.5 and earlier), then WebEngineWidgets (Qt5.6+)
    find_package(Qt5 COMPONENTS WebKitWidgets QUIET)
    if(Qt5WebKitWidgets_FOUND)
        list(APPEND QT_COMMON_COMPONENTS WebKitWidgets)
        add_compile_definitions(HAS_WEBKIT)
        message(STATUS "Qt5 WebKitWidgets component found")
    else()
        find_package(Qt5 COMPONENTS WebEngineWidgets QUIET)
        if(Qt5WebEngineWidgets_FOUND)
            list(APPEND QT_COMMON_COMPONENTS WebEngineWidgets)
            add_compile_definitions(HAS_WEBENGINE)
            message(STATUS "Qt5 WebEngineWidgets component found")
        else()
            message(WARNING "Neither Qt5 WebKitWidgets nor WebEngineWidgets found - web functionality will be limited")
        endif()
    endif()
endif()

# Qt6-specific adjustments
if(QT_VERSION_MAJOR EQUAL 6)
    # WebKit is not available in Qt6, WebEngine is the replacement
    find_package(Qt6 COMPONENTS WebEngineWidgets QUIET)
    if(Qt6WebEngineWidgets_FOUND)
        list(APPEND QT_COMMON_COMPONENTS WebEngineWidgets)
        add_compile_definitions(USE_WEBENGINE)
    else()
        message(WARNING "Qt6 WebEngine not found, web functionality may be limited")
    endif()
endif()

# Find Qt with all required components
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMMON_COMPONENTS})

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/connection/CheckConnection.cpp
    src/connection/Connect.cpp
    src/connection/RasConnection.cpp
    src/db/sqlconnection.h
    src/devices/ClassDevice.cpp
    src/devices/ConstantData.h
    src/devices/coinacceptor/AbstractAcceptor.cpp
    src/devices/coinacceptor/ClassAcceptor.cpp
    src/devices/coinacceptor/dev/CCTalk.cpp
    src/devices/modems/ClassModem.cpp
    src/devices/modems/protocol/ATProtocol.cpp
    src/devices/modems/protocol/qatresult.cpp
    src/devices/modems/protocol/qatresultparser.cpp
    src/devices/modems/protocol/qatutils.cpp
    src/devices/modems/protocol/qgsmcodec.cpp
    src/devices/printers/AbstractPrinter.cpp
    src/devices/printers/ClassPrinter.cpp
    src/devices/printers/dev/AV268.cpp
    src/devices/printers/dev/CitizenCBM1000.cpp
    src/devices/printers/dev/CitizenCTS2000.cpp
    src/devices/printers/dev/CitizenPPU700.cpp
    src/devices/printers/dev/CustomTG2480.cpp
    src/devices/printers/dev/CustomVKP80.cpp
    src/devices/printers/dev/KM1X.cpp
    src/devices/printers/dev/Phoenix.cpp
    src/devices/printers/dev/StarTUP900.cpp
    src/devices/validators/AbstractValidator.cpp
    src/devices/validators/ClassValidator.cpp
    src/devices/validators/dev/CCNetFirmware.h
    src/devices/validators/dev/CCNetSM.cpp
    src/devices/validators/dev/CashPayment.h
    src/devices/validators/dev/EBDS.cpp
    src/devices/watchdogs/watchdogs.cpp
    src/modules/AuthRequest.cpp
    src/modules/CheckOnline.cpp
    src/modules/CollectDaemons.cpp
    src/modules/CommandConfirm.cpp
    src/modules/ConfirmOtp.cpp
    src/modules/GetBalanceAgent.cpp
    src/modules/GetServices.cpp
    src/modules/JsonRequest.cpp
    src/modules/PayDaemons.cpp
    src/modules/SendLogInfo.cpp
    src/modules/SendOtp.cpp
    src/modules/SendReceipt.cpp
    src/modules/SendRequest.cpp
    src/modules/StatusDaemons.cpp
    src/modules/UserDaemons.cpp
    src/other/receipt.cpp
    src/other/utils.cpp
    src/ui/adminbutton.cpp
    src/ui/admindialog.cpp
    src/ui/avtorizationtoadminin.cpp
    src/ui/changepassword.cpp
    src/ui/createdialupconnection.cpp
    src/ui/incasaciyaform.cpp
    src/ui/keypud.cpp
    src/ui/loadinggprsform.cpp
    src/ui/mainpageloader.cpp
    src/ui/registrationdialog.cpp
    src/ui/registrationform.cpp
    src/ui/searchdevicesform.cpp
    src/ui/selectcategorylogview.cpp
    src/updater/textprogressbar.cpp
    src/updater/update.cpp
)

set(HEADERS
    src/main.h
    src/mainwindow.h
    src/connection/CheckConnection.h
    src/connection/Connect.h
    src/connection/RasConnection.h
    src/devices/ClassDevice.h
    src/devices/coinacceptor/AbstractAcceptor.h
    src/devices/coinacceptor/ClassAcceptor.h
    src/devices/coinacceptor/dev/CCTalk.h
    src/devices/modems/ClassModem.h
    src/devices/modems/protocol/ATProtocol.h
    src/devices/modems/protocol/qatresult.h
    src/devices/modems/protocol/qatresultparser.h
    src/devices/modems/protocol/qatutils.h
    src/devices/modems/protocol/qgsmcodec.h
    src/devices/printers/AbstractPrinter.h
    src/devices/printers/ClassPrinter.h
    src/devices/printers/dev/AV268.h
    src/devices/printers/dev/CitizenCBM1000.h
    src/devices/printers/dev/CitizenCTS2000.h
    src/devices/printers/dev/CitizenPPU700.h
    src/devices/printers/dev/CustomTG2480.h
    src/devices/printers/dev/CustomVKP80.h
    src/devices/printers/dev/KM1X.h
    src/devices/printers/dev/Phoenix.h
    src/devices/printers/dev/StarTUP900.h
    src/devices/validators/AbstractValidator.h
    src/devices/validators/ClassValidator.h
    src/devices/validators/dev/EBDS.h
    src/devices/watchdogs/watchdogs.h
    src/modules/AuthRequest.h
    src/modules/CheckOnline.h
    src/modules/CollectDaemons.h
    src/modules/CommandConfirm.h
    src/modules/ConfirmOtp.h
    src/modules/GetBalanceAgent.h
    src/modules/GetServices.h
    src/modules/JsonRequest.h
    src/modules/PayDaemons.h
    src/modules/SendLogInfo.h
    src/modules/SendOtp.h
    src/modules/SendReceipt.h
    src/modules/SendRequest.h
    src/modules/StatusDaemons.h
    src/modules/UserDaemons.h
    src/other/jsInterface.h
    src/other/logClean.h
    src/other/logger.h
    src/other/loggerValidator.h
    src/other/receipt.h
    src/other/systemInfo.h
    src/other/utils.h
    src/ui/adminbutton.h
    src/ui/admindialog.h
    src/ui/avtorizationtoadminin.h
    src/ui/changepassword.h
    src/ui/createdialupconnection.h
    src/ui/incasaciyaform.h
    src/ui/keypud.h
    src/ui/loadinggprsform.h
    src/ui/mainpageloader.h
    src/ui/registrationdialog.h
    src/ui/registrationform.h
    src/ui/searchdevicesform.h
    src/ui/selectcategorylogview.h
    src/updater/CopyFile.h
    src/updater/textprogressbar.h
    src/updater/update.h
)

set(UI_FILES
    src/mainwindow.ui
    src/ui/adminbutton.ui
    src/ui/admindialog.ui
    src/ui/avtorizationtoadminin.ui
    src/ui/changepassword.ui
    src/ui/createdialupconnection.ui
    src/ui/incasaciyaform.ui
    src/ui/keypud.ui
    src/ui/loadinggprsform.ui
    src/ui/mainpageloader.ui
    src/ui/registrationdialog.ui
    src/ui/registrationform.ui
    src/ui/searchdevicesform.ui
    src/ui/selectcategorylogview.ui
)

set(RESOURCE_FILES
    ekiosk.qrc
)

if(QT_VERSION_MAJOR EQUAL 5)
    qt5_add_resources(RESOURCE_SOURCES ${RESOURCE_FILES})
else()
    qt6_add_resources(RESOURCE_SOURCES ${RESOURCE_FILES})
endif()

add_executable(EKiosk
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_SOURCES}
)

target_include_directories(EKiosk PRIVATE src)

target_link_libraries(EKiosk
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::SerialPort
)

# Add WebSockets if available
if(Qt${QT_VERSION_MAJOR}WebSockets_FOUND)
    target_link_libraries(EKiosk Qt${QT_VERSION_MAJOR}::WebSockets)
endif()

# Add web component based on Qt version
if(QT_VERSION_MAJOR EQUAL 5)
    if(Qt5WebKitWidgets_FOUND)
        target_link_libraries(EKiosk Qt5::WebKitWidgets)
    elseif(Qt5WebEngineWidgets_FOUND)
        target_link_libraries(EKiosk Qt5::WebEngineWidgets)
    endif()
else()
    if(Qt6WebEngineWidgets_FOUND)
        target_link_libraries(EKiosk Qt6::WebEngineWidgets)
    endif()
endif()

if(WIN32)
    target_link_libraries(EKiosk
        rasapi32
        winspool
        psapi
        user32
        Advapi32
    )
endif()

# MSVC-specific compiler flags to fix PDB file access issues
if(MSVC)
    target_compile_options(EKiosk PRIVATE /FS)
endif()

set_target_properties(EKiosk PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../EKiosk
)
