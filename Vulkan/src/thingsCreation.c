#include <thingsCreation.h>

void vk_createInstance(VkInstance* instance)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = "Vulkan2D";

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    uint32_t extensionCount;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

#ifdef __APPLE__
	instanceInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    extensionCount = glfwExtensionCount + 1;
    const char** requiredExtensions = malloc(sizeof(char*) * extensionCount);
    requiredExtensions[glfwExtensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
#else
    extensionCount = glfwExtensionCount;
    const char** requiredExtensions = malloc(sizeof(char*) * glfwExtensionCount);
#endif // __APPLE__
    
    memcpy(requiredExtensions, glfwExtensions, sizeof(const char*) * glfwExtensionCount);

    instanceInfo.enabledExtensionCount = extensionCount;
    instanceInfo.ppEnabledExtensionNames = requiredExtensions;

    CHECK_RESULT_VK(vkCreateInstance(&instanceInfo, NULL, instance));

    free(requiredExtensions);
}

void vk_createDevice(VkDevice* device)
{
    VkDeviceCreateInfo deviceInfo = {};
}