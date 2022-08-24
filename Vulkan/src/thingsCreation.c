#include <thingsCreation.h>

const char* ENABLE_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const int NUM_ENABLE_EXTENSIONS = sizeof(ENABLE_EXTENSIONS) / sizeof(ENABLE_EXTENSIONS[0]);

void vk_createInstance(vulkanThings* vulkan_things)
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

    CHECK_RESULT_VK(vkCreateInstance(&instanceInfo, NULL, &vulkan_things->instance))

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

void vk_selectPhysicalDevice(vulkanThings* vulkan_things)
{
    uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vulkan_things->instance, &deviceCount, NULL);
	if (deviceCount == 0) {
		printf("failed to find GPUs with Vulkan support!");
        exit(1);
	}
	VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);

	vkEnumeratePhysicalDevices(vulkan_things->instance, &deviceCount, devices);

	vulkan_things->physicalDevice = VK_NULL_HANDLE;

	for (uint32_t i = 0; i < deviceCount; i++) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, NULL);
		VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilies);

		if (isDeviceSuitable(devices[i], vulkan_things->surface)) {
			for (uint32_t j = 0; j < queueFamilyCount; j++) {
				VkBool32 presentSupported;
				vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, vulkan_things->surface, &presentSupported);
				if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupported) {
					vulkan_things->physicalDevice = devices[i];
					vulkan_things->queues.graphicsQueueIndex = j;

	                vkGetPhysicalDeviceProperties(devices[i], &vulkan_things->vulkan_info->deviceProperties);
                #if PRINT_INFO_MESSAGES 1
                    printf("Selected GPU : %s\n", vulkan_things->vulkan_info->deviceProperties.deviceName);
                #endif

					break;
				}
			}
		}
		free(queueFamilies);
	}
	free(devices);

//	if (vulkan_things->physicalDevice == VK_NULL_HANDLE) {
//		printf("failed to find a suitable GPU!");
//        exit(1);
//	}
}

void vk_createLogicalDevice(vulkanThings* vulkan_things)
{
    VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = vulkan_things->queues.graphicsQueueIndex;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(vulkan_things->physicalDevice, &deviceFeatures);

	uint32_t extensionPropertyCount;
	vkEnumerateDeviceExtensionProperties(vulkan_things->physicalDevice, NULL, &extensionPropertyCount, NULL);
	VkExtensionProperties* extensionProperties = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionPropertyCount);
	vkEnumerateDeviceExtensionProperties(vulkan_things->physicalDevice, NULL, &extensionPropertyCount, extensionProperties);
	
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

	CHECK_RESULT_VK(vkCreateDevice(vulkan_things->physicalDevice, &deviceCreateInfo, NULL, &vulkan_things->logicalDevice))

	vkGetDeviceQueue(vulkan_things->logicalDevice, vulkan_things->queues.graphicsQueueIndex, 0, &vulkan_things->queues.graphicsQueue);

    free(enabledExtensions);
}