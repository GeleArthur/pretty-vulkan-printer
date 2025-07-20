#include <tracy/Tracy.hpp>

#include "App.h"

#include <iostream>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Easy memory leak detector.
#if WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

int main()
{
    ZoneScoped;

#if WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    pvp::App{}.run();
    try
    {
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
