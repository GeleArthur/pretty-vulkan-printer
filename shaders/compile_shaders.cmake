# shaders handler


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders")
file(GLOB resources_files "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert")
foreach (shader ${resources_files})
    get_filename_component(file_name ${shader} NAME)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
            glslc ARGS
            ${shader} -o "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/${file_name}.spv")
endforeach ()