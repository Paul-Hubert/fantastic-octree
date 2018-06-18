#include <QVulkanDeviceFunctions>

#include "helper.h"
#include "compute.h"
#include "windu.h"

Compute::Compute(Windu *win) {
    this->win = win;
}

void Compute::init() {
    
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSize.descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo dpinfo = {};
    dpinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpinfo.poolSizeCount = 1;
    dpinfo.pPoolSizes = &poolSize;
    dpinfo.maxSets = win->swap.NUM_FRAMES;
    foAssert(win->vkd->vkCreateDescriptorPool(win->device.logical, &dpinfo, nullptr, &descriptorPool));
    
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
    
    VkPipelineLayoutCreateInfo plinfo = {};
    plinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plinfo.setLayoutCount = 1;
    plinfo.pSetLayouts = &descriptorSetLayout;
    
    foAssert(win->vkd->vkCreatePipelineLayout(win->device.logical, &plinfo, nullptr, &pipelineLayout));
    
    std::vector<VkDescriptorSetLayout> layouts(win->swap.NUM_FRAMES);
    for(uint32_t i = 0; i<layouts.size(); i++) {
        layouts[i] = descriptorSetLayout;
    }
    
    VkDescriptorSetAllocateInfo allocinfo = {};
    allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocinfo.descriptorSetCount = win->swap.NUM_FRAMES;
    allocinfo.pSetLayouts = &descriptorSetLayout;
    allocinfo.descriptorPool = descriptorPool;
    
    descriptorSet.resize(win->swap.NUM_FRAMES);
    foAssert(win->vkd->vkAllocateDescriptorSets(win->device.logical, &allocinfo, descriptorSet.data()));
    
    VkPipelineShaderStageCreateInfo shaderInfo = {};
    //shaderInfo.sType = 
    
    setup();
    
}

void Compute::setup() {
    
    std::vector<VkDescriptorImageInfo> imageInfos(win->swap.NUM_FRAMES);
    for(uint32_t i = 0; i<imageInfos.size(); i++) {
        imageInfos[i].sampler = nullptr;
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageInfos[i].imageView = win->swap.imageViews[i];
    }
    
    std::vector<VkWriteDescriptorSet> write = {};
    for(uint32_t i = 0; i<write.size(); i++) {
        write[i].descriptorCount = 1;
        write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        write[i].pImageInfo = &imageInfos[i];
        write[i].dstSet = descriptorSet[i];
        write[i].dstBinding = 0;
        write[i].dstArrayElement = 0;
    }
    
    win->vkd->vkUpdateDescriptorSets(win->device.logical, write.size(), write.data(), 0, nullptr);
    
    
    /*
    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stage;
    */
    
    
}

void Compute::cleanup() {
    
}

void Compute::reset() {
    cleanup();
    setup();
}


Compute::Compute::~Compute() {
    
    cleanup();
    
    win->vkd->vkFreeDescriptorSets(win->device.logical, descriptorPool, win->swap.NUM_FRAMES, descriptorSet.data());
    
    win->vkd->vkDestroyPipelineLayout(win->device.logical, pipelineLayout, nullptr);
    
    win->vkd->vkDestroyDescriptorSetLayout(win->device.logical, descriptorSetLayout, nullptr);
    
    win->vkd->vkDestroyDescriptorPool(win->device.logical, descriptorPool, nullptr);
    
}
