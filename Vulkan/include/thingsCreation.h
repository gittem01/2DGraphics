#ifndef THINGS_CREATION_H
#define THINGS_CREATION_H

#include <stdlib.h>
#include <definitions.h>
#include <GLFW/glfw3.h>
#include <string.h>


void vk_createInstance(St_vulkanThings* vulkanThings);
void vk_selectPhysicalDevice(St_vulkanThings* vulkanThings);
void vk_createLogicalDevice(St_vulkanThings* vulkanThings);
void vk_createSwapChain(St_vulkanThings* vulkanThings, GLFWwindow* window);
void vk_createRenderPass(St_vulkanThings* vulkanThings);
void vk_createFrameBuffers(St_vulkanThings* vulkanThings);
void vk_createCommands(St_vulkanThings* vulkanThings);
void vk_createSyncThings(St_vulkanThings* vulkanThings);


#endif // THINGS_CREATION_H