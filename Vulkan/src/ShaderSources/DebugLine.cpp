#include <ShaderHeaders/DebugLine.h>
#include <SwapChain.h>
#include <ThinDrawer.h>
#include <Camera.h>
#include <Shader.h>
#include <definitions.h>
#include <vkInit.h>

void DebugLine::prepareVertexData()
{
    const int bufferSize = 0;
    buffers.resize(bufferSize);
}

void DebugLine::prepareUniforms()
{
    const int bufferSize = 2;
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

        thinDrawer->uniformHelper(sizeof(s_uboVSLine), uniformBufferVS);

        uint8_t* pData;
        s_uboVSLine uboVS;
        uboVS.linePoints = glm::vec4(0.2f, -1.0f, -0.2f, 1.0f);
        CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferVS->memory, 0, sizeof(uboVS), 0, (void**)&pData));
        memcpy(pData, &uboVS, sizeof(uboVS));

        vkUnmapMemory(logicalDevice, uniformBufferVS->memory);

        thinDrawer->uniformHelper(sizeof(s_uboMixedLine), uniformBufferMix);

        s_uboMixedLine uboMixed;
        uboMixed.color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        uboMixed.data.x = 1.0f;
        uboMixed.data.y = 0.1f;
        uboMixed.data.z = 0.5f;
        CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferMix->memory, 0, sizeof(uboMixed), 0, (void**)&pData));
        memcpy(pData, &uboMixed, sizeof(uboMixed));

        vkUnmapMemory(logicalDevice, uniformBufferMix->memory);
    }
}

void DebugLine::setupDescriptorSetLayout()
{
    const int descriptorSetLayoutCount = 1;

    descriptorSets.resize(descriptorSetLayoutCount);

    VkDescriptorSetLayoutBinding layoutBinding0 =
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, 0);

    VkDescriptorSetLayoutBinding layoutBinding1 =
        vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, 1);

    VkDescriptorSetLayoutBinding layoutBinding2 = vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, 2);

    VkDescriptorSetLayoutBinding bindings1[] = { layoutBinding0, layoutBinding1, layoutBinding2 };

    VkDescriptorSetLayoutCreateInfo descriptorLayout = vkinit::descriptorSetLayoutCreateInfo(bindings1, sizeof(bindings1) / sizeof(bindings1[0]));
    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(logicalDevice, &descriptorLayout, VK_NULL_HANDLE, &descriptorSets[0]));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vkinit::pipelineLayoutCreateInfo(descriptorSets.data(), descriptorSets.size());
    CHECK_RESULT_VK(vkCreatePipelineLayout(logicalDevice, &pPipelineLayoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout));
}

void DebugLine::preparePipeline()
{
    VkGraphicsPipelineCreateInfo* pipelineCreateInfo = thinDrawer->getPipelineInfoBase();
    pipelineCreateInfo->layout = pipelineLayout;

    VkVertexInputBindingDescription vertexInputBinding = { };
    vertexInputBinding.binding = 0;
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBinding.stride = sizeof(s_basicVertex);

    VkVertexInputAttributeDescription vertexInputAttributeFor0;
    vertexInputAttributeFor0.binding = 0;
    vertexInputAttributeFor0.location = 0;
    vertexInputAttributeFor0.format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeFor0.offset = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputState = { };
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.vertexAttributeDescriptionCount = 1;
    vertexInputState.pVertexAttributeDescriptions = &vertexInputAttributeFor0;
    vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;

    std::vector<std::string> fileNames = {
            std::string("shaders/DebugLine/vertex_shader.vert.spv"),
            std::string("shaders/DebugLine/fragment_shader.frag.spv")
    };

    Shader shader = Shader(logicalDevice, fileNames);

    pipelineCreateInfo->stageCount = shader.shaderStages.size();
    pipelineCreateInfo->pStages = shader.shaderStages.data();
    pipelineCreateInfo->pVertexInputState = &vertexInputState;

    CHECK_RESULT_VK(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, pipelineCreateInfo, VK_NULL_HANDLE, &pipeline));
}