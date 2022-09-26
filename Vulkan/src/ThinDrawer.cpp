#include <ThinDrawer.h>
#include <SwapChain.h>
#include <Shader.h>
#include <vector>
#include <array>

ThinDrawer::ThinDrawer()
{
    initBase();
}

void ThinDrawer::initBase()
{
    width = 1200;
    height = 720;
    if (!glfwInit())
    {
        printf("!glfwInit()\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, "Start Window", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(1);
    }

    int x1, y1, x2, y2;
    glfwGetWindowSize(window, &x1, &y1);
    glfwGetFramebufferSize(window, &x2, &y2);

    dpiScaling = std::round(x2 / x1);

    width *= dpiScaling;
    height *= dpiScaling;

    vulkanInfo = new s_vulkanInfo;

    swapChain = new SwapChain(this);

    createInstance();

    CHECK_RESULT_VK(glfwCreateWindowSurface(instance, window, NULL, &swapChain->surface))

    selectPhysicalDevice();
    createLogicalDevice();

    setSamples();

    swapChain->createSwapChain(window);

    createRenderPass();
    swapChain->createFrameBuffers();

    createCommands();

    drawCommandBuffers.resize(swapChain->imageCount);
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = { };
    cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocateInfo.commandBufferCount = drawCommandBuffers.size();
    cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocateInfo.commandPool = frames->commandPool;
    CHECK_RESULT_VK(vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, drawCommandBuffers.data()))

    createSyncThings();
    prepareVertices();
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    setupDescriptorSet();
    buildCommandBuffers();
}

void ThinDrawer::renderLoop()
{
    s_frameData currentFrame = frames[frameNumber % NUM_FRAMES];

    CHECK_RESULT_VK(vkWaitForFences(logicalDevice, 1, &currentFrame.renderFence, true, UINT64_MAX))
    CHECK_RESULT_VK(vkResetFences(logicalDevice, 1, &currentFrame.renderFence))
    CHECK_RESULT_VK(vkResetCommandBuffer(currentFrame.commandBuffer, 0))

    vkAcquireNextImageKHR(logicalDevice, swapChain->swapChain, UINT64_MAX,
                          currentFrame.presentSemaphore, NULL, &lastSwapChainImageIndex);

    VkSubmitInfo submit = { };
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = NULL;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &currentFrame.presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &currentFrame.renderSemaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &drawCommandBuffers[frameNumber % drawCommandBuffers.size()];

    CHECK_RESULT_VK(vkQueueSubmit(queues.graphicsQueue, 1, &submit, currentFrame.renderFence))
    VkPresentInfoKHR presentInfo = { };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.pSwapchains = &swapChain->swapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &currentFrame.renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &lastSwapChainImageIndex;

    vkQueuePresentKHR(queues.graphicsQueue, &presentInfo);

    frameNumber++;
}

void ThinDrawer::createInstance()
{
    VkApplicationInfo appInfo = { };
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = "Vulkan2D";

    VkInstanceCreateInfo instanceInfo = { };
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    uint32_t extensionCount;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

#ifdef __APPLE__
    instanceInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    extensionCount = glfwExtensionCount + 2;
    const char** requiredExtensions = (const char**)malloc(sizeof(const char*) * extensionCount);
    requiredExtensions[glfwExtensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    requiredExtensions[glfwExtensionCount + 1] = "VK_KHR_get_physical_device_properties2";
#else
    extensionCount = glfwExtensionCount;
    const char** requiredExtensions = malloc(sizeof(char*) * glfwExtensionCount);
#endif // __APPLE__

    memcpy(requiredExtensions, glfwExtensions, sizeof(const char*) * glfwExtensionCount);

    instanceInfo.enabledExtensionCount = extensionCount;
    instanceInfo.ppEnabledExtensionNames = requiredExtensions;

    CHECK_RESULT_VK(vkCreateInstance(&instanceInfo, nullptr, &instance))

    free(requiredExtensions);
}

void ThinDrawer::createCommands()
{
    VkCommandPoolCreateInfo commandCreateInfo = { };
    commandCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandCreateInfo.queueFamilyIndex = queues.graphicsQueueFamilyIndex;
    CHECK_RESULT_VK(vkCreateCommandPool(logicalDevice, &commandCreateInfo, nullptr, &uploadPool))
    CHECK_RESULT_VK(vkCreateCommandPool(logicalDevice, &commandCreateInfo, nullptr, &frames->commandPool))

    commandCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (int i = 0; i < NUM_FRAMES; i++)
    {
        CHECK_RESULT_VK(vkCreateCommandPool(logicalDevice, &commandCreateInfo, nullptr, &frames[i].commandPool))

        VkCommandBufferAllocateInfo cmdAllocInfo = { };
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.commandPool = frames[i].commandPool;
        cmdAllocInfo.commandBufferCount = 1;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        CHECK_RESULT_VK(vkAllocateCommandBuffers(logicalDevice, &cmdAllocInfo, &frames[i].commandBuffer))
    }
}

void ThinDrawer::createSyncThings()
{
    VkFenceCreateInfo fenceCreateInfo = { };
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo = { };
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (int i = 0; i < NUM_FRAMES; i++)
    {
        CHECK_RESULT_VK(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &frames[i].renderFence))

        CHECK_RESULT_VK(vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore))
        CHECK_RESULT_VK(vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore))
    }

    fenceCreateInfo.flags = 0;
    CHECK_RESULT_VK(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &uploadFence))
}


void ThinDrawer::createRenderPass()
{
    std::vector<VkAttachmentDescription> attachments;

    VkAttachmentDescription colorAttachment = { };

    colorAttachment.format = swapChain->surfaceFormat.format;
    colorAttachment.samples = samples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (samples > VK_SAMPLE_COUNT_1_BIT)
    {
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    else
    {
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    attachments.push_back(colorAttachment);

    VkAttachmentReference colorReference = { };
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (samples > VK_SAMPLE_COUNT_1_BIT)
    {
        VkAttachmentDescription colorAttachmentResolve = { };

        colorAttachmentResolve.format = swapChain->surfaceFormat.format;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachments.push_back(colorAttachmentResolve);
    }

    VkAttachmentReference color_attachment_resolve_ref = { };
    color_attachment_resolve_ref.attachment = 1;
    color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = { };
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    if (samples > VK_SAMPLE_COUNT_1_BIT)
    {
        subpassDescription.pResolveAttachments = &color_attachment_resolve_ref;
    }

    VkRenderPassCreateInfo renderPassInfo = { };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = 0;

    CHECK_RESULT_VK(vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass))
}

void ThinDrawer::createLogicalDevice()
{
    VkDeviceQueueCreateInfo queueCreateInfo = { };
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queues.graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = { };
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    uint32_t extensionPropertyCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionPropertyCount, nullptr);
    VkExtensionProperties* extensionProperties = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionPropertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionPropertyCount, extensionProperties);

    int found = 0;
    for (uint32_t i = 0; i < extensionPropertyCount; i++)
    {
        if (strcmp(extensionProperties[i].extensionName, "VK_KHR_portability_subset") == 0)
        {
            found = 1;
            break;
        }
    }

    int extensionCount;
    if (found)
    {
        extensionCount = NUM_ENABLE_EXTENSIONS + 1;
    }
    else
    {
        extensionCount = NUM_ENABLE_EXTENSIONS;
    }

    const char** enabledExtensions = (const char**)malloc(sizeof(const char*) * extensionCount);
    memcpy(enabledExtensions, ENABLE_EXTENSIONS, NUM_ENABLE_EXTENSIONS * sizeof(const char*));
    if (found)
    {
        enabledExtensions[NUM_ENABLE_EXTENSIONS] = "VK_KHR_portability_subset";
    }

    VkDeviceCreateInfo deviceCreateInfo = { };
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = extensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions;

    CHECK_RESULT_VK(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &logicalDevice))

    vkGetDeviceQueue(logicalDevice, queues.graphicsQueueFamilyIndex, 0, &queues.graphicsQueue);

    free(enabledExtensions);
}

bool ThinDrawer::isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR* surfaceFormats;
    VkPresentModeKHR* presentModes;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    int formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, (uint32_t*)&formatCount, nullptr);
    if (formatCount != 0)
    {
        surfaceFormats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, (uint32_t*)&formatCount, surfaceFormats);
    }

    int presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, (uint32_t*)&presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        presentModes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, (uint32_t*)&presentModeCount, presentModes);
    }
    if (formatCount != 0)
    {
        free(surfaceFormats);
    }
    if (presentModeCount != 0)
    {
        free(presentModes);
    }

    return presentModeCount != 0 && formatCount != 0;
}

void ThinDrawer::selectPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        printf("failed to find GPUs with Vulkan support!");
        exit(1);
    }
    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * deviceCount);

    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    physicalDevice = VK_NULL_HANDLE;

    for (uint32_t i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, nullptr);
        VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilies);

        if (isDeviceSuitable(devices[i], swapChain->surface))
        {
            for (uint32_t j = 0; j < queueFamilyCount; j++)
            {
                VkBool32 presentSupported;
                vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, swapChain->surface, &presentSupported);
                if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupported)
                {
                    physicalDevice = devices[i];
                    queues.graphicsQueueFamilyIndex = j;
                    vkGetPhysicalDeviceProperties(devices[i], &vulkanInfo->deviceProperties);
                    vkGetPhysicalDeviceMemoryProperties(devices[i], &vulkanInfo->deviceMemoryProperties);
#if PRINT_INFO_MESSAGES 1
                    printf("Selected GPU : %s\n", vulkanInfo->deviceProperties.deviceName);
#endif
                    break;
                }
            }
        }
        free(queueFamilies);
    }
    free(devices);

    if (physicalDevice == VK_NULL_HANDLE)
    {
        printf("failed to find a suitable GPU!");
        exit(1);
    }
}

void ThinDrawer::prepareVertices()
{
    std::vector<glm::vec2> vertexBuffer =
            {
                { +1.0f, +1.0f },
                { -1.0f, +1.0f },
                { +0.0f, -1.0f }
            };
    uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(glm::vec2);

    std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
    indices.count = static_cast<uint32_t>(indexBuffer.size());
    uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

    VkMemoryAllocateInfo memAlloc = { };
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    void *data;

    VkBufferCreateInfo vertexBufferInfo = { };
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = vertexBufferSize;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    s_stagingBuffers stagingBuffers;

    CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &vertexBufferInfo, nullptr, &stagingBuffers.vertices.buffer))
    vkGetBufferMemoryRequirements(logicalDevice, stagingBuffers.vertices.buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &stagingBuffers.vertices.memory))

    CHECK_RESULT_VK(vkMapMemory(logicalDevice, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data))
    memcpy(data, vertexBuffer.data(), vertexBufferSize);
    vkUnmapMemory(logicalDevice, stagingBuffers.vertices.memory);
    CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0))

    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &vertexBufferInfo, nullptr, &vertices.buffer))
    vkGetBufferMemoryRequirements(logicalDevice, vertices.buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &vertices.memory))
    CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, vertices.buffer, vertices.memory, 0))

    VkBufferCreateInfo bufferCreateInfo = { };
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = indexBufferSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffers.indices.buffer))
    vkGetBufferMemoryRequirements(logicalDevice, stagingBuffers.indices.buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &stagingBuffers.indices.memory))
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data))
    memcpy(data, indexBuffer.data(), indexBufferSize);
    vkUnmapMemory(logicalDevice, stagingBuffers.indices.memory);
    CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0))

    bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &indices.buffer))
    vkGetBufferMemoryRequirements(logicalDevice, indices.buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &indices.memory))
    CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, indices.buffer, indices.memory, 0))

    VkCommandBuffer copyCmd = getCommandBuffer(true);

    VkBufferCopy copyRegion = { };


    copyRegion.size = vertexBufferSize;
    vkCmdCopyBuffer(copyCmd, stagingBuffers.vertices.buffer, vertices.buffer, 1, &copyRegion);

    copyRegion.size = indexBufferSize;
    vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, indices.buffer,	1, &copyRegion);

    flushCommandBuffer(copyCmd);

    vkDestroyBuffer(logicalDevice, stagingBuffers.vertices.buffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBuffers.vertices.memory, nullptr);
    vkDestroyBuffer(logicalDevice, stagingBuffers.indices.buffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBuffers.indices.memory, nullptr);
}

VkCommandBuffer ThinDrawer::getCommandBuffer(bool begin)
{
    VkCommandBuffer cmdBuffer;

    VkCommandBufferAllocateInfo cmdBufAllocateInfo = { };
    cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocateInfo.commandPool = uploadPool;
    cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocateInfo.commandBufferCount = 1;

    CHECK_RESULT_VK(vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, &cmdBuffer))

    if (begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = { };
        cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        CHECK_RESULT_VK(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo))
    }

    return cmdBuffer;
}

void ThinDrawer::flushCommandBuffer(VkCommandBuffer commandBuffer)
{
    assert(commandBuffer != VK_NULL_HANDLE);

    CHECK_RESULT_VK(vkEndCommandBuffer(commandBuffer))

    VkSubmitInfo submitInfo = { };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkFenceCreateInfo fenceCreateInfo = { };
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VkFence fence;
    CHECK_RESULT_VK(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence))

    CHECK_RESULT_VK(vkQueueSubmit(queues.graphicsQueue, 1, &submitInfo, fence))
    CHECK_RESULT_VK(vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX))

    vkDestroyFence(logicalDevice, fence, nullptr);
    vkFreeCommandBuffers(logicalDevice, uploadPool, 1, &commandBuffer);
}

uint32_t ThinDrawer::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < vulkanInfo->deviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((vulkanInfo->deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        typeBits >>= 1;
    }

    printf("Could not find a suitable memory type!");
    exit(-1);
}

void ThinDrawer::prepareUniformBuffers()
{
    VkMemoryRequirements memReqs;

    VkBufferCreateInfo bufferInfo = { };
    VkMemoryAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = 0;
    allocInfo.memoryTypeIndex = 0;

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(s_uboVS);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &uniformBufferVS.buffer))
    vkGetBufferMemoryRequirements(logicalDevice, uniformBufferVS.buffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &(uniformBufferVS.memory)))
    CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, uniformBufferVS.buffer, uniformBufferVS.memory, 0))

    uniformBufferVS.descriptor.buffer = uniformBufferVS.buffer;
    uniformBufferVS.descriptor.offset = 0;
    uniformBufferVS.descriptor.range = sizeof(s_uboVS);

    s_uboVS uboVS;

    uboVS.orthoMatrix = glm::ortho(-4.0f, +4.0f, -2.25f, +2.25f, -100.0f, 100.0f);
    uboVS.modelMatrix = glm::mat4(1.0f);

    uint8_t *pData;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferVS.memory, 0, sizeof(uboVS), 0, (void **)&pData))
    memcpy(pData, &uboVS, sizeof(uboVS));

    vkUnmapMemory(logicalDevice, uniformBufferVS.memory);
}

void ThinDrawer::setupDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding layoutBinding = { };
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorLayout = { };
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = nullptr;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &layoutBinding;

    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout))

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = { };
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    CHECK_RESULT_VK(vkCreatePipelineLayout(logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout))
}

void ThinDrawer::preparePipelines()
{
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = { };
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { };
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = { };
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { };
    colorBlendAttachment.colorWriteMask = 0xF;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo colorBlendState = { };
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachment;
    colorBlendState.blendConstants[0] = 1.0f;
    colorBlendState.blendConstants[1] = 1.0f;
    colorBlendState.blendConstants[2] = 1.0f;
    colorBlendState.blendConstants[3] = 1.0f;

    VkPipelineViewportStateCreateInfo viewportState = { };
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    VkPipelineDynamicStateCreateInfo dynamicState = { };
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    VkPipelineMultisampleStateCreateInfo multisampleState = { };
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = samples;
    multisampleState.pSampleMask = nullptr;

    VkVertexInputBindingDescription vertexInputBinding = { };
    vertexInputBinding.binding = 0;
    vertexInputBinding.stride = sizeof(glm::vec2);
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexInputAttribute;
    vertexInputAttribute.binding = 0;
    vertexInputAttribute.location = 0;
    vertexInputAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttribute.offset = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputState = { };
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
    vertexInputState.vertexAttributeDescriptionCount = 1;
    vertexInputState.pVertexAttributeDescriptions = &vertexInputAttribute;

    std::vector<std::string> fileNames =	{
            std::string("../assets/shaders/VulkanTriangle/vertex_shader.vert.spv"),
            std::string("../assets/shaders/VulkanTriangle/fragment_shader.frag.spv")
    };

    Shader shader = Shader(logicalDevice, fileNames);

    pipelineCreateInfo.stageCount = shader.shaderStages.size();
    pipelineCreateInfo.pStages = shader.shaderStages.data();

    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = VK_NULL_HANDLE;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    CHECK_RESULT_VK(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline))
}

void ThinDrawer::setupDescriptorPool()
{
    VkDescriptorPoolSize typeCounts[1];
    typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    typeCounts[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolInfo = { };
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext = nullptr;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = typeCounts;
    descriptorPoolInfo.maxSets = 1;

    CHECK_RESULT_VK(vkCreateDescriptorPool(logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool))
}

void ThinDrawer::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    CHECK_RESULT_VK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet))

    VkWriteDescriptorSet writeDescriptorSet = { };

    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &uniformBufferVS.descriptor;
    writeDescriptorSet.dstBinding = 0;

    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void ThinDrawer::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = { };
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = nullptr;

    VkClearValue clearValues[1];
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkRenderPassBeginInfo renderPassBeginInfo = { };
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCommandBuffers.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = swapChain->frameBuffers[i];

        CHECK_RESULT_VK(vkBeginCommandBuffer(drawCommandBuffers[i], &cmdBufInfo))

        vkCmdBeginRenderPass(drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = { };
        viewport.height = (float)height;
        viewport.width = (float)width;
        viewport.minDepth = (float) 0.0f;
        viewport.maxDepth = (float) 1.0f;
        vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = { };
        scissor.extent.width = width;
        scissor.extent.height = height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(drawCommandBuffers[i], 0, 1, &scissor);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
        vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(drawCommandBuffers[i], 0, 1, &vertices.buffer, offsets);
        vkCmdBindIndexBuffer(drawCommandBuffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(drawCommandBuffers[i], indices.count, 1, 0, 0, 1);
        vkCmdEndRenderPass(drawCommandBuffers[i]);
        CHECK_RESULT_VK(vkEndCommandBuffer(drawCommandBuffers[i]))
    }
}

void ThinDrawer::createImage(uint32_t p_width, uint32_t p_height, uint32_t mipLevels,
                             VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                             VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo = { };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = p_width;
    imageInfo.extent.height = p_height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CHECK_RESULT_VK(vkCreateImage(logicalDevice, &imageInfo, nullptr, &image))

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory))

    CHECK_RESULT_VK(vkBindImageMemory(logicalDevice, image, imageMemory, 0))
}

uint32_t ThinDrawer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    printf("failed to find suitable memory type!");
    exit(-1);
}

void ThinDrawer::setSamples()
{

    VkSampleCountFlags counts = vulkanInfo->deviceProperties.limits.framebufferColorSampleCounts &
                                vulkanInfo->deviceProperties.limits.framebufferDepthSampleCounts;

    for (uint32_t flags = desiredSamples; flags >= VK_SAMPLE_COUNT_1_BIT; flags >>= 1)
    {
        if (flags & counts)
        {
            samples = (VkSampleCountFlagBits)flags;
            break;
        }
    }

    uint32_t maxSampleCount = pow(2, (uint32_t)log2(counts));
    printf("Desired sample count: %d\nApplied sample count: %d\nMax sample count: %d\n\n", desiredSamples, samples, maxSampleCount);
}