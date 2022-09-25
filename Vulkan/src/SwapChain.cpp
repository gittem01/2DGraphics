#include <SwapChain.h>
#include <ThinDrawer.h>

#define MIN(a, b) a > (b) ? (a) : b
#define MAX(a, b) a < (b) ? (a) : b

SwapChain::SwapChain(ThinDrawer* td)
{
    thinDrawer = td;
}

void SwapChain::querySwapChainSupport()
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(thinDrawer->physicalDevice, surface, &surfaceCapabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(thinDrawer->physicalDevice, surface, (uint32_t*)&surfaceFormatCount, NULL);
    if (surfaceFormatCount != 0)
    {
        surfaceFormats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(thinDrawer->physicalDevice, surface, (uint32_t*)&surfaceFormatCount, surfaceFormats);
    }
    else
    {
        surfaceFormats = NULL;
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(thinDrawer->physicalDevice, surface, (uint32_t*)&presentModeCount, NULL);
    if (presentModeCount != 0)
    {
        presentModes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(thinDrawer->physicalDevice, surface, (uint32_t*)&presentModeCount, presentModes);
    }
}

void SwapChain::createSwapChain(GLFWwindow* window)
{
    querySwapChainSupport();

    if (surfaceFormatCount == 0 || presentModeCount == 0)
    {
        printf("surfaceFormatCount == 0 || presentModeCount == 0\n");
        exit(1);
    }

    int i;
    for (i = 0; i < surfaceFormatCount; i++)
    {
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = surfaceFormats[i];
            break;
        }
    }
    if (i == surfaceFormatCount)
    {
        surfaceFormat = surfaceFormats[0];
    }

    VkPresentModeKHR presentMode;
    int mask = 0;
    for (i = 0; i < presentModeCount; i++)
    {
        mask |= (int)presentModes[i];
    }
    if (mask & VK_PRESENT_MODE_MAILBOX_KHR)
    {
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else if (mask & VK_PRESENT_MODE_FIFO_KHR)
    {
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else
    {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        extent = surfaceCapabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D extent;

        extent.width = MIN(MAX(width, surfaceCapabilities.minImageExtent.width), surfaceCapabilities.maxImageExtent.width);
        extent.height = MIN(MAX(height, surfaceCapabilities.minImageExtent.height), surfaceCapabilities.maxImageExtent.height);

        extent = extent;
    }

    imageCount = (int)surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
    {
        imageCount = (int)surfaceCapabilities.maxImageCount;
    }
    else
    {
        imageCount = (int)surfaceCapabilities.minImageCount + 1;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo = { };
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageExtent = extent;

    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = VK_TRUE;

    CHECK_RESULT_VK(vkCreateSwapchainKHR(thinDrawer->logicalDevice, &swapChainCreateInfo, NULL, &swapChain))

    vkGetSwapchainImagesKHR(thinDrawer->logicalDevice, swapChain,
        (uint32_t*)&imageCount, NULL);
    images = (VkImage*)malloc(sizeof(VkImage) * imageCount);
    imageViews = (VkImageView*)malloc(sizeof(VkImageView) * imageCount);
    vkGetSwapchainImagesKHR(thinDrawer->logicalDevice, swapChain,
        (uint32_t*)&imageCount, images);

    for (i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo createInfo = { };
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = surfaceFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        CHECK_RESULT_VK(vkCreateImageView(thinDrawer->logicalDevice, &createInfo, NULL, &imageViews[i]))
    }
}

void SwapChain::createFrameBuffers()
{
    VkFramebufferCreateInfo frameBufferInfo = { };
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

	frameBufferInfo.renderPass = thinDrawer->renderPass;
	frameBufferInfo.width = extent.width;
	frameBufferInfo.height = extent.height;
	frameBufferInfo.layers = 1;

	frameBuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * imageCount);

	for (int i = 0; i < imageCount; i++)
    {
        VkImageView attachments[] = { imageViews[i] };
        
		frameBufferInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
		frameBufferInfo.pAttachments = attachments;

		vkCreateFramebuffer(thinDrawer->logicalDevice, &frameBufferInfo, NULL, frameBuffers + i);
	}
}