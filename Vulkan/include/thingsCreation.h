#ifndef THINGS_CREATION_H
#define THINGS_CREATION_H

#include <stdlib.h>
#include <definitions.h>
#include <GLFW/glfw3.h>
#include <string.h>

typedef struct
{
    VkInstance instance;
} vulkanThings;


void vk_createInstance(VkInstance* instance);
void vk_createDevice(VkDevice* device);


#endif // THINGS_CREATION_H