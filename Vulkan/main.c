#include <thingsCreation.h>

vulkanThings* getVulkanThings(){
    vulkanThings * vk_things = malloc(sizeof(vulkanThings));
    vk_things->vulkan_info = malloc(sizeof(vulkanInfo));
    vk_things->swapChainData = malloc(sizeof(swapChainData));

    return vk_things;
}

int main()
{
    vulkanThings* vk_things = getVulkanThings();

    GLFWwindow* window;

    if (!glfwInit())
    {
        printf("!glfwInit()\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(640, 480, "Start Window", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(1);
    }

    vk_createInstance(vk_things);

    CHECK_RESULT_VK(glfwCreateWindowSurface(vk_things->instance, window, NULL, &vk_things->surface))

    vk_selectPhysicalDevice(vk_things);
    vk_createLogicalDevice(vk_things);
    vk_createSwapChain(vk_things, window);
    vk_createRenderPass(vk_things);
    vk_createFrameBuffers(vk_things);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwTerminate();
    
    return 0;
}