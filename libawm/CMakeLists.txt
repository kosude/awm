set(SRCS
    "lib/htable.c"
    "lib/xstr.c"
)
set(TARGET_NAME "libawm")

add_library(${TARGET_NAME} ${SRCS})

# as lib is already part of the target name we don't want to append lib on it again and get 'liblibawm' :3
set_target_properties(${TARGET_NAME} PROPERTIES
    PREFIX "")

target_compile_options(${TARGET_NAME} PRIVATE ${AWM_COMPILE_OPTIONS})
target_compile_definitions(${TARGET_NAME} PUBLIC ${AWM_MACROS})

target_include_directories(${TARGET_NAME}
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include/"
        "${AWM_DEPS_DIR}/"
)

# zf_log v0.4.1
add_subdirectory("${AWM_DEPS_DIR}/zf_log/" "${PROJECT_BINARY_DIR}/deps/zf_log/")
target_link_libraries(${TARGET_NAME} zf_log)
