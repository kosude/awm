cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project("awm")

set(AWM_DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/deps/")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/mod/")

set(CMAKE_CONFIGURATION_TYPES Debug Release)
if (NOT CMAKE_BUILD_TYPE)
    message(WARNING "Missing build configuration type! Defaulted to RELEASE, reconfigure project with option -DCMAKE_BUILD_TYPE=(Debug|Release)")
    set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(AWM_DEBUG true)
    set(AWM_MACROS ${AWM_MACROS} "_DEBUG")
endif()

# treating warnings as errors may be problematic when compiling for prod use.
# we are assuming here that lib users will be compiling in release config and lib developers/contributors in debug config.
if (AWM_DEBUG)
    set(AWM_COMPILE_OPTIONS
        -Wall -Wextra -Wpedantic -Werror
        -Wno-error=unused-parameter
        -Wno-error=unused-function
        -Wno-implicit-fallthrough
        -Wmissing-prototypes -Wstrict-prototypes)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(AWM_OFFICIAL_PLUGINS "Compile official plugins for awm" ON)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src/" "${PROJECT_BINARY_DIR}/awm/") # rename build directory from src/ to awm/
if (AWM_OFFICIAL_PLUGINS)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/plugins/")
endif()
