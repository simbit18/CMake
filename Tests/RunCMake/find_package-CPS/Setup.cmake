# Protect tests from running inside the default install prefix.
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/NotDefaultPrefix")

# Disable built-in search paths.
set(CMAKE_FIND_USE_PACKAGE_ROOT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH OFF)
set(CMAKE_FIND_USE_INSTALL_PREFIX OFF)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR})
