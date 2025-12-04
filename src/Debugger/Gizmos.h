#pragma once
#include "DebugVertex.h"

#include <glm/glm.hpp>

namespace pvp
{
    namespace gizmos
    {
        void draw_line(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color);
        void draw_cone(const glm::vec3& tip, float height, const glm::vec3& direction, float angle);
        void toggle_spheres();

        const std::vector<DebugVertex>& get_lines();
        void                            clear();
        bool                            is_spheres_enabled();
    } // namespace gizmos
} // namespace pvp