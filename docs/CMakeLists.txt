find_package("Sphinx" REQUIRED)

add_custom_target("${PROJECT_NAME}doc"
    ALL
    COMMAND source "${CMAKE_CURRENT_LIST_DIR}/conf_wrapper.sh" "${SPHINX_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}"
                   "${CMAKE_CURRENT_BINARY_DIR}" "${AWM_TOOLS_DIR}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    COMMENT "Building HTML documentation")
