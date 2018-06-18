#include <QVulkanDeviceFunctions>

#include "helper.h"
#include "compute.h"
#include "windu.h"

Compute::Compute(Windu *win) {
    this->win = win;
}

void Compute::init() {
    
    {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        poolSize.descriptorCount = win->swap.NUM_FRAMES;
        
        VkDescriptorPoolCreateInfo dpinfo = {};
        dpinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpinfo.poolSizeCount = 1;
        dpinfo.pPoolSizes = &poolSize;
        dpinfo.maxSets = win->swap.NUM_FRAMES;
        foAssert(win->vkd->vkCreateDescriptorPool(win->device.logical, &dpinfo, nullptr, &descriptorPool));
    }
    
    
    {
        VkDescriptorSetLayoutBinding slb = {};
        slb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        slb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        slb.descriptorCount = 1;
        slb.binding = 0;
        
        VkDescriptorSetLayoutCreateInfo slinfo = {};
        slinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        slinfo.bindingCount = 1;
        slinfo.pBindings = &slb;
        
        foAssert(win->vkd->vkCreateDescriptorSetLayout(win->device.logical, &slinfo, nullptr, &descriptorSetLayout));
    }
    
    
    {
        VkPipelineLayoutCreateInfo plinfo = {};
        plinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plinfo.setLayoutCount = 1;
        plinfo.pSetLayouts = &descriptorSetLayout;
        
        foAssert(win->vkd->vkCreatePipelineLayout(win->device.logical, &plinfo, nullptr, &pipelineLayout));
    }
    
    
    {
        std::vector<VkDescriptorSetLayout> layouts(win->swap.NUM_FRAMES);
        for(uint32_t i = 0; i<layouts.size(); i++) {
            layouts[i] = descriptorSetLayout;
        }
        
        VkDescriptorSetAllocateInfo allocinfo = {};
        allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocinfo.descriptorSetCount = win->swap.NUM_FRAMES;
        allocinfo.pSetLayouts = layouts.data();
        allocinfo.descriptorPool = descriptorPool;
        
        descriptorSet.resize(win->swap.NUM_FRAMES);
        foAssert(win->vkd->vkAllocateDescriptorSets(win->device.logical, &allocinfo, descriptorSet.data()));
    }
    
    
    {
        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        QByteArray code = load("src/compute.comp.spv");
        moduleInfo.codeSize = code.size();
        moduleInfo.pCode = reinterpret_cast<const uint32_t*>(code.constData());
        
        foAssert(win->vkd->vkCreateShaderModule(win->device.logical, &moduleInfo, nullptr, &shaderModule));
    }
    
    
    {
        VkPipelineShaderStageCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo.module = shaderModule;
        shaderInfo.pName = "main";
        shaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        
        VkComputePipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.stage = shaderInfo;
        
        foAssert(win->vkd->vkCreateComputePipelines(win->device.logical, nullptr, 1, &pipelineInfo, nullptr, &pipeline));
    }
    
    
    {    
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = win->device.c_i;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        
        foAssert(win->vkd->vkCreateCommandPool(win->device.logical, &poolInfo, nullptr, &commandPool));
    }
    
    
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = win->swap.NUM_FRAMES;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        commandBuffers.resize(win->swap.NUM_FRAMES);
        foAssert(win->vkd->vkAllocateCommandBuffers(win->device.logical, &allocInfo, commandBuffers.data()));
    }
    
    
    setup();
    
}

void Compute::setup() {
    
    std::vector<VkDescriptorImageInfo> imageInfos(win->swap.NUM_FRAMES);
    for(uint32_t i = 0; i<imageInfos.size(); i++) {
        imageInfos[i].sampler = nullptr;
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageInfos[i].imageView = win->swap.imageViews[i];
    }
    
    std::vector<VkWriteDescriptorSet> write(win->swap.NUM_FRAMES);
    for(uint32_t i = 0; i<write.size(); i++) {
        write[i] = {};
        write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write[i].descriptorCount = 1;
        write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        write[i].pImageInfo = &imageInfos[i];
        write[i].dstSet = descriptorSet[i];
        write[i].dstBinding = 0;
        write[i].dstArrayElement = 0;
    }
    
    win->vkd->vkUpdateDescriptorSets(win->device.logical, write.size(), write.data(), 0, nullptr);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    
    win->vkd->vkQueueWaitIdle(win->device.compute);

    for(uint32_t i = 0; i < win->swap.NUM_FRAMES; i++) {
        
        foAssert(win->vkd->vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

        win->vkd->vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        win->vkd->vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet[i], 0, 0);

        win->vkd->vkCmdDispatch(commandBuffers[i], win->swap.extent.width / 16, win->swap.extent.height / 16, 1);

        foAssert(win->vkd->vkEndCommandBuffer(commandBuffers[i]));
    }
    
}

void Compute::cleanup() {
    
}

void Compute::reset() {
    cleanup();
    setup();
}


Compute::Compute::~Compute() {
    
    cleanup();
    
    win->vkd->vkDestroyCommandPool(win->device.logical, commandPool, nullptr);
    
    win->vkd->vkDestroyPipeline(win->device.logical, pipeline, nullptr);
    
    win->vkd->vkDestroyShaderModule(win->device.logical, shaderModule, nullptr);
    
    //foAssert(win->vkd->vkFreeDescriptorSets(win->device.logical, descriptorPool, descriptorSet.size(), descriptorSet.data()));
    
    win->vkd->vkDestroyPipelineLayout(win->device.logical, pipelineLayout, nullptr);
    
    win->vkd->vkDestroyDescriptorSetLayout(win->device.logical, descriptorSetLayout, nullptr);
    
    win->vkd->vkDestroyDescriptorPool(win->device.logical, descriptorPool, nullptr);
    
}
