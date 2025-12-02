#include "Camera.h"

#include "imgui.h"

#include <GlfwToRender.h>
#include <Renderer/Swapchain.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
pvp::Camera::Camera(const Context& context)
    : m_context(context)
{
    m_projection = glm::perspective(glm::radians(45.0f), static_cast<float>(context.swapchain->get_swapchain_extent().width) / static_cast<float>(context.swapchain->get_swapchain_extent().height), 0.1f, 100.0f);
    m_projection[1][1] *= -1;
    // glfwGetCursorPos(m_context.window_surface->get_window(), &m_prev_mouse_x, &m_prev_mouse_y);
}
void pvp::Camera::update(float delta_time)
{
    m_projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_context.swapchain->get_swapchain_extent().width) / static_cast<float>(m_context.swapchain->get_swapchain_extent().height), 0.1f, 100.0f);
    m_projection[1][1] *= -1;

    double                         xpos{}, ypos{};
    bool                           pressed{};
    std::array<int, GLFW_KEY_LAST> keys{};
    {
        std::lock_guard lock{ m_context.gtfw_to_render->lock };
        xpos = m_context.gtfw_to_render->mouse_pos_x;
        ypos = m_context.gtfw_to_render->mouse_pos_y;
        pressed = m_context.gtfw_to_render->mouse_down[0];
        keys = m_context.gtfw_to_render->keys_pressed;
    }

    if (pressed && !ImGui::GetIO().WantCaptureMouse)
    {
        float xoffset = xpos - m_prev_mouse_x;
        float yoffset = m_prev_mouse_y - ypos;

        xoffset *= m_sensitivity;
        yoffset *= m_sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;
    }

    m_prev_mouse_x = xpos;
    m_prev_mouse_y = ypos;

    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        direction.y = sin(glm::radians(m_pitch));
        direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(direction);

        if (keys[GLFW_KEY_W] == GLFW_PRESS)
            m_position += m_speed * m_front * delta_time;
        if (keys[GLFW_KEY_S] == GLFW_PRESS)
            m_position -= m_speed * m_front * delta_time;
        if (keys[GLFW_KEY_A] == GLFW_PRESS)
            m_position -= glm::normalize(glm::cross(m_front, m_camera_up)) * m_speed * delta_time;
        if (keys[GLFW_KEY_D] == GLFW_PRESS)
            m_position += glm::normalize(glm::cross(m_front, m_camera_up)) * m_speed * delta_time;

        m_view = glm::lookAt(m_position, m_position + m_front, m_camera_up);
    }
}
pvp::FrustumCone pvp::Camera::get_cone()
{
    FrustumCone cone = {};
    cone.tip = m_position;
    cone.direction = m_front;
    cone.height = 100.0f;             // far clip from perspective matrix
    cone.angle = glm::radians(45.0f); // Guessing fov

    // if (fitFarClip)
    // {
    //     // View projection matrix
    //     auto& VP = this->GetViewProjectionMatrix();
    //     // Inverse view projection matrix
    //     auto invVP = glm::inverse(VP);
    //     // Clip space coordinates
    //     auto csFarTL = glm::vec3(-1, 1, 1);
    //     auto csFarBL = glm::vec3(-1, -1, 1);
    //     auto csFarBR = glm::vec3(1, -1, 1);
    //     auto csFarTR = glm::vec3(1, 1, 1);
    //     // Transform into view coordinates using inverse view projection matrix
    //     auto farTL = invVP * glm::vec4(csFarTL, 1.0f);
    //     auto farBL = invVP * glm::vec4(csFarBL, 1.0f);
    //     auto farBR = invVP * glm::vec4(csFarBR, 1.0f);
    //     auto farTR = invVP * glm::vec4(csFarTR, 1.0f);
    //     // Divide to finalize unproject
    //     farTL /= farTL.w;
    //     farBL /= farBL.w;
    //     farBR /= farBR.w;
    //     farTR /= farTR.w;
    //     // Find center of far clip plane
    //     auto farCenter = (farTL + farBL + farBR + farTR) / 4.0f;
    //     // Distance from far clip plane center to top left corner of far clip plane
    //     float r = glm::distance(farCenter, farTL);
    //     // Calculate angle using arctan
    //     cone.Angle = 2.0f * atan(r / mFarClip);
    // }
    return cone;
}