function(install_glm)
    include(FetchContent)
    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG release-1.0.2
            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glm)
endfunction()

function(install_sdl3)
    include(FetchContent)
    FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.2.8
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(SDL3)
endfunction()

function(install_imgui)
    include(FetchContent)
    FetchContent_Declare(
            imgui
            URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.zip
    )
    FetchContent_MakeAvailable(imgui)

    add_library(imgui)
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
    target_sources(imgui PRIVATE
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp

            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.h
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
            ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    )
    install_sdl3()
    target_link_libraries(imgui PRIVATE
            SDL3::SDL3
            Vulkan::Vulkan
    )
endfunction()

function(install_volk)
    add_library(volk)
    target_sources(volk PRIVATE
            $ENV{VULKAN_SDK}/Include/Volk/volk.c
            $ENV{VULKAN_SDK}/Include/Volk/volk.h
    )
    target_link_libraries(volk PRIVATE
            Vulkan::Vulkan
    )
endfunction()

function(install_vma)
    include(FetchContent)
    FetchContent_Declare(
            vma
            URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.2.1.zip
    )
    FetchContent_MakeAvailable(vma)
    

endfunction()