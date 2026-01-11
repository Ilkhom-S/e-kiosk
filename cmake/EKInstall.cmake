# EKInstall.cmake - Standardized install rules

include(GNUInstallDirs)

function(ek_install_application TARGET_NAME)
    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT applications
    )
endfunction()

function(ek_install_library TARGET_NAME)
    install(TARGETS ${TARGET_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libraries
    )
endfunction()

function(ek_install_plugin TARGET_NAME)
    install(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins
        COMPONENT plugins
    )
endfunction()
