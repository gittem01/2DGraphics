#include <thingsCreation.h>

int main()
{
    vulkanThings* vk_things = malloc(sizeof(vulkanThings));

    GLFWwindow* window;

    if (!glfwInit())
    {
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(640, 480, "Start Window", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    vk_createInstance(&vk_things->instance);

    CHECK_RESULT_VK(glfwCreateWindowSurface(vk_things->instance, window, NULL, &vk_things->surface));

    vk_selectPhysicalDevice(vk_things);
    vk_createLogicalDevice(vk_things);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}