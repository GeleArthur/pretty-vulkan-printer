# shaders handler

file(GLOB resources_files "${CMAKE_SOURCE_DIR}/resources/*.frag" "${CMAKE_SOURCE_DIR}/resources/*.vert")
foreach (shader ${resources_files})
    get_filename_component(file_name ${shader} NAME)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
            glslc ARGS
            ${shader} -o "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/${file_name}.spv")
endforeach ()