# EKInstall.cmake - Standardized install rules

include(GNUInstallDirs)

function(ek_install_application TARGET_NAME)
    # Allow overriding the bin directory via EK_BIN_DIR environment variable
    if(DEFINED ENV{EK_BIN_DIR})
        set(_bin_dir $ENV{EK_BIN_DIR})
    else()
        set(_bin_dir ${CMAKE_INSTALL_BINDIR})
    endif()
    
    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${_bin_dir}
        COMPONENT applications
    )
endfunction()

function(ek_install_library TARGET_NAME)
    # Allow overriding the lib directory via EK_LIB_DIR environment variable
    if(DEFINED ENV{EK_LIB_DIR})
        set(_lib_dir $ENV{EK_LIB_DIR})
    else()
        set(_lib_dir ${CMAKE_INSTALL_LIBDIR})
    endif()
    
    install(TARGETS ${TARGET_NAME}
        ARCHIVE DESTINATION ${_lib_dir}
        LIBRARY DESTINATION ${_lib_dir}
        COMPONENT libraries
    )
endfunction()

function(ek_install_plugin TARGET_NAME)
    # Allow overriding the plugin directory via EK_PLUGIN_DIR environment variable
    if(DEFINED ENV{EK_PLUGIN_DIR})
        set(_plugin_dir $ENV{EK_PLUGIN_DIR})
    else()
        set(_plugin_dir ${CMAKE_INSTALL_LIBDIR}/plugins)
    endif()
    
    install(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION ${_plugin_dir}
        RUNTIME DESTINATION ${_plugin_dir}
        COMPONENT plugins
    )
endfunction()
