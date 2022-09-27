#include <ThinDrawer.h>
#include <SwapChain.h>
#include <Shader.h>
#include <vector>
#include <array>

ThinDrawer::ThinDrawer()
{
    initBase();
    initExtra();
}

void ThinDrawer::initExtra()
{
    prepareVertices();
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();

    loadTexture((char*)"../assets/textures/chain.png", &singleTexture);
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

    std::vector<uint32_t> indexBuffer = { 0, 1, 2, 2, 3, 0 };
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
    uboVS.modelMatrix = glm::scale(uboVS.modelMatrix, glm::vec3(2.5f, 1.0f, 1.0f));

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

    VkDescriptorSetLayoutBinding textureBind = { };
    textureBind.binding = 0;
    textureBind.descriptorCount = 1;
    textureBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo setInfo = { };
    setInfo.bindingCount = 1;
    setInfo.flags = 0;
    setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setInfo.pBindings = &textureBind;
    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(logicalDevice, &setInfo, NULL, &textureSetLayout))

    VkDescriptorSetLayout setLayouts[] = { descriptorSetLayout, textureSetLayout };

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = { };
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.setLayoutCount = sizeof(setLayouts) / sizeof(setLayouts[0]);
    pPipelineLayoutCreateInfo.pSetLayouts = setLayouts;

    CHECK_RESULT_VK(vkCreatePipelineLayout(logicalDevice, &pPipelineLayoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout))
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
    viewport.width = width;
    viewport.height = height;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = { width, height };

    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineMultisampleStateCreateInfo multisampleState = { };
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = samples;
    multisampleState.pSampleMask = nullptr;

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
    pipelineCreateInfo.pDynamicState = VK_NULL_HANDLE;

    CHECK_RESULT_VK(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline))
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
    descriptorPoolInfo.pNext = nullptr;
    descriptorPoolInfo.poolSizeCount = sizes.size();
    descriptorPoolInfo.pPoolSizes = sizes.data();
    descriptorPoolInfo.maxSets = 10;

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

    updateImageDescriptors(&singleTexture);
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
        viewport.minDepth = (float)0.0f;
        viewport.maxDepth = (float)1.0f;
        vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = { };
        scissor.extent.width = width;
        scissor.extent.height = height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;

        vkCmdSetScissor(drawCommandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout, 0, 1, &descriptorSet, 0, VK_NULL_HANDLE);

        vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout, 1, 1, &singleTexture.set, 0, VK_NULL_HANDLE);

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(drawCommandBuffers[i], 0, 1, &vertices.buffer, offsets);
        vkCmdBindIndexBuffer(drawCommandBuffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(drawCommandBuffers[i], indices.count, 1, 0, 0, 1);
        vkCmdEndRenderPass(drawCommandBuffers[i]);
        CHECK_RESULT_VK(vkEndCommandBuffer(drawCommandBuffers[i]))
    }
}