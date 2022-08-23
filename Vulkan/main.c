#include <thingsCreation.h>

int main()
{
    vulkanThings* vk_things = malloc(sizeof(vulkanThings));

    vk_createInstance(&vk_things->instance);

    GLFWwindow* window;

    if (!glfwInit())
    {
        return -1;
    }

    window = glfwCreateWindow(640, 480, "Start Window", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}