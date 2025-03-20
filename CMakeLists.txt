cmake_minimum_required(VERSION 3.28)
project(vulken_tutorial VERSION 1.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/stb_image.h
        src/tiny_obj_loader.h
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

include(FetchContent)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG release-1.0.2
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

set(GLFW_BUILD_WAYLAND FALSE) # Try out wayland later
set(GLFW_BUILD_X11 TRUE)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

include(FindVulkan REQUIRED)
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan SDK not installed.")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glm::glm-header-only
        Vulkan::Vulkan
)

#target_compile_options(${PROJECT_NAME} PRIVATE
#        $<$<CXX_COMPILER_ID:MSVC>:/W4>
#        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
#)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore *)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources")

file(GLOB resources_files "${CMAKE_SOURCE_DIR}/resources/*.frag" "${CMAKE_SOURCE_DIR}/resources/*.vert")
foreach (shader ${resources_files})
    get_filename_component(file_name ${shader} NAME)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
            glslc ARGS
            ${shader} -o "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/${file_name}.spv")
endforeach ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/textures" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/textures"
)
