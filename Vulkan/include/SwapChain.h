#pragma once

#include <stdlib.h>
#include <definitions.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

class ThinDrawer;

class SwapChain
{
public:
    ThinDrawer* thinDrawer;

    VkSwapchainKHR swapChain;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D extent;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    int surfaceFormatCount = 0;
    VkSurfaceFormatKHR* surfaceFormats;
    int presentModeCount = 0;
    VkPresentModeKHR* presentModes;

    VkImage* images;
    VkImageView* imageViews;
    VkFramebuffer* frameBuffers;
    int imageCount;

    SwapChain(ThinDrawer* td);

    void querySwapChainSupport();
    void createSwapChain(GLFWwindow* window);
    void createFrameBuffers();
};