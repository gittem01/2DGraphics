#include <thingsCreation.h>

VkSurfaceCapabilitiesKHR surfaceCapabilities;
static int surfaceFormatCount = 0;
static VkSurfaceFormatKHR* surfaceFormats;
static int presentModeCount = 0;
static VkPresentModeKHR* presentModes;

#define MIN(a, b) a > (b) ? (a) : b
#define MAX(a, b) a < (b) ? (a) : b

void querySwapChainSupport(St_vulkanThings* vulkanThings)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanThings->physicalDevice, vulkanThings->surface, &surfaceCapabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanThings->physicalDevice, vulkanThings->surface, (uint32_t*)&surfaceFormatCount, NULL);
    if (surfaceFormatCount != 0) {
        surfaceFormats = malloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanThings->physicalDevice, vulkanThings->surface, (uint32_t*)&surfaceFormatCount, surfaceFormats);
    }
    else{
        surfaceFormats = NULL;
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanThings->physicalDevice, vulkanThings->surface, (uint32_t*)&presentModeCount, NULL);
    if (presentModeCount != 0) {
        presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanThings->physicalDevice, vulkanThings->surface, (uint32_t*)&presentModeCount, presentModes);
    }
}

void vk_createSwapChain(St_vulkanThings* vulkanThings, GLFWwindow* window)
{
    querySwapChainSupport(vulkanThings);

    if (surfaceFormatCount == 0 || presentModeCount == 0){
        printf("surfaceFormatCount == 0 || presentModeCount == 0\n");
        exit(1);
    }

    int i;
    for (i = 0; i < surfaceFormatCount; i++) {
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            vulkanThings->swapChainData->surfaceFormat = surfaceFormats[i];
            break;
        }
    }
    if (i == surfaceFormatCount) {
        vulkanThings->swapChainData->surfaceFormat = surfaceFormats[0];
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
        vulkanThings->swapChainData->extent = surfaceCapabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D extent;

        extent.width = MIN(MAX(width, surfaceCapabilities.minImageExtent.width), surfaceCapabilities.maxImageExtent.width);
        extent.height = MIN(MAX(height, surfaceCapabilities.minImageExtent.height), surfaceCapabilities.maxImageExtent.height);

        vulkanThings->swapChainData->extent = extent;
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
    swapChainCreateInfo.surface = vulkanThings->surface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageExtent = vulkanThings->swapChainData->extent;

    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapChainCreateInfo.imageFormat = vulkanThings->swapChainData->surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = vulkanThings->swapChainData->surfaceFormat.colorSpace;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = VK_TRUE;

    CHECK_RESULT_VK(vkCreateSwapchainKHR(vulkanThings->logicalDevice, &swapChainCreateInfo, NULL, &vulkanThings->swapChainData->swapChain))

    vkGetSwapchainImagesKHR(vulkanThings->logicalDevice, vulkanThings->swapChainData->swapChain,
        (uint32_t*)&vulkanThings->swapChainData->imageCount, NULL);
    vulkanThings->swapChainData->images = malloc(sizeof(VkImage) * vulkanThings->swapChainData->imageCount);
    vulkanThings->swapChainData->imageViews = malloc(sizeof(VkImageView) * vulkanThings->swapChainData->imageCount);
    vkGetSwapchainImagesKHR(vulkanThings->logicalDevice, vulkanThings->swapChainData->swapChain,
        (uint32_t*)&vulkanThings->swapChainData->imageCount, vulkanThings->swapChainData->images);

    for (i = 0; i < vulkanThings->swapChainData->imageCount; i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vulkanThings->swapChainData->images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vulkanThings->swapChainData->surfaceFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        CHECK_RESULT_VK(vkCreateImageView(vulkanThings->logicalDevice, &createInfo, NULL, &vulkanThings->swapChainData->imageViews[i]))
    }
}

void vk_createFrameBuffers(St_vulkanThings* vulkanThings){
    VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

	frameBufferInfo.renderPass = vulkanThings->renderPass;
	frameBufferInfo.width = vulkanThings->swapChainData->extent.width;
	frameBufferInfo.height = vulkanThings->swapChainData->extent.height;
	frameBufferInfo.layers = 1;

	vulkanThings->swapChainData->frameBuffers = malloc(sizeof(VkFramebuffer) * vulkanThings->swapChainData->imageCount);

	for (int i = 0; i < vulkanThings->swapChainData->imageCount; i++) {
        VkImageView attachments[] = { vulkanThings->swapChainData->imageViews[i] };
        
		frameBufferInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
		frameBufferInfo.pAttachments = attachments;

		vkCreateFramebuffer(vulkanThings->logicalDevice, &frameBufferInfo, NULL, vulkanThings->swapChainData->frameBuffers + i);
	}
}