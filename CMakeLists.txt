cmake_minimum_required(VERSION 3.28)
project(pretty-vulkan-printer VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(libraries)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Window.cpp
        src/Window.h
        src/stb_image.h
        src/tiny_obj_loader.h
)

target_link_libraries(${PROJECT_NAME} pretty-vulkan-printer-libraries)

#target_link_libraries(${PROJECT_NAME} PRIVATE
#        glfw
#        glm::glm-header-only
#        imgui
#        Vulkan::Vulkan
#        VulkanMemoryAllocator
#        spdlog::spdlog
#        assimp::assimp
#)


#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders")
#file(GLOB resources_files "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert")
#foreach (shader ${resources_files})
#    get_filename_component(file_name ${shader} NAME)
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
#            glslc ARGS
#            ${shader} -o "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/${file_name}.spv")
#endforeach ()

#target_compile_options(${PROJECT_NAME} PRIVATE
#        $<$<CXX_COMPILER_ID:MSVC>:/W4>
#        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
#)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore *)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources")
