#pragma once

struct FrameContext;
namespace pvp
{
    struct Context;
    class ImguiRenderer final
    {
    public:
        explicit ImguiRenderer(const pvp::Context& context);
        void destroy();
        void render_frame(const FrameContext& frame_context);
    };
} // namespace pvp
