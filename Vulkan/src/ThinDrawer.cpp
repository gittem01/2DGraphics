#include <ThinDrawer.h>
#include <SwapChain.h>
#include <Camera.h>
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
    vkDestroyPipeline(logicalDevice, debugLineShader->pipeline, VK_NULL_HANDLE);
    vkDestroyPipeline(logicalDevice, debugPolygonShader->pipeline, VK_NULL_HANDLE);
    vkDestroyRenderPass(logicalDevice, renderPass, VK_NULL_HANDLE);
    swapChain->destroy();
    
    swapChain->creationLoop();
    createRenderPass();
    swapChain->createFrameBuffers();
    preparePipelines();

    for (int i = 0; i < frames.size(); i++)
    {
        vkResetCommandBuffer(frames[i].commandBuffer, 0);
    }

    buildCommandBuffers();

    vkDeviceWaitIdle(logicalDevice);
}

void ThinDrawer::initExtra()
{
    texturedShader = new VulkanTriangle(this);
    debugCircleShader = new DebugCircle(this);
    debugLineShader = new DebugLine(this);
    debugPolygonShader = new DebugPolygon(this);

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

void ThinDrawer::uniformFiller(int frameNum)
{
    // base
    s_sharedUniformData* sharedData;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, sharedOrthoUniform[frameNum]->memory, 0, sizeof(s_sharedUniformData), 0, (void**)&sharedData));
    sharedData->orthoMatrix = wh->cam->ortho;
    vkUnmapMemory(logicalDevice, sharedOrthoUniform[frameNum]->memory);

    // textured
    s_uboVS* pData;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, texturedShader->uniformBuffers[frameNum][0]->memory, 0, sizeof(s_uboVS), 0, (void**)&pData));
    vkUnmapMemory(logicalDevice, texturedShader->uniformBuffers[frameNum][0]->memory);

    CHECK_RESULT_VK(vkMapMemory(logicalDevice, debugCircleShader->uniformBuffers[frameNum][0]->memory, 0, sizeof(s_uboVS), 0, (void**)&pData));
    vkUnmapMemory(logicalDevice, debugCircleShader->uniformBuffers[frameNum][0]->memory);

    // circle
    s_uboFSPoly* uboFSCircle;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, debugCircleShader->uniformBuffers[frameNum][1]->memory, 0, sizeof(uboFSCircle), 0, (void**)&uboFSCircle));
    uboFSCircle->data.x = wh->cam->zoom;
    vkUnmapMemory(logicalDevice, debugCircleShader->uniformBuffers[frameNum][1]->memory);

    // line
    s_uboVSLine* uboVS;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, debugLineShader->uniformBuffers[frameNum][0]->memory, 0, sizeof(uboVS), 0, (void**)&uboVS));
    vkUnmapMemory(logicalDevice, debugLineShader->uniformBuffers[frameNum][0]->memory);

    s_uboMixedLine* uboMixed;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, debugLineShader->uniformBuffers[frameNum][1]->memory, 0, sizeof(uboMixed), 0, (void**)&uboMixed));
    uboMixed->data.x = wh->cam->zoom;
    vkUnmapMemory(logicalDevice, debugLineShader->uniformBuffers[frameNum][1]->memory);

    // polygon
    s_uboVSPOLY* polyData;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, debugPolygonShader->uniformBuffers[frameNum][0]->memory, 0, sizeof(polyData), 0, (void**)&polyData));
    polyData->modelMatrix = glm::mat4(1.0f);
    polyData->polyPoints[0] = glm::vec4(-2.0f, 1.0f,  2.0f, 0.5f);
    polyData->polyPoints[1] = glm::vec4(2.0f, -0.5f, -2.0f, -1.0f);
    polyData->polyPoints[2] = glm::vec4(-3.0f, 0.0f, 0.0f, 0.0f);
    polyData->numPoints = 5;
    vkUnmapMemory(logicalDevice, debugPolygonShader->uniformBuffers[frameNum][0]->memory);

    s_uboFSPoly* uboPoly;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, debugPolygonShader->uniformBuffers[frameNum][1]->memory, 0, sizeof(uboPoly), 0, (void**)&uboPoly));
    uboPoly->data.x = wh->cam->zoom;
    vkUnmapMemory(logicalDevice, debugPolygonShader->uniformBuffers[frameNum][1]->memory);
}

void ThinDrawer::renderLoop()
{
    wh->looper();
    wh->cam->update();

    int imNum = frameNumber % swapChain->imageCount;

    s_frameData currentFrame = frames[imNum];

    CHECK_RESULT_VK(vkWaitForFences(logicalDevice, 1, &currentFrame.renderFence, true, UINT64_MAX));
    CHECK_RESULT_VK(vkResetFences(logicalDevice, 1, &currentFrame.renderFence));

    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain->swapChain, UINT64_MAX,
                          currentFrame.presentSemaphore, VK_NULL_HANDLE, &lastSwapChainImageIndex);

    uniformFiller(lastSwapChainImageIndex);

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
    submit.pCommandBuffers = &frames[lastSwapChainImageIndex].commandBuffer;

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
    texturedShader->prepareVertexData();
    debugCircleShader->prepareVertexData();
    debugLineShader->prepareVertexData();
    debugPolygonShader->prepareVertexData();
}

void ThinDrawer::prepareUniformBuffers()
{
    sharedOrthoUniform.resize(swapChain->imageCount);
    for (int i = 0; i < swapChain->imageCount; i++)
    {
        sharedOrthoUniform[i] = (s_uniformBuffer*)malloc(sizeof(s_uniformBuffer));
        uniformHelper(sizeof(s_sharedUniformData), sharedOrthoUniform[i]);
    }
    texturedShader->prepareUniforms();
    debugCircleShader->prepareUniforms();
    debugLineShader->prepareUniforms();
    debugPolygonShader->prepareUniforms();
}

void ThinDrawer::setupDescriptorSetLayout()
{
    texturedShader->setupDescriptorSetLayout();
    debugCircleShader->setupDescriptorSetLayout();
    debugLineShader->setupDescriptorSetLayout();
    debugPolygonShader->setupDescriptorSetLayout();
}

void ThinDrawer::preparePipelines()
{
    texturedShader->preparePipeline();
    debugCircleShader->preparePipeline();
    debugLineShader->preparePipeline();
    debugPolygonShader->preparePipeline();
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
    debugLineShader->setupDescriptorSet();
    debugPolygonShader->setupDescriptorSet();
}

void ThinDrawer::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = vkinit::commandBufferBeginInfo();

    VkClearValue clearValue;
    clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };
    VkRenderPassBeginInfo renderPassBeginInfo = vkinit::renderPassBeginInfo(renderPass, swapChain->extent.width, swapChain->extent.height, &clearValue);

    for (int32_t i = 0; i < frames.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = swapChain->frameBuffers[i];

        CHECK_RESULT_VK(vkBeginCommandBuffer(frames[i].commandBuffer, &cmdBufInfo));

        vkCmdBeginRenderPass(frames[i].commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = { };
        viewport.height = (float)swapChain->extent.height;
        viewport.width = (float)swapChain->extent.width;
        viewport.minDepth = (float)0.0f;
        viewport.maxDepth = (float)1.0f;
        vkCmdSetViewport(frames[i].commandBuffer, 0, 1, &viewport);

        // for debug triangle
        vkCmdBindPipeline(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, texturedShader->pipeline);

        vkCmdBindDescriptorSets(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                texturedShader->pipelineLayout, 0, 1, &texturedShader->descriptorSet[i], 0, VK_NULL_HANDLE);

        vkCmdBindDescriptorSets(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                texturedShader->pipelineLayout, 1, 1, &texturedShader->textureData[0]->set, 0, VK_NULL_HANDLE);

        VkDeviceSize offsets = 0;
        vkCmdBindVertexBuffers(frames[i].commandBuffer, 0, 1, &texturedShader->buffers[0]->buffer, &offsets);
        vkCmdBindIndexBuffer(frames[i].commandBuffer, texturedShader->buffers[1]->buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frames[i].commandBuffer, texturedShader->buffers[1]->count, 1, 0, 0, 1);

        // for debug circle

        vkCmdBindPipeline(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, debugCircleShader->pipeline);

        vkCmdBindDescriptorSets(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                debugCircleShader->pipelineLayout, 0, 1, &debugCircleShader->descriptorSet[i], 0, VK_NULL_HANDLE);

        vkCmdBindVertexBuffers(frames[i].commandBuffer, 0, 1, &debugCircleShader->buffers[0]->buffer, &offsets);
        vkCmdBindIndexBuffer(frames[i].commandBuffer, debugCircleShader->buffers[1]->buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frames[i].commandBuffer, debugCircleShader->buffers[1]->count, 1, 0, 0, 1);

        // for debug line

        vkCmdBindPipeline(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, debugLineShader->pipeline);

        vkCmdBindDescriptorSets(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            debugLineShader->pipelineLayout, 0, 1, &debugLineShader->descriptorSet[i], 0, VK_NULL_HANDLE);

        vkCmdBindVertexBuffers(frames[i].commandBuffer, 0, 1, &debugCircleShader->buffers[0]->buffer, &offsets);
        vkCmdBindIndexBuffer(frames[i].commandBuffer, debugCircleShader->buffers[1]->buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frames[i].commandBuffer, debugCircleShader->buffers[1]->count, 1, 0, 0, 1);

        // for debug polygon

        vkCmdBindPipeline(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, debugPolygonShader->pipeline);

        vkCmdBindDescriptorSets(frames[i].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            debugPolygonShader->pipelineLayout, 0, 1, &debugPolygonShader->descriptorSet[i], 0, VK_NULL_HANDLE);

        vkCmdDraw(frames[i].commandBuffer, 9, 1, 0, 0);

        // end

        vkCmdEndRenderPass(frames[i].commandBuffer);
        CHECK_RESULT_VK(vkEndCommandBuffer(frames[i].commandBuffer));
    }
}