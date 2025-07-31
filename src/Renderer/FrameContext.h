#pragma once
#include <Buffer/Buffer.h>

struct FrameContext
{
    VkCommandBuffer command_buffer;
    int             buffer_index;
};
