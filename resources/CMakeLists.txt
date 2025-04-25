
file(GLOB_RECURSE RESOURCE_FILES RELATIVE
        ${CMAKE_CURRENT_SOURCE_DIR} "*"
)
list(FILTER RESOURCE_FILES EXCLUDE REGEX "^CMakeLists\\.txt$")

add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)

foreach(file ${RESOURCE_FILES})

    add_custom_command(TARGET copy_resources POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${file} "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/${file}")
endforeach()