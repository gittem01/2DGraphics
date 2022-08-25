#include <thingsCreation.h>

St_vulkanThings* getVulkanThings(){
    St_vulkanThings * vulkanThings = malloc(sizeof(St_vulkanThings));
    vulkanThings->vulkan_info = malloc(sizeof(St_vulkanInfo));
    vulkanThings->swapChainData = malloc(sizeof(St_swapChainData));

    return vulkanThings;
}

int main()
{
    St_vulkanThings* vulkanThings = getVulkanThings();

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

    vk_createInstance(vulkanThings);

    CHECK_RESULT_VK(glfwCreateWindowSurface(vulkanThings->instance, window, NULL, &vulkanThings->surface))

    vk_selectPhysicalDevice(vulkanThings);
    vk_createLogicalDevice(vulkanThings);
    vk_createSwapChain(vulkanThings, window);
    vk_createRenderPass(vulkanThings);
    vk_createFrameBuffers(vulkanThings);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}