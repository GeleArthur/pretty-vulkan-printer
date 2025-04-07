
file(GLOB_RECURSE RESOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*
)

message(STATUS "WASSSUP ${PROJECT_NAME}")

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/resources COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources")


foreach (file ${RESOURCE_FILES})

endforeach ()