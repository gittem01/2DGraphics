#include <thingsCreation.h>

const char* ENABLE_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const int NUM_ENABLE_EXTENSIONS = sizeof(ENABLE_EXTENSIONS) / sizeof(ENABLE_EXTENSIONS[0]);

void vk_createInstance(St_vulkanThings* vulkanThings)
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
    extensionCount = glfwExtensionCount + 2;
    const char** requiredExtensions = malloc(sizeof(char*) * extensionCount);
    requiredExtensions[glfwExtensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    requiredExtensions[glfwExtensionCount + 1] = "VK_KHR_get_physical_device_properties2";
#else
    extensionCount = glfwExtensionCount;
    const char** requiredExtensions = malloc(sizeof(char*) * glfwExtensionCount);
#endif // __APPLE__
    
    memcpy(requiredExtensions, glfwExtensions, sizeof(const char*) * glfwExtensionCount);

    instanceInfo.enabledExtensionCount = extensionCount;
    instanceInfo.ppEnabledExtensionNames = requiredExtensions;

    CHECK_RESULT_VK(vkCreateInstance(&instanceInfo, NULL, &vulkanThings->instance))

    free(requiredExtensions);
}

static int isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkSurfaceFormatKHR* surfaceFormats;
	VkPresentModeKHR* presentModes;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	int formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, (uint32_t*)&formatCount, NULL);
	if (formatCount != 0) {
		surfaceFormats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, (uint32_t*)&formatCount, surfaceFormats);
	}

	int presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, (uint32_t*)&presentModeCount, NULL);
	if (presentModeCount != 0) {
		presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, (uint32_t*)&presentModeCount, presentModes);
	}
    if (formatCount != 0){
        free(surfaceFormats);
    }
	if (presentModeCount != 0){
        free(presentModes);
    }

	return presentModeCount != 0 && formatCount != 0;
}

void vk_selectPhysicalDevice(St_vulkanThings* vulkanThings)
{
    uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vulkanThings->instance, &deviceCount, NULL);
	if (deviceCount == 0) {
		printf("failed to find GPUs with Vulkan support!");
        exit(1);
	}
	VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);

	vkEnumeratePhysicalDevices(vulkanThings->instance, &deviceCount, devices);

	vulkanThings->physicalDevice = VK_NULL_HANDLE;

	for (uint32_t i = 0; i < deviceCount; i++) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, NULL);
		VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilies);

		if (isDeviceSuitable(devices[i], vulkanThings->surface)) {
			for (uint32_t j = 0; j < queueFamilyCount; j++) {
				VkBool32 presentSupported;
				vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, vulkanThings->surface, &presentSupported);
				if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupported) {
					vulkanThings->physicalDevice = devices[i];
					vulkanThings->queues.graphicsQueueIndex = j;

	                vkGetPhysicalDeviceProperties(devices[i], &vulkanThings->vulkan_info->deviceProperties);
                #if PRINT_INFO_MESSAGES 1
                    printf("Selected GPU : %s\n", vulkanThings->vulkan_info->deviceProperties.deviceName);
                #endif

					break;
				}
			}
		}
		free(queueFamilies);
	}
	free(devices);

	if (vulkanThings->physicalDevice == VK_NULL_HANDLE) {
		printf("failed to find a suitable GPU!");
       exit(1);
	}
}

void vk_createLogicalDevice(St_vulkanThings* vulkanThings)
{
    VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = vulkanThings->queues.graphicsQueueIndex;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(vulkanThings->physicalDevice, &deviceFeatures);

	uint32_t extensionPropertyCount;
	vkEnumerateDeviceExtensionProperties(vulkanThings->physicalDevice, NULL, &extensionPropertyCount, NULL);
	VkExtensionProperties* extensionProperties = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionPropertyCount);
	vkEnumerateDeviceExtensionProperties(vulkanThings->physicalDevice, NULL, &extensionPropertyCount, extensionProperties);
	
    int found = 0;
    for (uint32_t i = 0; i < extensionPropertyCount; i++) {
		if (strcmp(extensionProperties[i].extensionName, "VK_KHR_portability_subset") == 0) {
            found = 1;
            break;
		}
	}

    int extensionCount;
    if (found)
    {
        extensionCount = NUM_ENABLE_EXTENSIONS + 1;
    }
    else{
        extensionCount = NUM_ENABLE_EXTENSIONS;
    }

    const char** enabledExtensions = malloc(sizeof(const char*) * extensionCount);
    memcpy(enabledExtensions, ENABLE_EXTENSIONS, NUM_ENABLE_EXTENSIONS * sizeof(const char*));
    if (found) {
        enabledExtensions[NUM_ENABLE_EXTENSIONS] = "VK_KHR_portability_subset";
    }

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = extensionCount;
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions;

	CHECK_RESULT_VK(vkCreateDevice(vulkanThings->physicalDevice, &deviceCreateInfo, NULL, &vulkanThings->logicalDevice))

	vkGetDeviceQueue(vulkanThings->logicalDevice, vulkanThings->queues.graphicsQueueIndex, 0, &vulkanThings->queues.graphicsQueue);

    free(enabledExtensions);
}

void vk_createRenderPass(St_vulkanThings* vulkanThings){
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = vulkanThings->swapChainData->surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 1;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	VkAttachmentDescription attachments[] = { colorAttachment };

	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription);
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = (VkSubpassDescription[]){ subpass };

	CHECK_RESULT_VK(vkCreateRenderPass(vulkanThings->logicalDevice, &renderPassInfo, NULL, &vulkanThings->renderPass))
}