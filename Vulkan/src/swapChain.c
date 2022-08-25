#include <thingsCreation.h>

VkSurfaceCapabilitiesKHR surfaceCapabilities;
static int surfaceFormatCount = 0;
static VkSurfaceFormatKHR* surfaceFormats;
static int presentModeCount = 0;
static VkPresentModeKHR* presentModes;

#define MIN(a, b) a > (b) ? (a) : b
#define MAX(a, b) a < (b) ? (a) : b

void querySwapChainSupport(vulkanThings* vk_things)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_things->physicalDevice, vk_things->surface, &surfaceCapabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_things->physicalDevice, vk_things->surface, (uint32_t*)&surfaceFormatCount, NULL);
    if (surfaceFormatCount != 0) {
        surfaceFormats = malloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_things->physicalDevice, vk_things->surface, (uint32_t*)&surfaceFormatCount, surfaceFormats);
    }
    else{
        surfaceFormats = NULL;
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_things->physicalDevice, vk_things->surface, (uint32_t*)&presentModeCount, NULL);
    if (presentModeCount != 0) {
        presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_things->physicalDevice, vk_things->surface, (uint32_t*)&presentModeCount, presentModes);
    }
}

void vk_createSwapChain(vulkanThings* vk_things, GLFWwindow* window)
{
    querySwapChainSupport(vk_things);

    if (surfaceFormatCount == 0 || presentModeCount == 0){
        printf("surfaceFormatCount == 0 || presentModeCount == 0\n");
        exit(1);
    }

    int i;
    for (i = 0; i < surfaceFormatCount; i++) {
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            vk_things->swapChainData->surfaceFormat = surfaceFormats[i];
            break;
        }
    }
    if (i == surfaceFormatCount) {
        vk_things->swapChainData->surfaceFormat = surfaceFormats[0];
    }

    VkPresentModeKHR presentMode;
    int mask = 0;
    for (i = 0; i < presentModeCount; i++) {
        mask |= (int)presentModes[i];
    }
    if (mask & VK_PRESENT_MODE_MAILBOX_KHR){
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else if (mask & VK_PRESENT_MODE_FIFO_KHR){
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else{
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
        vk_things->swapChainData->extent = surfaceCapabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D extent;

        extent.width = MIN(MAX(width, surfaceCapabilities.minImageExtent.width), surfaceCapabilities.maxImageExtent.width);
        extent.height = MIN(MAX(height, surfaceCapabilities.minImageExtent.height), surfaceCapabilities.maxImageExtent.height);

        vk_things->swapChainData->extent = extent;
    }

    int imageCount = (int)surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
        imageCount = (int)surfaceCapabilities.maxImageCount;
    }
    else {
        imageCount = (int)surfaceCapabilities.minImageCount + 1;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = vk_things->surface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageExtent = vk_things->swapChainData->extent;

    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapChainCreateInfo.imageFormat = vk_things->swapChainData->surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = vk_things->swapChainData->surfaceFormat.colorSpace;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = VK_TRUE;

    CHECK_RESULT_VK(vkCreateSwapchainKHR(vk_things->logicalDevice, &swapChainCreateInfo, NULL, &vk_things->swapChainData->swapChain))

    vkGetSwapchainImagesKHR(vk_things->logicalDevice, vk_things->swapChainData->swapChain,
        (uint32_t*)&vk_things->swapChainData->imageCount, NULL);
    vk_things->swapChainData->images = malloc(sizeof(VkImage) * vk_things->swapChainData->imageCount);
    vk_things->swapChainData->imageViews = malloc(sizeof(VkImageView) * vk_things->swapChainData->imageCount);
    vkGetSwapchainImagesKHR(vk_things->logicalDevice, vk_things->swapChainData->swapChain,
        (uint32_t*)&vk_things->swapChainData->imageCount, vk_things->swapChainData->images);

    for (i = 0; i < vk_things->swapChainData->imageCount; i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vk_things->swapChainData->images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vk_things->swapChainData->surfaceFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        CHECK_RESULT_VK(vkCreateImageView(vk_things->logicalDevice, &createInfo, NULL, &vk_things->swapChainData->imageViews[i]))
    }
}

void vk_createFrameBuffers(vulkanThings* vulkan_things){
    VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

	frameBufferInfo.renderPass = vulkan_things->renderPass;
	frameBufferInfo.width = vulkan_things->swapChainData->extent.width;
	frameBufferInfo.height = vulkan_things->swapChainData->extent.height;
	frameBufferInfo.layers = 1;

	vulkan_things->swapChainData->frameBuffers = malloc(sizeof(VkFramebuffer) * vulkan_things->swapChainData->imageCount);

	for (int i = 0; i < vulkan_things->swapChainData->imageCount; i++) {
        VkImageView attachments[] = { vulkan_things->swapChainData->imageViews[i] };
        
		frameBufferInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
		frameBufferInfo.pAttachments = attachments;

		vkCreateFramebuffer(vulkan_things->logicalDevice, &frameBufferInfo, NULL, vulkan_things->swapChainData->frameBuffers + i);
	}
}