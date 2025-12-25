#pragma once
#include <vulkan/vulkan.h>

namespace pvp
{
    enum class DiscriptorTag
    {
        nothing = 0,
        scene_globals,
        bindless_textures,
        lights,
        gbuffers,
        meshlets,
        big_buffers,
        pointers,
    };

}
