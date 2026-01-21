# RunEKVersionConfig.cmake
# Invokes ek_configure_version to generate version files for EKiosk

include(scripts/build/cmake/EKVersionConfig.cmake)
ek_configure_version()
