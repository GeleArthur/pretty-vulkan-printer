#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <Volk/volk.h>
#include <print>
#include <imgui.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

int main(int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::println("Couldn't initialize SDL: {}", SDL_GetError());
        return -1;
    }

    const SDL_WindowFlags window_flags =
        SDL_WINDOW_VULKAN |
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_HIGH_PIXEL_DENSITY |
        SDL_WINDOW_HIDDEN;

    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL3+Vulkan example", 1280, 720, window_flags);
    if (window == nullptr)
    {
        std::println("");
        return -1;
    }

    SDL_Vulkan_LoadLibrary(nullptr);
    // SDL_CreateRenderer()

    volkInitialize();

    SDL_ShowWindow(window);

    bool done = false;

    while (!done)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                done = true;
            }
        }
    }

    return 0;
}
