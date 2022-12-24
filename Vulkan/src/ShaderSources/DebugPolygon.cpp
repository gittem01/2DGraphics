#include <ShaderHeaders/DebugPolygon.h>
#include <SwapChain.h>
#include <ThinDrawer.h>
#include <Camera.h>
#include <Shader.h>
#include <definitions.h>
#include <vkInit.h>

void DebugPolygon::prepareVertexData()
{
    const int bufferSize = 0;
    buffers.resize(bufferSize);
}

void DebugPolygon::prepareUniforms()
{
    const int bufferSize = 4;
    int imCount = thinDrawer->swapChain->imageCount;
    uniformBuffers.resize(imCount);

    for (int i = 0; i < imCount; i++)
    {
        uniformBuffers[i].resize(bufferSize);

        for (int j = 0; j < bufferSize; j++)
        {
            uniformBuffers[i][j] = (s_uniformBuffer*)malloc(sizeof(s_uniformBuffer));
        }

        s_uniformBuffer* uniformBufferVS = uniformBuffers[i][0];
        s_uniformBuffer* uniformBufferMix = uniformBuffers[i][1];
        s_uniformBuffer* uniformBufferPortalNum = uniformBuffers[i][2];
        s_uniformBuffer* uniformBufferPortals = uniformBuffers[i][3];

        thinDrawer->uniformHelper(sizeof(s_uboVSPOLY), uniformBufferVS);

        uint8_t* pData;

        s_uboFSPoly data;
        data.color = glm::vec4(0.0f, 0.5f, 1.0f, 0.1f);
        data.outerColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
        data.data.y = 0.1f;
        data.data.z = 0.5f;
        thinDrawer->uniformHelper(sizeof(s_uboFSPoly), uniformBufferMix);
        CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferMix->memory, 0, sizeof(s_uboFSPoly), 0, (void**)&pData));
        memcpy(pData, &data, sizeof(s_uboFSPoly));
        vkUnmapMemory(logicalDevice, uniformBufferMix->memory);

        //glm::vec4 vec[2] = { glm::vec4(-1.0f, 0.5f, 1.0f, 1.0f), glm::vec4(1.0f, -0.5f, -1.0f, -1.0f) };
        glm::vec4 vec[] = { };
        thinDrawer->uniformHelper(256 * sizeof(glm::vec4), uniformBufferPortals);
        if (sizeof(vec))
        {
            CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferPortals->memory, 0, sizeof(vec), 0, (void**)&pData));
            memcpy(pData, &vec, sizeof(vec));
            vkUnmapMemory(logicalDevice, uniformBufferPortals->memory);
        }

        thinDrawer->uniformHelper(sizeof(int), uniformBufferPortalNum);
        CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferPortalNum->memory, 0, sizeof(int), 0, (void**)&pData));
        int val = sizeof(vec) / sizeof(glm::vec4);
        memcpy(pData, &val, sizeof(int));
        vkUnmapMemory(logicalDevice, uniformBufferPortalNum->memory);
    }
}

void DebugPolygon::setupDescriptorSetLayout()
{
    const int descriptorSetLayoutCount = 1;

    descriptorSets.resize(descriptorSetLayoutCount);

    VkDescriptorSetLayoutBinding layoutBinding0 =
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, 0);

    VkDescriptorSetLayoutBinding layoutBinding1 =
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);

    VkDescriptorSetLayoutBinding layoutBinding2 =
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 2);

    VkDescriptorSetLayoutBinding layoutBinding3 =
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 3);

    VkDescriptorSetLayoutBinding layoutBinding4 =
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 4);

    VkDescriptorSetLayoutBinding bindings[] = { layoutBinding0, layoutBinding1, layoutBinding2, layoutBinding3, layoutBinding4 };

    VkDescriptorSetLayoutCreateInfo descriptorLayout = vkinit::descriptorSetLayoutCreateInfo(bindings, sizeof(bindings) / sizeof(bindings[0]));
    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(logicalDevice, &descriptorLayout, VK_NULL_HANDLE, &descriptorSets[0]));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vkinit::pipelineLayoutCreateInfo(
        descriptorSets.data(), descriptorSets.size());
    CHECK_RESULT_VK(vkCreatePipelineLayout(logicalDevice, &pPipelineLayoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout));
}

void DebugPolygon::preparePipeline()
{
    VkGraphicsPipelineCreateInfo* pipelineCreateInfo = thinDrawer->getPipelineInfoBase();

    pipelineCreateInfo->layout = pipelineLayout;

    VkPipelineVertexInputStateCreateInfo vertexInputState = { };
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 0;
    vertexInputState.vertexAttributeDescriptionCount = 0;

    std::vector<std::string> fileNames = {
            std::string("shaders/DebugPolygon/vertex_shader.vert.spv"),
            std::string("shaders/DebugPolygon/fragment_shader.frag.spv")
    };

    Shader shader = Shader(logicalDevice, fileNames);

    pipelineCreateInfo->stageCount = shader.shaderStages.size();
    pipelineCreateInfo->pStages = shader.shaderStages.data();

    pipelineCreateInfo->pVertexInputState = &vertexInputState;

    CHECK_RESULT_VK(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, pipelineCreateInfo, VK_NULL_HANDLE, &pipeline));

    ThinDrawer::freePipelineData(pipelineCreateInfo);
}