#include <exception>
#include <iostream>
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTraingle
{
public:
    void run()
    {
        init_vulkan();
        main_loop();
        clean_up();
    }

private:
    void init_vulkan()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void main_loop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }

    void clean_up()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    GLFWwindow* window{};

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
};

int main()
{
    HelloTraingle app;
    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
