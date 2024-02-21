set(SRCS
    "data/htable.c"

    "init/main.c"
    "init/sighandle.c"

    "manager/multihead/monitor.c"
    "manager/multihead/monitorset.c"
    "manager/multihead/randr.c"
    "manager/multihead/xinerama.c"
    "manager/client.c"
    "manager/clientset.c"
    "manager/drag.c"
    "manager/events.c"
    "manager/session.c"

    "util/xstr.c"
)
set(TARGET_NAME "awm")

add_executable(${TARGET_NAME} ${SRCS})

target_compile_options(${TARGET_NAME} PRIVATE ${AWM_COMPILE_OPTIONS})
target_compile_definitions(${TARGET_NAME} PRIVATE ${AWM_MACROS})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} "${AWM_DEPS_DIR}/")

# xcb libs
find_package(XCB QUIET REQUIRED COMPONENTS XCB RANDR XINERAMA)
target_link_libraries(${TARGET_NAME} ${XCB_LIBRARIES})
target_include_directories(${TARGET_NAME} PRIVATE ${XCB_INCLUDE_DIRS})

# zf_log v0.4.1
add_subdirectory("${AWM_DEPS_DIR}/zf_log/" "${PROJECT_BINARY_DIR}/deps/zf_log/")
target_link_libraries(${TARGET_NAME} zf_log)
