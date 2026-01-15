# Drivers plugins

add_subdirectory(BillAcceptor)
add_subdirectory(BillDispensers)
add_subdirectory(CardReader)
add_subdirectory(CoinAcceptor)

# FR (Fiscal Register) - deprecated, conditionally included
if(EKIOSK_ENABLE_FR)
    message(STATUS "Including FR (Fiscal Register) plugin")
    add_subdirectory(FR)
else()
    message(STATUS "Skipping FR (Fiscal Register) plugin (deprecated)")
endif()

add_subdirectory(HID)
add_subdirectory(IOPort)
add_subdirectory(Modem)
add_subdirectory(Printer)
add_subdirectory(VirtualDevices)
add_subdirectory(Watchdog)
