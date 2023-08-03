set(SRCS
    "main.c"
)
set(TARGET_NAME "awmstart")

add_executable(${TARGET_NAME} ${SRCS})

target_compile_options(${TARGET_NAME} PRIVATE ${AWM_COMPILE_OPTIONS})
target_compile_definitions(${TARGET_NAME} PRIVATE ${AWM_MACROS})

target_link_libraries(${TARGET_NAME} "libawm")
