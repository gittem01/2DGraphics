#ifndef THINGS_CREATION_H
#define THINGS_CREATION_H

#include <stdlib.h>
#include <definitions.h>
#include <GLFW/glfw3.h>
#include <string.h>

void vk_createSwapChain(vulkanThings* vk_things, GLFWwindow* window);

void vk_createInstance(vulkanThings* vulkan_things);
void vk_selectPhysicalDevice(vulkanThings* vulkan_things);
void vk_createLogicalDevice(vulkanThings* vulkan_things);
void vk_createRenderPass(vulkanThings* vulkan_things);
void vk_createFrameBuffers(vulkanThings* vulkan_things);

#endif // THINGS_CREATION_H