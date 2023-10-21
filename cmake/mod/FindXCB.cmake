# Copyright (c) KDE Developers.
# ==============================================================================
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ==============================================================================

# KDE/extra-cmake-modules: ECMFindModuleHelpers.cmake

#]=======================================================================]
# SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

include(CMakeParseArguments)

macro(ecm_find_package_version_check module_name)
    if(CMAKE_VERSION VERSION_LESS 3.16.0)
        message(FATAL_ERROR "CMake 3.16.0 is required by Find${module_name}.cmake")
    endif()
    if(CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 3.16.0)
        message(AUTHOR_WARNING "Your project should require at least CMake 3.16.0 to use Find${module_name}.cmake")
    endif()
endmacro()

macro(ecm_find_package_parse_components module_name)
    set(ecm_fppc_options SKIP_DEPENDENCY_HANDLING)
    set(ecm_fppc_oneValueArgs RESULT_VAR)
    set(ecm_fppc_multiValueArgs KNOWN_COMPONENTS DEFAULT_COMPONENTS)
    cmake_parse_arguments(ECM_FPPC "${ecm_fppc_options}" "${ecm_fppc_oneValueArgs}" "${ecm_fppc_multiValueArgs}" ${ARGN})

    if(ECM_FPPC_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to ecm_find_package_parse_components: ${ECM_FPPC_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT ECM_FPPC_RESULT_VAR)
        message(FATAL_ERROR "Missing RESULT_VAR argument to ecm_find_package_parse_components")
    endif()
    if(NOT ECM_FPPC_KNOWN_COMPONENTS)
        message(FATAL_ERROR "Missing KNOWN_COMPONENTS argument to ecm_find_package_parse_components")
    endif()
    if(NOT ECM_FPPC_DEFAULT_COMPONENTS)
        set(ECM_FPPC_DEFAULT_COMPONENTS ${ECM_FPPC_KNOWN_COMPONENTS})
    endif()

    if(${module_name}_FIND_COMPONENTS)
        set(ecm_fppc_requestedComps ${${module_name}_FIND_COMPONENTS})

        if(NOT ECM_FPPC_SKIP_DEPENDENCY_HANDLING)
            # Make sure deps are included
            foreach(ecm_fppc_comp ${ecm_fppc_requestedComps})
                foreach(ecm_fppc_dep_comp ${${module_name}_${ecm_fppc_comp}_component_deps})
                    list(FIND ecm_fppc_requestedComps "${ecm_fppc_dep_comp}" ecm_fppc_index)
                    if("${ecm_fppc_index}" STREQUAL "-1")
                        if(NOT ${module_name}_FIND_QUIETLY)
                            message(STATUS "${module_name}: ${ecm_fppc_comp} requires ${${module_name}_${ecm_fppc_comp}_component_deps}")
                        endif()
                        list(APPEND ecm_fppc_requestedComps "${ecm_fppc_dep_comp}")
                    endif()
                endforeach()
            endforeach()
        else()
            message(STATUS "Skipping dependency handling for ${module_name}")
        endif()
        list(REMOVE_DUPLICATES ecm_fppc_requestedComps)

        # This makes sure components are listed in the same order as
        # KNOWN_COMPONENTS (potentially important for inter-dependencies)
        set(${ECM_FPPC_RESULT_VAR})
        foreach(ecm_fppc_comp ${ECM_FPPC_KNOWN_COMPONENTS})
            list(FIND ecm_fppc_requestedComps "${ecm_fppc_comp}" ecm_fppc_index)
            if(NOT "${ecm_fppc_index}" STREQUAL "-1")
                list(APPEND ${ECM_FPPC_RESULT_VAR} "${ecm_fppc_comp}")
                list(REMOVE_AT ecm_fppc_requestedComps ${ecm_fppc_index})
            endif()
        endforeach()
        # if there are any left, they are unknown components
        if(ecm_fppc_requestedComps)
            set(ecm_fppc_msgType STATUS)
            if(${module_name}_FIND_REQUIRED)
                set(ecm_fppc_msgType FATAL_ERROR)
            endif()
            if(NOT ${module_name}_FIND_QUIETLY)
                message(${ecm_fppc_msgType} "${module_name}: requested unknown components ${ecm_fppc_requestedComps}")
            endif()
            return()
        endif()
    else()
        set(${ECM_FPPC_RESULT_VAR} ${ECM_FPPC_DEFAULT_COMPONENTS})
    endif()
endmacro()

macro(ecm_find_package_handle_library_components module_name)
    set(ecm_fpwc_options SKIP_PKG_CONFIG SKIP_DEPENDENCY_HANDLING)
    set(ecm_fpwc_oneValueArgs)
    set(ecm_fpwc_multiValueArgs COMPONENTS)
    cmake_parse_arguments(ECM_FPWC "${ecm_fpwc_options}" "${ecm_fpwc_oneValueArgs}" "${ecm_fpwc_multiValueArgs}" ${ARGN})

    if(ECM_FPWC_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to ecm_find_package_handle_components: ${ECM_FPWC_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT ECM_FPWC_COMPONENTS)
        message(FATAL_ERROR "Missing COMPONENTS argument to ecm_find_package_handle_components")
    endif()

    include(FindPackageHandleStandardArgs)
    find_package(PkgConfig QUIET)
    foreach(ecm_fpwc_comp ${ECM_FPWC_COMPONENTS})
        set(ecm_fpwc_dep_vars)
        set(ecm_fpwc_dep_targets)
        if(NOT SKIP_DEPENDENCY_HANDLING)
            foreach(ecm_fpwc_dep ${${module_name}_${ecm_fpwc_comp}_component_deps})
                list(APPEND ecm_fpwc_dep_vars "${module_name}_${ecm_fpwc_dep}_FOUND")
                list(APPEND ecm_fpwc_dep_targets "${module_name}::${ecm_fpwc_dep}")
            endforeach()
        endif()

        if(NOT ECM_FPWC_SKIP_PKG_CONFIG AND ${module_name}_${ecm_fpwc_comp}_pkg_config)
            pkg_check_modules(PKG_${module_name}_${ecm_fpwc_comp} QUIET
                              ${${module_name}_${ecm_fpwc_comp}_pkg_config})
        endif()

        find_path(${module_name}_${ecm_fpwc_comp}_INCLUDE_DIR
            NAMES ${${module_name}_${ecm_fpwc_comp}_header}
            HINTS ${PKG_${module_name}_${ecm_fpwc_comp}_INCLUDE_DIRS}
            PATH_SUFFIXES ${${module_name}_${ecm_fpwc_comp}_header_subdir}
        )
        find_library(${module_name}_${ecm_fpwc_comp}_LIBRARY
            NAMES ${${module_name}_${ecm_fpwc_comp}_lib}
            HINTS ${PKG_${module_name}_${ecm_fpwc_comp}_LIBRARY_DIRS}
        )

        set(${module_name}_${ecm_fpwc_comp}_VERSION "${PKG_${module_name}_${ecm_fpwc_comp}_VERSION}")
        if(NOT ${module_name}_VERSION)
            set(${module_name}_VERSION ${${module_name}_${ecm_fpwc_comp}_VERSION})
        endif()

        set(FPHSA_NAME_MISMATCHED 1)
        find_package_handle_standard_args(${module_name}_${ecm_fpwc_comp}
            FOUND_VAR
                ${module_name}_${ecm_fpwc_comp}_FOUND
            REQUIRED_VARS
                ${module_name}_${ecm_fpwc_comp}_LIBRARY
                ${module_name}_${ecm_fpwc_comp}_INCLUDE_DIR
                ${ecm_fpwc_dep_vars}
            VERSION_VAR
                ${module_name}_${ecm_fpwc_comp}_VERSION
            )
        unset(FPHSA_NAME_MISMATCHED)

        mark_as_advanced(
            ${module_name}_${ecm_fpwc_comp}_LIBRARY
            ${module_name}_${ecm_fpwc_comp}_INCLUDE_DIR
        )

        if(${module_name}_${ecm_fpwc_comp}_FOUND)
            list(APPEND ${module_name}_LIBRARIES
                        "${${module_name}_${ecm_fpwc_comp}_LIBRARY}")
            list(APPEND ${module_name}_INCLUDE_DIRS
                        "${${module_name}_${ecm_fpwc_comp}_INCLUDE_DIR}")
            set(${module_name}_DEFINITIONS
                    ${${module_name}_DEFINITIONS}
                    ${PKG_${module_name}_${ecm_fpwc_comp}_DEFINITIONS})
            if(NOT TARGET ${module_name}::${ecm_fpwc_comp})
                add_library(${module_name}::${ecm_fpwc_comp} UNKNOWN IMPORTED)
                set_target_properties(${module_name}::${ecm_fpwc_comp} PROPERTIES
                    IMPORTED_LOCATION "${${module_name}_${ecm_fpwc_comp}_LIBRARY}"
                    INTERFACE_COMPILE_OPTIONS "${PKG_${module_name}_${ecm_fpwc_comp}_DEFINITIONS}"
                    INTERFACE_INCLUDE_DIRECTORIES "${${module_name}_${ecm_fpwc_comp}_INCLUDE_DIR}"
                    INTERFACE_LINK_LIBRARIES "${ecm_fpwc_dep_targets}"
                )
            endif()
            list(APPEND ${module_name}_TARGETS
                        "${module_name}::${ecm_fpwc_comp}")
        endif()
    endforeach()
    if(${module_name}_LIBRARIES)
        list(REMOVE_DUPLICATES ${module_name}_LIBRARIES)
    endif()
    if(${module_name}_INCLUDE_DIRS)
        list(REMOVE_DUPLICATES ${module_name}_INCLUDE_DIRS)
    endif()
    if(${module_name}_DEFINITIONS)
        list(REMOVE_DUPLICATES ${module_name}_DEFINITIONS)
    endif()
    if(${module_name}_TARGETS)
        list(REMOVE_DUPLICATES ${module_name}_TARGETS)
    endif()
endmacro()

# KDE/extra-cmake-modules: FindXCB.cmake

#]=======================================================================]
# SPDX-FileCopyrightText: 2011 Fredrik Höglund <fredrik@kde.org>
# SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
# SPDX-FileCopyrightText: 2014-2015 Alex Merry <alex.merry@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

ecm_find_package_version_check(XCB)

# Note that this list needs to be ordered such that any component
# appears after its dependencies
set(XCB_known_components
    XCB
    RENDER
    SHAPE
    XFIXES
    SHM
    ATOM
    AUX
    COMPOSITE
    CURSOR
    DAMAGE
    DPMS
    DRI2
    DRI3
    EVENT
    EWMH
    GLX
    ICCCM
    IMAGE
    KEYSYMS
    PRESENT
    RANDR
    RECORD
    RENDERUTIL
    RES
    SCREENSAVER
    SYNC
    UTIL
    XF86DRI
    XINERAMA
    XINPUT
    XKB
    XTEST
    XV
    XVMC
)

# default component info: xcb components have fairly predictable
# header files, library names and pkg-config names
foreach(_comp ${XCB_known_components})
    string(TOLOWER "${_comp}" _lc_comp)
    set(XCB_${_comp}_component_deps XCB)
    set(XCB_${_comp}_pkg_config "xcb-${_lc_comp}")
    set(XCB_${_comp}_lib "xcb-${_lc_comp}")
    set(XCB_${_comp}_header "xcb/${_lc_comp}.h")
endforeach()
# exceptions
set(XCB_XCB_component_deps)
set(XCB_COMPOSITE_component_deps XCB XFIXES)
set(XCB_DAMAGE_component_deps XCB XFIXES)
set(XCB_IMAGE_component_deps XCB SHM)
set(XCB_RENDERUTIL_component_deps XCB RENDER)
set(XCB_XFIXES_component_deps XCB RENDER SHAPE)
set(XCB_XVMC_component_deps XCB XV)
set(XCB_XV_component_deps XCB SHM)
set(XCB_XCB_pkg_config "xcb")
set(XCB_XCB_lib "xcb")
set(XCB_ATOM_header "xcb/xcb_atom.h")
set(XCB_ATOM_lib "xcb-util")
set(XCB_AUX_header "xcb/xcb_aux.h")
set(XCB_AUX_lib "xcb-util")
set(XCB_CURSOR_header "xcb/xcb_cursor.h")
set(XCB_EVENT_header "xcb/xcb_event.h")
set(XCB_EVENT_lib "xcb-util")
set(XCB_EWMH_header "xcb/xcb_ewmh.h")
set(XCB_ICCCM_header "xcb/xcb_icccm.h")
set(XCB_IMAGE_header "xcb/xcb_image.h")
set(XCB_KEYSYMS_header "xcb/xcb_keysyms.h")
set(XCB_PIXEL_header "xcb/xcb_pixel.h")
set(XCB_RENDERUTIL_header "xcb/xcb_renderutil.h")
set(XCB_RENDERUTIL_lib "xcb-render-util")
set(XCB_UTIL_header "xcb/xcb_util.h")

ecm_find_package_parse_components(XCB
    RESULT_VAR XCB_components
    KNOWN_COMPONENTS ${XCB_known_components}
    DEFAULT_COMPONENTS ${XCB_default_components}
)

ecm_find_package_handle_library_components(XCB
    COMPONENTS ${XCB_components}
)

find_package_handle_standard_args(XCB
    FOUND_VAR
        XCB_FOUND
    REQUIRED_VARS
        XCB_LIBRARIES
    VERSION_VAR
        XCB_VERSION
    HANDLE_COMPONENTS
)

include(FeatureSummary)
set_package_properties(XCB PROPERTIES
    URL "https://xcb.freedesktop.org/"
    DESCRIPTION "X protocol C-language Binding"
)
