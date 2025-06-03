#include "Camera.h"

#include <Renderer/Swapchain.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
pvp::Camera::Camera(const Context& context)
    : m_context(context)
{
    m_projection = glm::perspective(glm::radians(45.0f), static_cast<float>(context.swapchain->get_swapchain_extent().width) / static_cast<float>(context.swapchain->get_swapchain_extent().height), 0.1f, 100.0f);
    m_projection[1][1] *= -1;
    glfwGetCursorPos(m_context.window_surface->get_window(), &m_prev_mouse_x, &m_prev_mouse_y);
}
void pvp::Camera::update(float delta_time)
{
    double xpos, ypos;
    glfwGetCursorPos(m_context.window_surface->get_window(), &xpos, &ypos);

    if (glfwGetMouseButton(m_context.window_surface->get_window(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
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

    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(direction);

    if (glfwGetKey(m_context.window_surface->get_window(), GLFW_KEY_W) == GLFW_PRESS)
        m_position += m_speed * m_front * delta_time;
    if (glfwGetKey(m_context.window_surface->get_window(), GLFW_KEY_S) == GLFW_PRESS)
        m_position -= m_speed * m_front * delta_time;
    if (glfwGetKey(m_context.window_surface->get_window(), GLFW_KEY_A) == GLFW_PRESS)
        m_position -= glm::normalize(glm::cross(m_front, m_camera_up)) * m_speed * delta_time;
    if (glfwGetKey(m_context.window_surface->get_window(), GLFW_KEY_D) == GLFW_PRESS)
        m_position += glm::normalize(glm::cross(m_front, m_camera_up)) * m_speed * delta_time;

    m_view = glm::lookAt(m_position, m_position + m_front, m_camera_up);
}