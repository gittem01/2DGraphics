#include <stdlib.h>
#include <definitions.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

#define NUM_FRAMES 2
#define PRINT_INFO_MESSAGES

static const char* ENABLE_EXTENSIONS[] =
{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static const int NUM_ENABLE_EXTENSIONS = sizeof(ENABLE_EXTENSIONS) / sizeof(ENABLE_EXTENSIONS[0]);

typedef struct
{
    VkShaderModule shaderModules[2];
    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2];
} s_shader;

typedef struct
{
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkFence renderFence;
    VkSemaphore presentSemaphore;
    VkSemaphore renderSemaphore;
} s_frameData;

typedef struct
{
    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamilyIndex;
} s_queueData;

typedef struct
{
    VkPhysicalDeviceProperties deviceProperties;
} s_vulkanInfo;

class SwapChain;

class ThinDrawer
{
public:

    ThinDrawer();

    s_vulkanInfo* vulkan_info;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    s_queueData queues;
    SwapChain* swapChain;
    VkRenderPass renderPass;
    VkCommandPool uploadPool;
    VkFence uploadFence;
    GLFWwindow* window;
    s_frameData frames[NUM_FRAMES];

    void initBase();

    void createInstance();
    void createCommands();
    void createSyncThings();
    void createRenderPass();
    void createLogicalDevice();
    void selectPhysicalDevice();

    static bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static VkCommandPoolCreateInfo fillCommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);
};