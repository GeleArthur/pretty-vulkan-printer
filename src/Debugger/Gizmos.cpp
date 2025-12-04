#include "Gizmos.h"

#include "DebugVertex.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace
{
    std::vector<pvp::DebugVertex> lines;
}

void pvp::gizmos::draw_line(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color)
{
    lines.push_back(DebugVertex{ p1, color });
    lines.push_back(DebugVertex{ p2, color });
}

void pvp::gizmos::draw_cone(const glm::vec3& tip, float height, const glm::vec3& direction, float angle)
{
    glm::vec<3, float> dir1 = glm::normalize(glm::cross(direction, glm::vec3{ 1, 0, 0 }));
    glm::vec<3, float> dir2 = glm::normalize(glm::cross(direction, dir1));

    for (int i = 0; i < 36; ++i)
    {
        lines.push_back(DebugVertex{ tip, { 1, 1, 1, 1 } });

        double rot_angle = (i / 36.0f) * M_PI * 2.0f;
        lines.push_back(DebugVertex{ tip + dir1 * angle * std::cosf(rot_angle) + dir2 * angle * std::sinf(rot_angle) + direction * height, { 1, 1, 1, 1 } });
    }
}
const std::vector<pvp::DebugVertex>& pvp::gizmos::get_lines()
{
    return lines;
}
void pvp::gizmos::clear()
{
    lines.clear();
}
