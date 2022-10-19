#include <ThinDrawer.h>
#include <SwapChain.h>
#include <Shader.h>
#include <vkInit.h>
#include <vector>
#include <array>

ThinDrawer::ThinDrawer()
{
    initBase();
    initExtra();
}

void ThinDrawer::surfaceRecreate()
{
    vkDeviceWaitIdle(logicalDevice);

    vkDestroyPipeline(logicalDevice, pipeline, VK_NULL_HANDLE);
    vkDestroyPipeline(logicalDevice, dc_pipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(logicalDevice, dc_pipelineLayout, VK_NULL_HANDLE);
    vkDestroyRenderPass(logicalDevice, renderPass, VK_NULL_HANDLE);

    vkDestroyDescriptorPool(logicalDevice, descriptorPool, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(logicalDevice, dc_descriptorSetLayout, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(logicalDevice, textureSetLayout, VK_NULL_HANDLE);

    vkFreeMemory(logicalDevice, vertices.memory, VK_NULL_HANDLE);
    vkFreeMemory(logicalDevice, vertices2.memory, VK_NULL_HANDLE);
    vkFreeMemory(logicalDevice, indices.memory, VK_NULL_HANDLE);

    vkDestroyBuffer(logicalDevice, vertices.buffer, VK_NULL_HANDLE);
    vkDestroyBuffer(logicalDevice, vertices2.buffer, VK_NULL_HANDLE);
    vkDestroyBuffer(logicalDevice, indices.buffer, VK_NULL_HANDLE);

    vkFreeMemory(logicalDevice, uniformBufferVS.memory, VK_NULL_HANDLE);
    vkFreeMemory(logicalDevice, dc_uniformBufferVS.memory, VK_NULL_HANDLE);
    vkFreeMemory(logicalDevice, dc_uniformBufferFS.memory, VK_NULL_HANDLE);

    vkDestroyBuffer(logicalDevice, uniformBufferVS.buffer, VK_NULL_HANDLE);
    vkDestroyBuffer(logicalDevice, dc_uniformBufferVS.buffer, VK_NULL_HANDLE);
    vkDestroyBuffer(logicalDevice, dc_uniformBufferFS.buffer, VK_NULL_HANDLE);

    vkDestroyFence(logicalDevice, uploadFence, VK_NULL_HANDLE);
    for (int i = 0; i < frames.size(); i++)
    {
        vkDestroyFence(logicalDevice, frames[i].renderFence, VK_NULL_HANDLE);

        vkDestroySemaphore(logicalDevice, frames[i].presentSemaphore, VK_NULL_HANDLE);
        vkDestroySemaphore(logicalDevice, frames[i].renderSemaphore, VK_NULL_HANDLE);

        vkDestroyCommandPool(logicalDevice, frames[i].commandPool, VK_NULL_HANDLE);
    }

    swapChain->destroy();
    swapChain->creationLoop();

    createSyncThings();
    createCommands();
    createRenderPass();
    swapChain->createFrameBuffers();

    prepareVertices();
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    for (auto iter = loadedTextures.begin(); iter != loadedTextures.end(); iter++)
    {
        s_texture* tex = *iter;
        updateImageDescriptors(tex);
    }
    setupDescriptorSet();
    buildCommandBuffers();

    vkDeviceWaitIdle(logicalDevice);
}

void ThinDrawer::initExtra()
{
    prepareVertices();
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();

    loadTexture((char*)"textures/chain.png", &singleTexture);
    setupDescriptorSet();

    buildCommandBuffers();
}

void ThinDrawer::renderLoop()
{
    s_frameData currentFrame = frames[frameNumber % swapChain->imageCount];

    CHECK_RESULT_VK(vkWaitForFences(logicalDevice, 1, &currentFrame.renderFence, true, UINT64_MAX));
    CHECK_RESULT_VK(vkResetFences(logicalDevice, 1, &currentFrame.renderFence));
    CHECK_RESULT_VK(vkResetCommandBuffer(currentFrame.commandBuffer, 0));

    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain->swapChain, UINT64_MAX,
                          currentFrame.presentSemaphore, VK_NULL_HANDLE, &lastSwapChainImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        surfaceRecreate();
        return;
    }

    VkSubmitInfo submit = { };
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &currentFrame.presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &currentFrame.renderSemaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &drawCommandBuffers[lastSwapChainImageIndex % drawCommandBuffers.size()];

    CHECK_RESULT_VK(vkQueueSubmit(queues.graphicsQueue, 1, &submit, currentFrame.renderFence));
    VkPresentInfoKHR presentInfo = { };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pSwapchains = &swapChain->swapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &currentFrame.renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &lastSwapChainImageIndex;

    result = vkQueuePresentKHR(queues.graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        surfaceRecreate();
    }

    frameNumber++;
}

void ThinDrawer::prepareVertices()
{
    std::vector<s_vertex> vertexBuffer =
            {
                { {+1.0f, +1.0f}, {+1.0f, +0.0f} },
                { {-1.0f, +1.0f}, {+0.0f, +0.0f} },
                { {-1.0f, -1.0f}, {+0.0f, +1.0f} },
                { {+1.0f, -1.0f}, {+1.0f, +1.0f} },
            };
    uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(s_vertex);

    std::vector<s_basicVertex> vertexBuffer2 =
            {
                    { {+1.0f, +1.0f} },
                    { {-1.0f, +1.0f} },
                    { {-1.0f, -1.0f} },
                    { {+1.0f, -1.0f} },
            };
    uint32_t vertexBufferSize2 = static_cast<uint32_t>(vertexBuffer2.size()) * sizeof(s_basicVertex);

    std::vector<uint32_t> indexBuffer = { 0, 1, 2, 2, 3, 0 };
    indices.count = static_cast<uint32_t>(indexBuffer.size());
    uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

    void *data;

    auto func = [&](void* bufferData, uint32_t dataSize, VkBufferUsageFlags flags,
            s_stagingBuffer& stagingBuffer, s_buffers& fillBuffer){
        VkMemoryAllocateInfo memAlloc = { };
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memReqs;

        VkBufferCreateInfo vertexBufferInfo = vkinit::bufferCreateInfo(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &vertexBufferInfo, VK_NULL_HANDLE, &stagingBuffer.buffer));
        vkGetBufferMemoryRequirements(logicalDevice, stagingBuffer.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &memAlloc, VK_NULL_HANDLE, &stagingBuffer.memory));

        CHECK_RESULT_VK(vkMapMemory(logicalDevice, stagingBuffer.memory, 0, memAlloc.allocationSize, 0, &data));
        memcpy(data, bufferData, dataSize);
        vkUnmapMemory(logicalDevice, stagingBuffer.memory);
        CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, stagingBuffer.buffer, stagingBuffer.memory, 0));

        vertexBufferInfo.usage = flags;
        CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &vertexBufferInfo, VK_NULL_HANDLE, &fillBuffer.buffer));
        vkGetBufferMemoryRequirements(logicalDevice, fillBuffer.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &memAlloc, VK_NULL_HANDLE, &fillBuffer.memory));
        CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, fillBuffer.buffer, fillBuffer.memory, 0));
    };

    s_stagingBuffer vertexBuffers;
    s_stagingBuffer vertexBuffers2;
    s_stagingBuffer indexBuffers;

    func(vertexBuffer.data(), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
         vertexBuffers, vertices);
    func(vertexBuffer2.data(), vertexBufferSize2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
         vertexBuffers2, vertices2);
    func(indexBuffer.data(), indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
         indexBuffers, indices);

    VkCommandBuffer copyCmd = getCommandBuffer(true);

    VkBufferCopy copyRegion = { };

    copyRegion.size = vertexBufferSize;
    vkCmdCopyBuffer(copyCmd, vertexBuffers.buffer, vertices.buffer, 1, &copyRegion);

    copyRegion.size = vertexBufferSize2;
    vkCmdCopyBuffer(copyCmd, vertexBuffers2.buffer, vertices2.buffer, 1, &copyRegion);

    copyRegion.size = indexBufferSize;
    vkCmdCopyBuffer(copyCmd, indexBuffers.buffer, indices.buffer,	1, &copyRegion);

    flushCommandBuffer(copyCmd);

    vkDestroyBuffer(logicalDevice, vertexBuffers.buffer, VK_NULL_HANDLE);
    vkFreeMemory(logicalDevice, vertexBuffers.memory, VK_NULL_HANDLE);
    vkDestroyBuffer(logicalDevice, vertexBuffers2.buffer, VK_NULL_HANDLE);
    vkFreeMemory(logicalDevice, vertexBuffers2.memory, VK_NULL_HANDLE);
    vkDestroyBuffer(logicalDevice, indexBuffers.buffer, VK_NULL_HANDLE);
    vkFreeMemory(logicalDevice, indexBuffers.memory, VK_NULL_HANDLE);
}

void ThinDrawer::uniformHelper(int size, s_uniformBuffer* uniformBuffer)
{
    VkBufferCreateInfo bufferInfo = vkinit::bufferCreateInfo(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &bufferInfo, VK_NULL_HANDLE, &uniformBuffer->buffer));

    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo allocInfo = vkinit::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(logicalDevice, uniformBuffer->buffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &allocInfo, VK_NULL_HANDLE, &uniformBuffer->memory));
    CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, uniformBuffer->buffer, uniformBuffer->memory, 0));

    uniformBuffer->descriptor.buffer = uniformBuffer->buffer;
    uniformBuffer->descriptor.offset = 0;
    uniformBuffer->descriptor.range = size;
}

void ThinDrawer::prepareUniformBuffers()
{
    uniformHelper(sizeof(s_uboVS), &uniformBufferVS);

    s_uboVS uboVS;

    uboVS.orthoMatrix = glm::ortho(-4.0f, +4.0f, -2.25f, +2.25f, -100.0f, 100.0f);
    uboVS.orthoMatrix[1][1] *= -1;
    uboVS.modelMatrix = glm::mat4(1.0f);
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::pi<float>() * 0.25f, glm::vec3(0, 0, 1));
    uboVS.modelMatrix = glm::scale(uboVS.modelMatrix, glm::vec3(2.5f, 1.0f, 1.0f));

    uint8_t *pData;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferVS.memory, 0, sizeof(s_uboVS), 0, (void**)&pData));
    memcpy(pData, &uboVS, sizeof(s_uboVS));
    vkUnmapMemory(logicalDevice, uniformBufferVS.memory);

    glm::vec4 vec[2] = { glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, -1.0f, -1.0f, -1.0f) };
    uniformHelper(sizeof(vec), &uniformBufferFS2);
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferFS2.memory, 0, sizeof(vec), 0, (void**)&pData));
    memcpy(pData, &vec, sizeof(vec));
    vkUnmapMemory(logicalDevice, uniformBufferFS2.memory);

    uniformHelper(sizeof(int), &uniformBufferFS);
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferFS.memory, 0, sizeof(int), 0, (void**)&pData));
    int val = sizeof(vec) / sizeof(glm::vec4);
    memcpy(pData, &val, sizeof(int));
    vkUnmapMemory(logicalDevice, uniformBufferFS.memory);

    // for debug circle

    uniformHelper(sizeof(s_uboVS), &dc_uniformBufferVS);

    uboVS.modelMatrix = glm::mat4(1.0f);
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, dc_uniformBufferVS.memory, 0, sizeof(uboVS), 0, (void**)&pData));
    memcpy(pData, &uboVS, sizeof(uboVS));

    vkUnmapMemory(logicalDevice, dc_uniformBufferVS.memory);

    uniformHelper(sizeof(s_uboFSColor), &dc_uniformBufferFS);

    s_uboFSColor uboFSColor;
    uboFSColor.color = glm::vec4(0.2f, 1.0f, 1.0f, 0.6f);

    CHECK_RESULT_VK(vkMapMemory(logicalDevice, dc_uniformBufferFS.memory, 0, sizeof(uboFSColor), 0, (void**)&pData));
    memcpy(pData, &uboFSColor, sizeof(uboFSColor));

    vkUnmapMemory(logicalDevice, dc_uniformBufferFS.memory);
}

void ThinDrawer::setupDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding layoutBinding =
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, 0);

    VkDescriptorSetLayoutBinding layoutBinding2 = 
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);

    VkDescriptorSetLayoutBinding layoutBinding3 =
        vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 2);

    VkDescriptorSetLayoutBinding bindings[] = { layoutBinding, layoutBinding2, layoutBinding3 };

    VkDescriptorSetLayoutCreateInfo descriptorLayout = vkinit::descriptorSetLayoutCreateInfo(bindings, sizeof(bindings) / sizeof(bindings[0]));
    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(logicalDevice, &descriptorLayout, VK_NULL_HANDLE, &descriptorSetLayout));

    VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 0);

    VkDescriptorSetLayoutCreateInfo setInfo = vkinit::descriptorSetLayoutCreateInfo(&textureBind, 1);
    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(logicalDevice, &setInfo, VK_NULL_HANDLE, &textureSetLayout));

    VkDescriptorSetLayout setLayouts[] = { descriptorSetLayout, textureSetLayout };

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vkinit::pipelineLayoutCreateInfo(
            setLayouts, sizeof(setLayouts) / sizeof(setLayouts[0]));
    CHECK_RESULT_VK(vkCreatePipelineLayout(logicalDevice, &pPipelineLayoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout));

    //for  debug circle

    layoutBinding2 = vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);

    VkDescriptorSetLayoutBinding bindings1[] = { layoutBinding, layoutBinding2 };

    descriptorLayout = vkinit::descriptorSetLayoutCreateInfo(bindings1, sizeof(bindings1) / sizeof(bindings1[0]));
    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(logicalDevice, &descriptorLayout, VK_NULL_HANDLE, &dc_descriptorSetLayout));

    VkDescriptorSetLayout setLayouts2[] = { dc_descriptorSetLayout };
    pPipelineLayoutCreateInfo = vkinit::pipelineLayoutCreateInfo(setLayouts2, sizeof(setLayouts2) / sizeof(setLayouts2[0]));
    CHECK_RESULT_VK(vkCreatePipelineLayout(logicalDevice, &pPipelineLayoutCreateInfo, VK_NULL_HANDLE, &dc_pipelineLayout));
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

    VkViewport viewport = { };
    viewport.width = swapChain->extent.width;
    viewport.height = swapChain->extent.height;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = swapChain->extent;

    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineMultisampleStateCreateInfo multisampleState = { };
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = samples;
    multisampleState.sampleShadingEnable = VK_TRUE;
    multisampleState.minSampleShading = 1.0f;

    VkVertexInputBindingDescription vertexInputBinding = { };
    vertexInputBinding.binding = 0;
    vertexInputBinding.stride = sizeof(s_vertex);
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexInputAttributeFor0;
    vertexInputAttributeFor0.binding = 0;
    vertexInputAttributeFor0.location = 0;
    vertexInputAttributeFor0.format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeFor0.offset = 0;

    VkVertexInputAttributeDescription vertexInputAttributeFor1;
    vertexInputAttributeFor1.binding = 0;
    vertexInputAttributeFor1.location = 1;
    vertexInputAttributeFor1.format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeFor1.offset = offsetof(s_vertex, uv);

    VkVertexInputAttributeDescription attributes[] = { vertexInputAttributeFor0, vertexInputAttributeFor1 };

    VkPipelineVertexInputStateCreateInfo vertexInputState = { };
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
    vertexInputState.vertexAttributeDescriptionCount = sizeof(attributes) / sizeof(attributes[0]);
    vertexInputState.pVertexAttributeDescriptions = attributes;

    std::vector<std::string> fileNames =	{
            std::string("shaders/VulkanTriangle/vertex_shader.vert.spv"),
            std::string("shaders/VulkanTriangle/fragment_shader.frag.spv")
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
    pipelineCreateInfo.pDynamicState = VK_NULL_HANDLE;

    CHECK_RESULT_VK(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VK_NULL_HANDLE, &pipeline));

    // for debug circle

    pipelineCreateInfo.layout = dc_pipelineLayout;

    vertexInputBinding.stride = sizeof(s_basicVertex);

    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.vertexAttributeDescriptionCount = 1;
    vertexInputState.pVertexAttributeDescriptions = &vertexInputAttributeFor0;

    fileNames =	{
            std::string("../assets/shaders/VulkanDebugCircle/vertex_shader.vert.spv"),
            std::string("../assets/shaders/VulkanDebugCircle/fragment_shader.frag.spv")
    };

    Shader shader2 = Shader(logicalDevice, fileNames);

    pipelineCreateInfo.stageCount = shader2.shaderStages.size();
    pipelineCreateInfo.pStages = shader2.shaderStages.data();

    CHECK_RESULT_VK(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VK_NULL_HANDLE, &dc_pipeline));
}

void ThinDrawer::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> sizes =
    {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = { };
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = sizes.size();
    descriptorPoolInfo.pPoolSizes = sizes.data();
    descriptorPoolInfo.maxSets = 10;

    CHECK_RESULT_VK(vkCreateDescriptorPool(logicalDevice, &descriptorPoolInfo, VK_NULL_HANDLE, &descriptorPool));
}

void ThinDrawer::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo = vkinit::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

    CHECK_RESULT_VK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet));

    VkWriteDescriptorSet writeDescriptorSet = { };

    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &uniformBufferVS.descriptor;
    writeDescriptorSet.dstBinding = 0;

    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);

    writeDescriptorSet.pBufferInfo = &uniformBufferFS.descriptor;
    writeDescriptorSet.dstBinding = 1;

    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);

    writeDescriptorSet.pBufferInfo = &uniformBufferFS2.descriptor;
    writeDescriptorSet.dstBinding = 2;

    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);

    updateImageDescriptors(&singleTexture);

    // for debug circle

    allocInfo = vkinit::descriptorSetAllocateInfo(descriptorPool, &dc_descriptorSetLayout, 1);

    CHECK_RESULT_VK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &dc_descriptorSet));

    writeDescriptorSet.dstSet = dc_descriptorSet;
    writeDescriptorSet.pBufferInfo = &dc_uniformBufferVS.descriptor;
    writeDescriptorSet.dstBinding = 0;
    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);

    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.pBufferInfo = &dc_uniformBufferFS.descriptor;
    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);
}

void ThinDrawer::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = vkinit::commandBufferBeginInfo();

    VkClearValue clearValue;
    clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };
    VkRenderPassBeginInfo renderPassBeginInfo = vkinit::renderPassBeginInfo(renderPass, swapChain->extent.width, swapChain->extent.height, &clearValue);

    for (int32_t i = 0; i < drawCommandBuffers.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = swapChain->frameBuffers[i];

        CHECK_RESULT_VK(vkBeginCommandBuffer(drawCommandBuffers[i], &cmdBufInfo));

        vkCmdBeginRenderPass(drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = { };
        viewport.height = (float)swapChain->extent.height;
        viewport.width = (float)swapChain->extent.width;
        viewport.minDepth = (float)0.0f;
        viewport.maxDepth = (float)1.0f;
        vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);

        vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout, 0, 1, &descriptorSet, 0, VK_NULL_HANDLE);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout, 1, 1, &singleTexture.set, 0, VK_NULL_HANDLE);

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(drawCommandBuffers[i], 0, 1, &vertices.buffer, offsets);
        vkCmdBindIndexBuffer(drawCommandBuffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(drawCommandBuffers[i], indices.count, 1, 0, 0, 1);

        // for debug circle

        vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, dc_pipeline);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                dc_pipelineLayout, 0, 1, &dc_descriptorSet, 0, VK_NULL_HANDLE);

        vkCmdBindVertexBuffers(drawCommandBuffers[i], 0, 1, &vertices2.buffer, offsets);
        vkCmdBindIndexBuffer(drawCommandBuffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(drawCommandBuffers[i], indices.count, 1, 0, 0, 1);

        vkCmdEndRenderPass(drawCommandBuffers[i]);
        CHECK_RESULT_VK(vkEndCommandBuffer(drawCommandBuffers[i]));
    }
}