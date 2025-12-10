#include "Gizmos.h"

#include "DebugVertex.h"
#include <cmath>
#include <numbers>

namespace
{
    std::vector<pvp::DebugVertex> lines;
    bool                          spheres_enabled;
} // namespace

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

        double rot_angle = (i / 36.0f) * std::numbers::pi * 2.0f;
        lines.push_back(DebugVertex{ tip + (dir1 * angle * static_cast<float>(std::cos(rot_angle)) + dir2 * angle * static_cast<float>(std::sin(rot_angle)) + direction) * height, { 1, 1, 1, 1 } });

        lines.push_back(DebugVertex{ tip + (dir1 * angle * static_cast<float>(std::cos(rot_angle)) + dir2 * angle * static_cast<float>(std::sin(rot_angle)) + direction) * height, { 1, 1, 1, 1 } });
        lines.push_back(DebugVertex{ tip + (dir1 * angle * static_cast<float>(std::cos(rot_angle + ((1 / 36.0f) * std::numbers::pi * 2.0f))) + dir2 * angle * static_cast<float>(std::sin(rot_angle + ((1 / 36.0f) * std::numbers::pi * 2.0f))) + direction) * height, { 1, 1, 1, 1 } });
    }
}
void pvp::gizmos::toggle_spheres()
{
    spheres_enabled = !spheres_enabled;
}

const std::vector<pvp::DebugVertex>& pvp::gizmos::get_lines()
{
    return lines;
}
void pvp::gizmos::clear()
{
    lines.clear();
}
bool pvp::gizmos::is_spheres_enabled()
{
    return spheres_enabled;
}
