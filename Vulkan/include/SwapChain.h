#pragma once

#include <definitions.h>
#include <GLFW/glfw3.h>
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
    VkImage colorImage;
    VkImageView colorImageView;
    VkDeviceMemory colorImageMemory;
    VkFramebuffer* frameBuffers;
    int imageCount;

    SwapChain(ThinDrawer* td);

    void destroy();

    VkExtent2D chooseSwapExtent();
    void querySwapChainSupport();
    void createSwapChain();
    void createFrameBuffers();
    void createColorResources();

    void creationLoop();
};