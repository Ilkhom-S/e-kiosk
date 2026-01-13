# CMakeLists for src/ libraries

# Module layout: modules/common holds shared utilities for apps
# Moved base application to `modules/common/application` as a single BasicApplication module
add_subdirectory(modules/common/application)
