cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project("awm")

set(AWM_DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/deps")
set(AWM_TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/tools")
set(AWM_GEN_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake/gen")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/mod")

set(CMAKE_CONFIGURATION_TYPES Debug Release)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(AWM_DEBUG true)
    set(AWM_MACROS ${AWM_MACROS} "_DEBUG")
    message(STATUS "awm: DEBUG configuration")
else()
    message(STATUS "awm: RELEASE configuration")
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
option(AWM_WM "Compile the Awm project" ON)
option(AWM_DOCS "Compile HTML documentation (requires Sphinx)" OFF)

execute_process(
    COMMAND "${AWM_TOOLS_DIR}/gversion.sh" --short --no-v
    OUTPUT_VARIABLE AWM_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND "${AWM_TOOLS_DIR}/gversion.sh"
    OUTPUT_VARIABLE AWM_VERSION_LONG
    OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "awm ${AWM_VERSION_LONG}")

if (AWM_WM)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src/" "${PROJECT_BINARY_DIR}/awm/")
endif()
if (AWM_DOCS)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/docs/")
endif()
