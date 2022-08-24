#ifndef THINGS_CREATION_H
#define THINGS_CREATION_H

#include <stdlib.h>
#include <definitions.h>
#include <GLFW/glfw3.h>
#include <string.h>

typedef struct
{
    VkQueue graphicsQueue;
    uint32_t graphicsQueueIndex;
} queueData;

typedef struct
{
    VkPhysicalDeviceProperties deviceProperties;
} vulkanInfo;

typedef struct
{
    vulkanInfo vulkan_info;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    queueData queues;

} vulkanThings;

void vk_createInstance(VkInstance* instance);
void vk_selectPhysicalDevice(vulkanThings* vulkan_things);
void vk_createLogicalDevice(vulkanThings* vulkan_things);


#endif // THINGS_CREATION_H