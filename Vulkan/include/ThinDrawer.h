#include <stdlib.h>
#include <definitions.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

#define NUM_FRAMES 2
#define PRINT_INFO_MESSAGES 1

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
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
} s_vulkanInfo;

typedef struct
{
    VkDeviceMemory memory;
    VkBuffer buffer;
    uint32_t count;
} s_buffers;

typedef struct
{
    VkDeviceMemory memory;
    VkBuffer buffer;
    VkDescriptorBufferInfo descriptor;
} s_uniformBuffer;

typedef struct
{
    VkDeviceMemory memory;
    VkBuffer buffer;
} s_stagingBuffer;

typedef struct
{
    s_stagingBuffer vertices;
    s_stagingBuffer indices;
} s_stagingBuffers;

typedef struct
{
    glm::mat4 orthoMatrix;
    glm::mat4 modelMatrix;
} s_uboVS;

typedef struct
{
    glm::vec4 color;
} s_uboFSColor;

typedef struct
{
    VkSampler sampler;
    VkImage image;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    VkDescriptorSet set;
    VkImageView view;
    uint32_t width, height;
    uint32_t mipLevels;
} s_texture;

typedef struct
{
    glm::vec2 pos;
    glm::vec2 uv;
} s_vertex;

typedef struct
{
    glm::vec2 pos;
} s_basicVertex;

class SwapChain;

class ThinDrawer
{
public:

    ThinDrawer();

    uint32_t dpiScaling = 1;
    uint32_t frameNumber = 0;
    uint32_t lastSwapChainImageIndex;
    VkSampleCountFlagBits samples;
    VkSampleCountFlagBits desiredSamples = VK_SAMPLE_COUNT_4_BIT;

    s_vulkanInfo* vulkanInfo;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    s_queueData queues;
    SwapChain* swapChain;
    VkRenderPass renderPass;
    VkCommandPool uploadPool;
    VkFence uploadFence;
    GLFWwindow* window;
    uint32_t width, height;
    s_frameData frames[NUM_FRAMES];
    std::vector<VkCommandBuffer> drawCommandBuffers;

    VkDescriptorPool descriptorPool;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout textureSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkDescriptorSet descriptorSet;

    VkDescriptorSetLayout dc_descriptorSetLayout;
    VkPipelineLayout dc_pipelineLayout;
    VkPipeline dc_pipeline;
    VkDescriptorSet dc_descriptorSet;

    s_buffers vertices;
    s_buffers indices;
    s_uniformBuffer uniformBufferVS;

    s_buffers vertices2;
    s_uniformBuffer dc_uniformBufferVS;
    s_uniformBuffer dc_uniformBufferFS;

    s_texture singleTexture;

    void createWindow();
    void initBase();
    void initExtra();

    void createInstance();
    void createCommands();
    void createSyncThings();
    void createRenderPass();
    void createLogicalDevice();
    void selectPhysicalDevice();
    void prepareVertices();
    void prepareUniformBuffers();
    void setupDescriptorSetLayout();
    void preparePipelines();
    void setupDescriptorPool();
    void setupDescriptorSet();
    void buildCommandBuffers();

    void renderLoop();

    VkCommandBuffer getCommandBuffer(bool begin);
    void flushCommandBuffer(VkCommandBuffer commandBuffer);
    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);
    void setSamples();

    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin);
    void loadTexture(char* fileName, s_texture* texture);
    void updateImageDescriptors(s_texture* tex);
    void createImage(uint32_t width, uint32_t p_height, uint32_t mipLevels,
                     VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    static bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};