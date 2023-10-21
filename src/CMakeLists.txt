set(SRCS
    "init/main.c"

    "util/x_to_str.c"
)
set(TARGET_NAME "awm")

add_executable(${TARGET_NAME} ${SRCS})

target_compile_options(${TARGET_NAME} PRIVATE ${AWM_COMPILE_OPTIONS})
target_compile_definitions(${TARGET_NAME} PRIVATE ${AWM_MACROS})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${TARGET_NAME} "libawm")

# find and link to xcb libs
find_package(XCB QUIET REQUIRED
    COMPONENTS XCB)
target_link_libraries(${TARGET_NAME}
    ${XCB_LIBRARIES})
target_include_directories(${TARGET_NAME}
    PRIVATE
        ${XCB_INCLUDE_DIRS})
