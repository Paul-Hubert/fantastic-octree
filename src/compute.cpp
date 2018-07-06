#include <QVulkanDeviceFunctions>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "helper.h"
#include "compute.h"
#include "windu.h"

Compute::Compute(Windu *win) {
    this->win = win;
}

struct Transform {
    glm::mat4 mvp;
};

void Compute::init() {
    
    {
        VkDescriptorPoolSize size[2];
        size[0] = {};
        size[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        size[0].descriptorCount = win->swap.NUM_FRAMES;
        size[1] = {};
        size[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        size[1].descriptorCount = win->swap.NUM_FRAMES;
        
        VkDescriptorPoolCreateInfo dpinfo = {};
        dpinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpinfo.poolSizeCount = 2;
        dpinfo.pPoolSizes = &size[0];
        dpinfo.maxSets = win->swap.NUM_FRAMES;
        foAssert(win->vkd->vkCreateDescriptorPool(win->device.logical, &dpinfo, nullptr, &descriptorPool));
    }
    
    
    {
        VkDescriptorSetLayoutBinding slb[2];
        slb[0] = {};
        slb[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        slb[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        slb[0].descriptorCount = 1;
        slb[0].binding = 0;
        
        slb[1] = {};
        slb[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        slb[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        slb[1].descriptorCount = 1;
        slb[1].binding = 1;
        
        
        VkDescriptorSetLayoutCreateInfo slinfo = {};
        slinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        slinfo.bindingCount = 2;
        slinfo.pBindings = &slb[0];
        
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
        QByteArray code = foLoad("src/compute.comp.spv");
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
        
        win->vkd->vkDestroyShaderModule(win->device.logical, shaderModule, nullptr);
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
    
    {
    
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = &win->device.c_i;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.size = sizeof(Transform);
        
        win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, nullptr, &ubo);
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, ubo, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, nullptr, &uniformMemory));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, ubo, uniformMemory, 0));
        
    }
    
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        win->vkd->vkCreateFence(win->device.logical, &info, nullptr, &fence);
    }
    
    setup();
    
    prepare(&win->sync);
}

void Compute::setup() {
    
    {
        VkExtent3D extent = {};
        extent.width = win->swap.extent.width;
        extent.height = win->swap.extent.height;
        extent.depth = 1;
        
        
    
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.extent = extent;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        
        std::vector<uint32_t> qi;
        qi.push_back(win->device.c_i);
        if(win->device.t_i != win->device.c_i) qi.push_back(win->device.t_i);
        imageInfo.sharingMode = qi.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = qi.size();
        imageInfo.pQueueFamilyIndices = qi.data();
        
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        
        
        images.resize(win->swap.NUM_FRAMES);
        for(uint32_t i = 0; i<win->swap.NUM_FRAMES; i++) {
            foAssert(win->vkd->vkCreateImage(win->device.logical, &imageInfo, nullptr, &images[i]));
        }
    }
    
    
    {
        VkMemoryRequirements memreq = {};
        std::vector<VkMemoryRequirements> req(images.size());
        for(uint32_t i = 0; i < images.size(); i++) {
            
            win->vkd->vkGetImageMemoryRequirements(win->device.logical, images[i], &req[i]);
            memreq.size += (req[i].size/req[i].alignment+1)*req[i].alignment;
            memreq.alignment = req[i].alignment;
            memreq.memoryTypeBits |= req[i].memoryTypeBits;
        }
        
        if(size < memreq.size) {
            
            if(size != 0) {
                win->vkd->vkFreeMemory(win->device.logical, memory, nullptr);
            }
            size = memreq.size;
            
            std::cout << "Reallocating compute image memory\n";
            
            VkMemoryAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            info.allocationSize = size;
            win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
            
            foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, nullptr, &memory));
            
        }
        
        for(uint32_t i = 0; i < images.size(); i++) {
            foAssert(win->vkd->vkBindImageMemory(win->device.logical, images[i], memory, i * size/3));
        }
    }
    
    
    
    {
        imageViews.resize(images.size());
        for(uint32_t i = 0; i < imageViews.size(); i++) {
            
            VkImageViewCreateInfo vInfo = {};
            vInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            vInfo.image = images[i];
            vInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            vInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            
            vInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            vInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            vInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            vInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            
            vInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vInfo.subresourceRange.baseMipLevel = 0;
            vInfo.subresourceRange.levelCount = 1;
            vInfo.subresourceRange.baseArrayLayer = 0;
            vInfo.subresourceRange.layerCount = 1;
            
            foAssert(win->vkd->vkCreateImageView(win->device.logical, &vInfo, nullptr, &imageViews[i]));
            
        }
    }
    
    {
        std::vector<VkDescriptorImageInfo> imageInfos(win->swap.NUM_FRAMES);
        for(uint32_t i = 0; i<imageInfos.size(); i++) {
            imageInfos[i].sampler = nullptr;
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageInfos[i].imageView = imageViews[i];
        }
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = ubo;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Transform);
        
        std::vector<VkWriteDescriptorSet> write(win->swap.NUM_FRAMES*2);
        for(uint32_t i = 0; i<win->swap.NUM_FRAMES; i++) {
            write[i] = {};
            write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write[i].descriptorCount = 1;
            write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            write[i].pImageInfo = &imageInfos[i];
            write[i].dstSet = descriptorSet[i];
            write[i].dstBinding = 0;
            write[i].dstArrayElement = 0;
        }
        for(uint32_t i = win->swap.NUM_FRAMES; i<win->swap.NUM_FRAMES*2; i++) {
            write[i] = {};
            write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write[i].descriptorCount = 1;
            write[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write[i].pBufferInfo = &bufferInfo;
            write[i].dstSet = descriptorSet[i - win->swap.NUM_FRAMES];
            write[i].dstBinding = 1;
            write[i].dstArrayElement = 0;
        }
        
        
        win->vkd->vkUpdateDescriptorSets(win->device.logical, write.size(), write.data(), 0, nullptr);
    }
    
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        
        win->vkd->vkQueueWaitIdle(win->device.compute);
        
        for(uint32_t i = 0; i < win->swap.NUM_FRAMES; i++) {
            
            foAssert(win->vkd->vkBeginCommandBuffer(commandBuffers[i], &beginInfo));
            
            win->vkd->vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
            win->vkd->vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet[i], 0, 0);
            
            win->vkd->vkCmdDispatch(commandBuffers[i], win->swap.extent.width / 16 + 1, win->swap.extent.height / 16 + 1, 1);
            
            foAssert(win->vkd->vkEndCommandBuffer(commandBuffers[i]));
            
        }
    }
    
}


void Compute::render(uint32_t i) {
    
    
    t++;
    
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &commandBuffers[i];
    std::vector<VkSemaphore> wait(waitSemaphores);
    info.pWaitSemaphores = wait.data();
    sync();
    info.waitSemaphoreCount = waitCount;
    info.pWaitDstStageMask = waitStages.data();
    info.signalSemaphoreCount = signalCount;
    info.pSignalSemaphores = signalSemaphores.data();
    
    
    Transform ubo = {};
    ubo.mvp = win->camera.getViewProj();
    
    win->vkd->vkWaitForFences(win->device.logical, 1, &fence, true, 1000000000000000L);
    
    void* data;
    win->vkd->vkMapMemory(win->device.logical, uniformMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    win->vkd->vkUnmapMemory(win->device.logical, uniformMemory);
    
    win->vkd->vkResetFences(win->device.logical, 1, &fence);
    
    foAssert(win->vkd->vkQueueSubmit(win->device.compute, 1, &info, fence));
    
}


void Compute::cleanup() {
    for(uint32_t i = 0; i < images.size(); i++) {
        win->vkd->vkDestroyImageView(win->device.logical, imageViews[i], nullptr);
        win->vkd->vkDestroyImage(win->device.logical, images[i], nullptr);
    }
}

void Compute::reset() {
    cleanup();
    setup();
}


Compute::Compute::~Compute() {
    
    cleanup();
    
    win->vkd->vkDestroyFence(win->device.logical, fence, nullptr);
    
    win->vkd->vkDestroyBuffer(win->device.logical, ubo, nullptr);
    
    win->vkd->vkFreeMemory(win->device.logical, uniformMemory, nullptr);
    
    win->vkd->vkFreeMemory(win->device.logical, memory, nullptr);
    
    win->vkd->vkDestroyCommandPool(win->device.logical, commandPool, nullptr);
    
    win->vkd->vkDestroyPipeline(win->device.logical, pipeline, nullptr);
    
    //foAssert(win->vkd->vkFreeDescriptorSets(win->device.logical, descriptorPool, descriptorSet.size(), descriptorSet.data()));
    
    win->vkd->vkDestroyPipelineLayout(win->device.logical, pipelineLayout, nullptr);
    
    win->vkd->vkDestroyDescriptorSetLayout(win->device.logical, descriptorSetLayout, nullptr);
    
    win->vkd->vkDestroyDescriptorPool(win->device.logical, descriptorPool, nullptr);
    
}
