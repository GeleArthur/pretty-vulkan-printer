cmake_minimum_required(VERSION 3.28)
project(hades-hot-pot VERSION 1.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

include(scripts/install_sdl3.cmake)

add_executable(${PROJECT_NAME}
        src/main.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

include(FindVulkan REQUIRED)
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan SDK not installed.")
endif ()

install_sdl3()
install_glm()
install_imgui()
install_volk()
install_vma()


target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3::SDL3
        glm::glm-header-only
        imgui
        Vulkan::Vulkan
        volk
        VulkanMemoryAllocator
)

#target_compile_options(${PROJECT_NAME} PRIVATE
#        $<$<CXX_COMPILER_ID:MSVC>:/W4>
#        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
#)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore *)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources")
