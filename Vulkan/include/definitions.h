#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <math.h>

#define PRINT_INFO_MESSAGES

typedef struct
{
    VkQueue graphicsQueue;
    uint32_t graphicsQueueIndex;
} queueData;

typedef struct
{
    VkPhysicalDeviceProperties deviceProperties;
} vulkanInfo;

typedef struct
{
    VkSwapchainKHR swapChain;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D extent;
    
    VkImage* images;
    VkImageView* imageViews;
    VkFramebuffer* frameBuffers;
    int imageCount;
} swapChainData;

typedef struct
{
    vulkanInfo* vulkan_info;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    queueData queues;
    swapChainData* swapChainData;
    VkRenderPass renderPass;

} vulkanThings;

static VkResult ERROR_VALUES[]
=
{
    VK_SUCCESS,
    VK_NOT_READY,
    VK_TIMEOUT,
    VK_EVENT_SET,
    VK_EVENT_RESET,
    VK_INCOMPLETE,
    VK_ERROR_OUT_OF_HOST_MEMORY,
    VK_ERROR_OUT_OF_DEVICE_MEMORY,
    VK_ERROR_INITIALIZATION_FAILED,
    VK_ERROR_DEVICE_LOST,
    VK_ERROR_MEMORY_MAP_FAILED,
    VK_ERROR_LAYER_NOT_PRESENT,
    VK_ERROR_EXTENSION_NOT_PRESENT,
    VK_ERROR_FEATURE_NOT_PRESENT,
    VK_ERROR_INCOMPATIBLE_DRIVER,
    VK_ERROR_TOO_MANY_OBJECTS,
    VK_ERROR_FORMAT_NOT_SUPPORTED,
    VK_ERROR_FRAGMENTED_POOL,
    VK_ERROR_UNKNOWN,
    VK_ERROR_OUT_OF_POOL_MEMORY,
    VK_ERROR_INVALID_EXTERNAL_HANDLE,
    VK_ERROR_FRAGMENTATION,
    VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    VK_PIPELINE_COMPILE_REQUIRED,
    VK_ERROR_SURFACE_LOST_KHR,
    VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
    VK_SUBOPTIMAL_KHR,
    VK_ERROR_OUT_OF_DATE_KHR,
    VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
    VK_ERROR_VALIDATION_FAILED_EXT,
    VK_ERROR_INVALID_SHADER_NV,
    VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
    VK_ERROR_NOT_PERMITTED_KHR,
    VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
    VK_THREAD_IDLE_KHR,
    VK_THREAD_DONE_KHR,
    VK_OPERATION_DEFERRED_KHR,
    VK_OPERATION_NOT_DEFERRED_KHR,
    VK_ERROR_COMPRESSION_EXHAUSTED_EXT,
    VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
    VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
    VK_ERROR_FRAGMENTATION_EXT,
    VK_ERROR_NOT_PERMITTED_EXT,
    VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
    VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR,
    VK_PIPELINE_COMPILE_REQUIRED_EXT,
    VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT,
    VK_RESULT_MAX_ENUM,
};

static const char* ERROR_STRINGS[]
=
{
    "VK_SUCCESS",
    "VK_NOT_READY",
    "VK_TIMEOUT",
    "VK_EVENT_SET",
    "VK_EVENT_RESET",
    "VK_INCOMPLETE",
    "VK_ERROR_OUT_OF_HOST_MEMORY",
    "VK_ERROR_OUT_OF_DEVICE_MEMORY",
    "VK_ERROR_INITIALIZATION_FAILED",
    "VK_ERROR_DEVICE_LOST",
    "VK_ERROR_MEMORY_MAP_FAILED",
    "VK_ERROR_LAYER_NOT_PRESENT",
    "VK_ERROR_EXTENSION_NOT_PRESENT",
    "VK_ERROR_FEATURE_NOT_PRESENT",
    "VK_ERROR_INCOMPATIBLE_DRIVER",
    "VK_ERROR_TOO_MANY_OBJECTS",
    "VK_ERROR_FORMAT_NOT_SUPPORTED",
    "VK_ERROR_FRAGMENTED_POOL",
    "VK_ERROR_UNKNOWN",
    "VK_ERROR_OUT_OF_POOL_MEMORY",
    "VK_ERROR_INVALID_EXTERNAL_HANDLE",
    "VK_ERROR_FRAGMENTATION",
    "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS",
    "VK_PIPELINE_COMPILE_REQUIRED",
    "VK_ERROR_SURFACE_LOST_KHR",
    "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR",
    "VK_SUBOPTIMAL_KHR",
    "VK_ERROR_OUT_OF_DATE_KHR",
    "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR",
    "VK_ERROR_VALIDATION_FAILED_EXT",
    "VK_ERROR_INVALID_SHADER_NV",
    "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT",
    "VK_ERROR_NOT_PERMITTED_KHR",
    "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT",
    "VK_THREAD_IDLE_KHR",
    "VK_THREAD_DONE_KHR",
    "VK_OPERATION_DEFERRED_KHR",
    "VK_OPERATION_NOT_DEFERRED_KHR",
    "VK_ERROR_COMPRESSION_EXHAUSTED_EXT",
    "VK_ERROR_OUT_OF_POOL_MEMORY_KHR",
    "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR",
    "VK_ERROR_FRAGMENTATION_EXT",
    "VK_ERROR_NOT_PERMITTED_EXT",
    "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT",
    "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR",
    "VK_PIPELINE_COMPILE_REQUIRED_EXT",
    "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT",
    "VK_RESULT_MAX_ENUM",
};

static const int NUM_ERRORS = sizeof(ERROR_VALUES) / sizeof(ERROR_VALUES[0]);

static int findErrorIndex(VkResult error)
{
    for (int i = 0; i < NUM_ERRORS; i++)
    {
        if (error == ERROR_VALUES[i])
        {
            return i;
        }
    }

    return ERROR_VALUES[NUM_ERRORS - 1];
}

static void printVkError(VkResult error)
{
    int errorIndex = findErrorIndex(error);
    if (errorIndex == ERROR_VALUES[NUM_ERRORS - 1])
    {
        printf("Unknown error value: %d\n", error);
    }
    else
    {
        printf("Vulkan error : %s\n", ERROR_STRINGS[errorIndex]);
    }
}

#define CHECK_RESULT_VK(result)     \
{                                   \
    if ((result) != VK_SUCCESS)     \
    {                               \
        printVkError(result);       \
        exit(1);                    \
    }                               \
}

#endif // DEFINITIONS_H