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

    vkDestroyPipeline(logicalDevice, texturedShader->pipeline, VK_NULL_HANDLE);
    vkDestroyPipeline(logicalDevice, debugCircleShader->pipeline, VK_NULL_HANDLE);
    vkDestroyRenderPass(logicalDevice, renderPass, VK_NULL_HANDLE);
    swapChain->destroy();
    
    swapChain->creationLoop();
    createRenderPass();
    swapChain->createFrameBuffers();
    preparePipelines();
    buildCommandBuffers();

    vkDeviceWaitIdle(logicalDevice);
}

void ThinDrawer::initExtra()
{
    texturedShader = new VulkanTriangle(this);
    debugCircleShader = new DebugCircle(this);

    prepareVertices();
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();

    s_texture* tex = (s_texture*)malloc(sizeof(s_texture));
    loadTexture((char*)"textures/chain.png", tex);
    texturedShader->textureData.push_back(tex);

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

void ThinDrawer::bufferStage(void* bufferData, uint32_t dataSize, VkBufferUsageFlags flags, s_buffers* fillBuffer)
{
    VkMemoryAllocateInfo memAlloc = { };
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    VkBufferCreateInfo vertexBufferInfo = vkinit::bufferCreateInfo(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    s_stagingBuffer stagingBuffer;

    CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &vertexBufferInfo, VK_NULL_HANDLE, &stagingBuffer.buffer));
    vkGetBufferMemoryRequirements(logicalDevice, stagingBuffer.buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &memAlloc, VK_NULL_HANDLE, &stagingBuffer.memory));

    void* data;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, stagingBuffer.memory, 0, memAlloc.allocationSize, 0, &data));
    memcpy(data, bufferData, dataSize);
    vkUnmapMemory(logicalDevice, stagingBuffer.memory);
    CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, stagingBuffer.buffer, stagingBuffer.memory, 0));

    vertexBufferInfo.usage = flags;
    CHECK_RESULT_VK(vkCreateBuffer(logicalDevice, &vertexBufferInfo, VK_NULL_HANDLE, &fillBuffer->buffer));
    vkGetBufferMemoryRequirements(logicalDevice, fillBuffer->buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CHECK_RESULT_VK(vkAllocateMemory(logicalDevice, &memAlloc, VK_NULL_HANDLE, &fillBuffer->memory));
    CHECK_RESULT_VK(vkBindBufferMemory(logicalDevice, fillBuffer->buffer, fillBuffer->memory, 0));

    VkCommandBuffer copyCmd = getCommandBuffer(true);
    VkBufferCopy copyRegion = { };
    copyRegion.size = dataSize;
    vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, fillBuffer->buffer, 1, &copyRegion);
    flushCommandBuffer(copyCmd);

    vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, VK_NULL_HANDLE);
    vkFreeMemory(logicalDevice, stagingBuffer.memory, VK_NULL_HANDLE);
};

void ThinDrawer::prepareVertices()
{
    texturedShader->prepareVertexData();
    debugCircleShader->prepareVertexData();
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
    texturedShader->prepareUniforms();
    debugCircleShader->prepareUniforms();
}

void ThinDrawer::setupDescriptorSetLayout()
{
    texturedShader->setupDescriptorSetLayout();
    debugCircleShader->setupDescriptorSetLayout();
}

VkGraphicsPipelineCreateInfo* ThinDrawer::getPipelineInfoBase()
{
    VkGraphicsPipelineCreateInfo* pipelineCreateInfo = (VkGraphicsPipelineCreateInfo*)calloc(1, sizeof(VkGraphicsPipelineCreateInfo));
    pipelineCreateInfo->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo->renderPass = renderPass;

    VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState = (VkPipelineInputAssemblyStateCreateInfo*)calloc(1, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    inputAssemblyState->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo* rasterizationStateCreateInfo = (VkPipelineRasterizationStateCreateInfo*)calloc(1, sizeof(VkPipelineRasterizationStateCreateInfo));
    rasterizationStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo->polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo->cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCreateInfo->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo->depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo->rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo->depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo->lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState* colorBlendAttachment = (VkPipelineColorBlendAttachmentState*)calloc(1, sizeof(VkPipelineColorBlendAttachmentState));
    colorBlendAttachment->colorWriteMask = 0xF;
    colorBlendAttachment->blendEnable = VK_TRUE;
    colorBlendAttachment->alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo* colorBlendState = (VkPipelineColorBlendStateCreateInfo*)calloc(1, sizeof(VkPipelineColorBlendStateCreateInfo));
    colorBlendState->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState->attachmentCount = 1;
    colorBlendState->pAttachments = colorBlendAttachment;
    colorBlendState->blendConstants[0] = 1.0f;
    colorBlendState->blendConstants[1] = 1.0f;
    colorBlendState->blendConstants[2] = 1.0f;
    colorBlendState->blendConstants[3] = 1.0f;

    VkPipelineViewportStateCreateInfo* viewportState = (VkPipelineViewportStateCreateInfo*)calloc(1, sizeof(VkPipelineViewportStateCreateInfo));
    viewportState->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    VkViewport* viewport = (VkViewport*)calloc(1, sizeof(VkViewport));
    viewport->width = swapChain->extent.width;
    viewport->height = swapChain->extent.height;

    viewportState->viewportCount = 1;
    viewportState->pViewports = viewport;

    VkRect2D* scissor = (VkRect2D*)calloc(1, sizeof(VkRect2D));
    scissor->offset = { 0, 0 };
    scissor->extent = swapChain->extent;

    viewportState->scissorCount = 1;
    viewportState->pScissors = scissor;

    VkPipelineMultisampleStateCreateInfo* multisampleState = (VkPipelineMultisampleStateCreateInfo*)calloc(1, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisampleState->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState->rasterizationSamples = samples;
    multisampleState->sampleShadingEnable = VK_TRUE;
    multisampleState->minSampleShading = 1.0f;

    pipelineCreateInfo->pInputAssemblyState = inputAssemblyState;
    pipelineCreateInfo->pRasterizationState = rasterizationStateCreateInfo;
    pipelineCreateInfo->pColorBlendState = colorBlendState;
    pipelineCreateInfo->pMultisampleState = multisampleState;
    pipelineCreateInfo->pViewportState = viewportState;
    pipelineCreateInfo->pDepthStencilState = VK_NULL_HANDLE;
    pipelineCreateInfo->pDynamicState = VK_NULL_HANDLE;

    return pipelineCreateInfo;
}

void ThinDrawer::freePipelineData(VkGraphicsPipelineCreateInfo* pipelineCreateInfo)
{
    free((void*)pipelineCreateInfo->pColorBlendState->pAttachments);
    free((void*)pipelineCreateInfo->pColorBlendState);
    free((void*)pipelineCreateInfo->pViewportState->pViewports);
    free((void*)pipelineCreateInfo->pViewportState->pScissors);
    free((void*)pipelineCreateInfo->pViewportState);
    free((void*)pipelineCreateInfo->pInputAssemblyState);
    free((void*)pipelineCreateInfo->pRasterizationState);
    free((void*)pipelineCreateInfo->pMultisampleState);
    free((void*)pipelineCreateInfo);
}

void ThinDrawer::preparePipelines()
{
    texturedShader->preparePipeline();
    debugCircleShader->preparePipeline();
}

void ThinDrawer::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> sizes =
    {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = { };
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = sizes.size();
    descriptorPoolInfo.pPoolSizes = sizes.data();
    descriptorPoolInfo.maxSets = 100;

    CHECK_RESULT_VK(vkCreateDescriptorPool(logicalDevice, &descriptorPoolInfo, VK_NULL_HANDLE, &descriptorPool));
}

void ThinDrawer::setupDescriptorSet()
{
    texturedShader->setupDescriptorSet();
    debugCircleShader->setupDescriptorSet();
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

        vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, texturedShader->pipeline);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                texturedShader->pipelineLayout, 0, 1, &texturedShader->descriptorSet, 0, VK_NULL_HANDLE);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                texturedShader->pipelineLayout, 1, 1, &texturedShader->textureData[0]->set, 0, VK_NULL_HANDLE);

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(drawCommandBuffers[i], 0, 1, &texturedShader->buffers[0]->buffer, offsets);
        vkCmdBindIndexBuffer(drawCommandBuffers[i], texturedShader->buffers[1]->buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(drawCommandBuffers[i], texturedShader->buffers[1]->count, 1, 0, 0, 1);

        // for debug circle

        vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, debugCircleShader->pipeline);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                debugCircleShader->pipelineLayout, 0, 1, &debugCircleShader->descriptorSet, 0, VK_NULL_HANDLE);

        vkCmdBindVertexBuffers(drawCommandBuffers[i], 0, 1, &debugCircleShader->buffers[0]->buffer, offsets);
        vkCmdBindIndexBuffer(drawCommandBuffers[i], debugCircleShader->buffers[1]->buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(drawCommandBuffers[i], debugCircleShader->buffers[1]->count, 1, 0, 0, 1);

        vkCmdEndRenderPass(drawCommandBuffers[i]);
        CHECK_RESULT_VK(vkEndCommandBuffer(drawCommandBuffers[i]));
    }
}