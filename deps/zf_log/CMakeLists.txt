# zf_log v0.4.1
#
# The MIT License (MIT)
#
# Copyright (c) 2017 wonder-mice
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.10) # From the Awm project -- MODIFIED from original: updated minimum version from 3.2

# zf_log target (required)
set(HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADERS zf_log.h)
set(SOURCES zf_log.c)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
# if(MSVC)
# 	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
# else()
# 	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic-errors")
# endif()

add_library(zf_log ${HEADERS} ${SOURCES})
target_include_directories(zf_log PUBLIC $<BUILD_INTERFACE:${HEADERS_DIR}>)
if(ZF_LOG_LIBRARY_PREFIX)
	target_compile_definitions(zf_log PRIVATE "ZF_LOG_LIBRARY_PREFIX=${ZF_LOG_LIBRARY_PREFIX}")
endif()
if(ZF_LOG_USE_ANDROID_LOG)
	target_compile_definitions(zf_log PRIVATE "ZF_LOG_USE_ANDROID_LOG")
	target_link_libraries(zf_log log)
endif()
if(ZF_LOG_USE_NSLOG)
	target_compile_definitions(zf_log PRIVATE "ZF_LOG_USE_NSLOG")
	target_link_libraries(zf_log "-framework CoreFoundation")
endif()
if(ZF_LOG_USE_DEBUGSTRING)
	target_compile_definitions(zf_log PRIVATE "ZF_LOG_USE_DEBUGSTRING")
endif()
if(ZF_LOG_OPTIMIZE_SIZE)
	target_compile_definitions(zf_log PRIVATE "ZF_LOG_OPTIMIZE_SIZE")
endif()

# install (optional)
if(ZF_LOG_CONFIGURE_INSTALL)
	if(NOT DEFINED INSTALL_INCLUDE_DIR)
		set(INSTALL_INCLUDE_DIR include)
	endif()
	if(NOT DEFINED INSTALL_LIB_DIR)
		set(INSTALL_LIB_DIR lib)
	endif()
	install(TARGETS zf_log EXPORT zf_log
		INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR}
		ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
	install(DIRECTORY ${HEADERS_DIR}/
		DESTINATION ${INSTALL_INCLUDE_DIR}
		FILES_MATCHING PATTERN "zf_*.h*")
endif()
