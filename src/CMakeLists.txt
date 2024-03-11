set(SRCS
    "data/htable.c"

    "init/config.c"
    "init/main.c"
    "init/sighandle.c"

    "manager/client/client.c"
    "manager/client/clientprops.c"
    "manager/client/clientset.c"
    "manager/multihead/monitor.c"
    "manager/multihead/monitorset.c"
    "manager/multihead/randr.c"
    "manager/multihead/xinerama.c"
    "manager/drag.c"
    "manager/events.c"
    "manager/session.c"

    "util/genutil.c"
    "util/path.c"
    "util/xstr.c"
)
set(TARGET_NAME "awm")

add_executable(${TARGET_NAME} ${SRCS})

target_compile_options(${TARGET_NAME} PRIVATE ${AWM_COMPILE_OPTIONS})
target_compile_definitions(${TARGET_NAME} PRIVATE ${AWM_MACROS})
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${AWM_DEPS_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/gen/")

string(REPLACE "." ";" AWM_SEMVER_DELIM ${AWM_VERSION})
list(GET AWM_SEMVER_DELIM 0 AWM_SEMVER_MAJOR)
list(GET AWM_SEMVER_DELIM 1 AWM_SEMVER_MINOR)
list(GET AWM_SEMVER_DELIM 2 AWM_SEMVER_PATCH)
configure_file("${AWM_GEN_DIR}/__version.h" "${CMAKE_CURRENT_BINARY_DIR}/gen/version.h" @ONLY)

# xcb libs
find_package(XCB QUIET REQUIRED COMPONENTS
    XCB
    RANDR
    XINERAMA
    ICCCM)
target_link_libraries(${TARGET_NAME} ${XCB_LIBRARIES})
target_include_directories(${TARGET_NAME} PRIVATE ${XCB_INCLUDE_DIRS})

# zf_log v0.4.1
add_subdirectory("${AWM_DEPS_DIR}/zf_log/" "${PROJECT_BINARY_DIR}/deps/zf_log/")
# inih r58
add_subdirectory("${AWM_DEPS_DIR}/inih/" "${PROJECT_BINARY_DIR}/deps/inih/")

target_link_libraries(${TARGET_NAME} zf_log inih)
