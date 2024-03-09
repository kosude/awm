function(awm_add_plugin TARGET_NAME SRCS)
    add_library(${TARGET_NAME} SHARED ${SRCS})
    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "") # remove 'lib' prefix
endfunction()

# test plugin (temp)
awm_add_plugin(test
    "test/plugin.c")

set(TESTCFG_DIR "${CMAKE_CURRENT_LIST_DIR}/../tests")
add_custom_target("awmtestcfg"
    ALL
    COMMAND source "${TESTCFG_DIR}/copy_plugins.sh" "--quiet"
    WORKING_DIRECTORY "${TESTCFG_DIR}")
