#include <ShaderBase.h>
#include <ThinDrawer.h>
#include <SwapChain.h>
#include <vkInit.h>

ShaderBase::ShaderBase(ThinDrawer* thinDrawer)
{
    this->thinDrawer = thinDrawer;
    this->logicalDevice = thinDrawer->logicalDevice;
}

void ShaderBase::setupDescriptorSet()
{
    int imCount = thinDrawer->swapChain->imageCount;
    descriptorSet.resize(imCount);

    for (int i = 0; i < imCount; i++)
    {
        VkDescriptorSetAllocateInfo allocInfo = vkinit::descriptorSetAllocateInfo(thinDrawer->descriptorPool, &descriptorSets[0], 1);
        CHECK_RESULT_VK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet[i]));

        VkWriteDescriptorSet writeDescriptorSet = { };

        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet[i];
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        writeDescriptorSet.pBufferInfo = &thinDrawer->sharedOrthoUniform[i]->descriptor;
        writeDescriptorSet.dstBinding = 0;
        vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);

        for (int j = 0; j < uniformBuffers[i].size(); j++)
        {
            writeDescriptorSet.pBufferInfo = &uniformBuffers[i][j]->descriptor;
            writeDescriptorSet.dstBinding = j + 1;

            vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);
        }
    }
}