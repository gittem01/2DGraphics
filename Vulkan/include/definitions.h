const int errorValues[]
=
{
    0,
    1,
    2,
    3,
    4,
    5,
    -1,
    -2,
    -3,
    -4,
    -5,
    -6,
    -7,
    -8,
    -9,
    -10,
    -11,
    -12,
    -13,
    -1000069000,
    -1000072003,
    -1000161000,
    -1000257000,
    -1000000000,
    -1000000001,
    1000001003,
    -1000001004,
    -1000003001,
    -1000011001,
    -1000012000,
    -1000158000,
    -1000174001,
    -1000255000,
    1000268000,
    1000268001,
    1000268002,
    1000268003,
    1000297000,
    VK_ERROR_OUT_OF_POOL_MEMORY,
    VK_ERROR_INVALID_EXTERNAL_HANDLE,
    VK_ERROR_FRAGMENTATION,
    VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    VK_PIPELINE_COMPILE_REQUIRED_EXT,
    0x7FFFFFFF
};

const char *errorStrings[]
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
    "VK_ERROR_SURFACE_LOST_KHR",
    "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR",
    "VK_SUBOPTIMAL_KHR",
    "VK_ERROR_OUT_OF_DATE_KHR",
    "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR",
    "VK_ERROR_VALIDATION_FAILED_EXT",
    "VK_ERROR_INVALID_SHADER_NV",
    "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT",
    "VK_ERROR_NOT_PERMITTED_EXT",
    "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT",
    "VK_THREAD_IDLE_KHR",
    "VK_THREAD_DONE_KHR",
    "VK_OPERATION_DEFERRED_KHR",
    "VK_OPERATION_NOT_DEFERRED_KHR",
    "VK_PIPELINE_COMPILE_REQUIRED_EXT",
    "VK_ERROR_OUT_OF_POOL_MEMORY_KHR",
    "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR",
    "VK_ERROR_FRAGMENTATION_EXT",
    "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT",
    "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR",
    "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT",
    "VK_RESULT_MAX_ENUM",
};

const int numOfErrors = sizeof(errorValues) / sizeof(errorValues[0]);

int findErrorIndex(int error)
{
    for (int i = 0; i < sizeof(errorValues) / sizeof(int); i++)
    {
        if (error == errorValues[i])
        {
            return i;
        }
    }

    return errorValues[numOfErrors - 1];
}

void printVkError(int error)
{
    int errorIndex = findErrorIndex(error);

    if (errorIndex == errorValues[numOfErrors - 1])
    {
        printf("Unknown error value: %d\n", error);
    }
    else
    {
        printf("Vulkan error : %s\n", errorStrings[errorIndex]);
    }
}