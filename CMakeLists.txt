cmake_minimum_required(VERSION 3.28)
project(vulken_tutorial VERSION 1.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME}
        src/main.cpp
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
        glm::glm
        Vulkan::Vulkan
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore *)