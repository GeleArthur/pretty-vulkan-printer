cmake_minimum_required(VERSION 3.28)
project(pretty-vulkan-printer VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(libraries)
add_subdirectory(resources)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/App.cpp
        src/App.h
        src/DestructorQueue.cpp
        src/DestructorQueue.h
        src/PVPInstance/PVPInstance.cpp
        src/PVPInstance/PVPInstance.h
        src/VulkanExternalFunctions.h

        src/stb_image.h
        src/tiny_obj_loader.h
        src/PVPPhysicalDevice/PVPPhysicalDevice.cpp
        src/PVPPhysicalDevice/PVPPhysicalDevice.h
        src/PVPSwapChain/PVPSwapChain.cpp
        src/PVPSwapChain/PVPSwapChain.h
        src/PVPPhysicalDevice/PVPDeviceBuilder.cpp
        src/PVPPhysicalDevice/PVPDeviceBuilder.h
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} pretty-vulkan-printer-libraries)

include(shaders/compile_shaders.cmake)

#target_compile_options(${PROJECT_NAME} PRIVATE
#        $<$<CXX_COMPILER_ID:MSVC>:/W4>
#        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
#)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore *)
