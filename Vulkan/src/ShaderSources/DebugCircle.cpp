#include <ShaderHeaders/DebugCircle.h>
#include <ThinDrawer.h>
#include <Camera.h>
#include <Shader.h>
#include <definitions.h>
#include <vkInit.h>

void DebugCircle::prepareVertexData()
{
    const int bufferSize = 2;
    buffers.resize(bufferSize);

    for (int i = 0; i < bufferSize; i++)
    {
        buffers[i] = (s_buffers*)malloc(sizeof(s_buffers));
    }
    s_buffers* vertices = buffers[0];
    s_buffers* indices = buffers[1];

    std::vector<s_basicVertex> vertexBuffer =
    {
            { {+1.0f, +1.0f} },
            { {-1.0f, +1.0f} },
            { {-1.0f, -1.0f} },
            { {+1.0f, -1.0f} },
    };
    uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(s_basicVertex);

    std::vector<uint32_t> indexBuffer = { 0, 1, 2, 2, 3, 0 };
    indices->count = static_cast<uint32_t>(indexBuffer.size());
    uint32_t indexBufferSize = indices->count * sizeof(uint32_t);

    thinDrawer->bufferStage(vertexBuffer.data(), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vertices);
    thinDrawer->bufferStage(indexBuffer.data(), indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, indices);
}

void DebugCircle::prepareUniforms()
{
    const int bufferSize = 2;

    uniformBuffers.resize(bufferSize);

    for (int i = 0; i < bufferSize; i++)
    {
        uniformBuffers[i] = (s_uniformBuffer*)malloc(sizeof(s_uniformBuffer));
    }

    s_uniformBuffer* uniformBufferVS = uniformBuffers[0];
    s_uniformBuffer* uniformBufferFS = uniformBuffers[1];

    thinDrawer->uniformHelper(sizeof(s_uboVS), uniformBufferVS);

    uint8_t* pData;
    s_uboVS uboVS;
    uboVS.modelMatrix = glm::mat4(1.0f);
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::pi<float>() * 0.25f, glm::vec3(0, 0, 1));
    uboVS.modelMatrix = glm::scale(uboVS.modelMatrix, glm::vec3(2.5f, 1.0f, 1.0f));
    uboVS.modelMatrix = glm::mat4(1.0f);
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferVS->memory, 0, sizeof(uboVS), 0, (void**)&pData));
    memcpy(pData, &uboVS, sizeof(uboVS));

    vkUnmapMemory(logicalDevice, uniformBufferVS->memory);

    thinDrawer->uniformHelper(sizeof(s_uboFSColor), uniformBufferFS);

    s_uboFSColor uboFSColor;
    uboFSColor.color = glm::vec4(0.2f, 1.0f, 1.0f, 0.6f);

    CHECK_RESULT_VK(vkMapMemory(logicalDevice, uniformBufferFS->memory, 0, sizeof(uboFSColor), 0, (void**)&pData));
    memcpy(pData, &uboFSColor, sizeof(uboFSColor));

    vkUnmapMemory(logicalDevice, uniformBufferFS->memory);
}

void DebugCircle::setupDescriptorSetLayout()
{
    const int descriptorSetLayoutCount = 1;

    descriptorSets.resize(descriptorSetLayoutCount);

    VkDescriptorSetLayoutBinding layoutBinding =
        vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, 0);
    VkDescriptorSetLayoutBinding layoutBinding2 = vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);

    VkDescriptorSetLayoutBinding bindings1[] = { layoutBinding, layoutBinding2 };

    VkDescriptorSetLayoutCreateInfo descriptorLayout = vkinit::descriptorSetLayoutCreateInfo(bindings1, sizeof(bindings1) / sizeof(bindings1[0]));
    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(logicalDevice, &descriptorLayout, VK_NULL_HANDLE, &descriptorSets[0]));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vkinit::pipelineLayoutCreateInfo(descriptorSets.data(), descriptorSets.size());
    CHECK_RESULT_VK(vkCreatePipelineLayout(logicalDevice, &pPipelineLayoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout));
}

void DebugCircle::preparePipeline()
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
            std::string("../assets/shaders/VulkanDebugCircle/vertex_shader.vert.spv"),
            std::string("../assets/shaders/VulkanDebugCircle/fragment_shader.frag.spv")
    };

    Shader shader2 = Shader(logicalDevice, fileNames);

    pipelineCreateInfo->stageCount = shader2.shaderStages.size();
    pipelineCreateInfo->pStages = shader2.shaderStages.data();
    pipelineCreateInfo->pVertexInputState = &vertexInputState;

    CHECK_RESULT_VK(vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, pipelineCreateInfo, VK_NULL_HANDLE, &pipeline));
}

void DebugCircle::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo = vkinit::descriptorSetAllocateInfo(thinDrawer->descriptorPool, descriptorSets.data(), 1);

    CHECK_RESULT_VK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet));

    VkWriteDescriptorSet writeDescriptorSet = { };
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.pBufferInfo = &uniformBuffers[0]->descriptor;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.dstBinding = 0;
    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);

    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.pBufferInfo = &uniformBuffers[1]->descriptor;
    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);
}