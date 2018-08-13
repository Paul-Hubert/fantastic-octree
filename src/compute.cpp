#include <QVulkanDeviceFunctions>
#include <iostream>
#include <math.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "compute.h"
#include "windu.h"
#include "terrain.h"
#include "helper.h"

Compute::Compute(Windu *win) {
    this->win = win;
}

struct Transform {
    glm::mat4 viewinvproj;
    glm::vec3 pos;
};

void Compute::init() {
    
    {
        VkDescriptorPoolSize size[3];
        size[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        size[0].descriptorCount = win->swap.NUM_FRAMES;
        size[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        size[1].descriptorCount = win->swap.NUM_FRAMES;
        size[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        size[2].descriptorCount = win->swap.NUM_FRAMES;
        
        VkDescriptorPoolCreateInfo dpinfo = {};
        dpinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpinfo.poolSizeCount = 3;
        dpinfo.pPoolSizes = &size[0];
        dpinfo.maxSets = win->swap.NUM_FRAMES;
        foAssert(win->vkd->vkCreateDescriptorPool(win->device.logical, &dpinfo, VK_NULL_HANDLE, &descriptorPool));
    }
    
    
    {
        VkDescriptorSetLayoutBinding slb[3];
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
        
        slb[2] = {};
        slb[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        slb[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        slb[2].descriptorCount = 1;
        slb[2].binding = 2;
        
        VkDescriptorSetLayoutCreateInfo slinfo = {};
        slinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        slinfo.bindingCount = 3;
        slinfo.pBindings = &slb[0];
        
        foAssert(win->vkd->vkCreateDescriptorSetLayout(win->device.logical, &slinfo, VK_NULL_HANDLE, &descriptorSetLayout));
    }
    
    
    {
        VkPipelineLayoutCreateInfo plinfo = {};
        plinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plinfo.setLayoutCount = 1;
        plinfo.pSetLayouts = &descriptorSetLayout;
        
        foAssert(win->vkd->vkCreatePipelineLayout(win->device.logical, &plinfo, VK_NULL_HANDLE, &pipelineLayout));
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
        
        foAssert(win->vkd->vkCreateShaderModule(win->device.logical, &moduleInfo, VK_NULL_HANDLE, &shaderModule));
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
        
        foAssert(win->vkd->vkCreateComputePipelines(win->device.logical, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &pipeline));
        
        win->vkd->vkDestroyShaderModule(win->device.logical, shaderModule, VK_NULL_HANDLE);
    }
    
    
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = win->device.c_i;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        
        foAssert(win->vkd->vkCreateCommandPool(win->device.logical, &poolInfo, VK_NULL_HANDLE, &commandPool));
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
        
        win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &ubo);
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, ubo, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &uniformMemory));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, ubo, uniformMemory, 0));
        
    }
    
    {
        
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = ubo;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Transform);
        
        std::vector<VkWriteDescriptorSet> write(win->swap.NUM_FRAMES);
        for(uint32_t i = 0; i<win->swap.NUM_FRAMES; i++) {
            write[i] = {};
            write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write[i].descriptorCount = 1;
            write[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write[i].pBufferInfo = &bufferInfo;
            write[i].dstSet = descriptorSet[i];
            write[i].dstBinding = 1;
            write[i].dstArrayElement = 0;
        }
        
        win->vkd->vkUpdateDescriptorSets(win->device.logical, write.size(), write.data(), 0, VK_NULL_HANDLE);
        
    }
    
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        win->vkd->vkCreateFence(win->device.logical, &info, VK_NULL_HANDLE, &fence);
    }
    
    
    
    
    
    
    
    
    
    // OCTREE BUFFER STUFF
    
    {    
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = win->device.t_i;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        
        foAssert(win->vkd->vkCreateCommandPool(win->device.logical, &poolInfo, VK_NULL_HANDLE, &transferPool));
    }
    
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        win->vkd->vkCreateSemaphore(win->device.logical, &info, VK_NULL_HANDLE, &transferSem);
    }
    
    {
    
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = &win->device.c_i;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.size = OCTREE_SIZE * sizeof(Chunk);
        
        win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &octreeHostBuffer);
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, octreeHostBuffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &octreeHostMemory));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, octreeHostBuffer, octreeHostMemory, 0));
        
    }
    
    {
    
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        std::vector<uint32_t> qi;
        qi.push_back(win->device.c_i);
        if(win->device.t_i != win->device.c_i) qi.push_back(win->device.t_i);
        bufferInfo.sharingMode = qi.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = qi.size();
        bufferInfo.pQueueFamilyIndices = qi.data();
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.size = OCTREE_SIZE * sizeof(Chunk);
        
        win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &octreeBuffer);
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, octreeBuffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &octreeMemory));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, octreeBuffer, octreeMemory, 0));
        
    }
    
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = this->octreeBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = OCTREE_SIZE * sizeof(Chunk);
        
        std::vector<VkWriteDescriptorSet> write(win->swap.NUM_FRAMES);
        for(uint32_t i = 0; i<win->swap.NUM_FRAMES; i++) {
            write[i] = {};
            write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write[i].descriptorCount = 1;
            write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write[i].pBufferInfo = &bufferInfo;
            write[i].dstSet = descriptorSet[i];
            write[i].dstBinding = 2;
            write[i].dstArrayElement = 0;
        }
        
        win->vkd->vkUpdateDescriptorSets(win->device.logical, write.size(), write.data(), 0, VK_NULL_HANDLE);
        
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
        imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT;
        
        std::vector<uint32_t> qi;
        qi.push_back(win->device.c_i);
        if(win->device.g_i != win->device.c_i) qi.push_back(win->device.g_i);
        imageInfo.sharingMode = qi.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = qi.size();
        imageInfo.pQueueFamilyIndices = qi.data();
        
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        
        
        images.resize(win->swap.NUM_FRAMES);
        for(uint32_t i = 0; i<win->swap.NUM_FRAMES; i++) {
            foAssert(win->vkd->vkCreateImage(win->device.logical, &imageInfo, VK_NULL_HANDLE, &images[i]));
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
                win->vkd->vkFreeMemory(win->device.logical, memory, VK_NULL_HANDLE);
            }
            size = memreq.size;
            
            std::cout << "Reallocating compute image memory\n";
            
            VkMemoryAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            info.allocationSize = size;
            win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
            
            foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &memory));
            
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
            
            foAssert(win->vkd->vkCreateImageView(win->device.logical, &vInfo, VK_NULL_HANDLE, &imageViews[i]));
            
        }
    }
    
    {
        std::vector<VkDescriptorImageInfo> imageInfos(win->swap.NUM_FRAMES);
        for(uint32_t i = 0; i<imageInfos.size(); i++) {
            imageInfos[i].sampler = VK_NULL_HANDLE;
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageInfos[i].imageView = imageViews[i];
        }
        
        std::vector<VkWriteDescriptorSet> write(win->swap.NUM_FRAMES);
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
        
        win->vkd->vkUpdateDescriptorSets(win->device.logical, write.size(), write.data(), 0, VK_NULL_HANDLE);
    }
    
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        
        for(uint32_t i = 0; i < win->swap.NUM_FRAMES; i++) {
            
            foAssert(win->vkd->vkBeginCommandBuffer(commandBuffers[i], &beginInfo));
            
            win->vkd->vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
            win->vkd->vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet[i], 0, 0);
            
            win->vkd->vkCmdDispatch(commandBuffers[i], (int) ceil(win->swap.extent.width / 8.), (int) ceil(win->swap.extent.height / 8.), 1);
            
            foAssert(win->vkd->vkEndCommandBuffer(commandBuffers[i]));
            
        }
    }
    
}





void* Compute::allocate(int size) {
    
    void* ptr;
    foAssert(win->vkd->vkMapMemory(win->device.logical, octreeHostMemory, 0, size, 0, &ptr));
    
    return ptr;
}

void Compute::upload(int offset, int size) {
    
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = transferPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    
    VkCommandBuffer transferCmd;
    foAssert(win->vkd->vkAllocateCommandBuffers(win->device.logical, &allocInfo, &transferCmd));
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    foAssert(win->vkd->vkBeginCommandBuffer(transferCmd, &beginInfo));
    
    VkBufferCopy copy = {};
    copy.dstOffset = offset;
    copy.srcOffset = offset;
    copy.size = size;
    
    win->vkd->vkCmdCopyBuffer(transferCmd, octreeHostBuffer, octreeBuffer, 1, &copy);
    
    foAssert(win->vkd->vkEndCommandBuffer(transferCmd));
    
    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &transferCmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &transferSem;
    
    this->prepareSignal(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, transferSem);
    
    win->vkd->vkQueueSubmit(win->device.transfer, 1, &submit, VK_NULL_HANDLE);
    
}

void Compute::deallocate() {
    win->vkd->vkUnmapMemory(win->device.logical, octreeHostMemory);
    
    win->vkd->vkDestroyBuffer(win->device.logical, octreeHostBuffer, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, octreeHostMemory, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyBuffer(win->device.logical, octreeBuffer, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, octreeMemory, VK_NULL_HANDLE);
}






void Compute::render(uint32_t i) {
    sync();

    t++;
    
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &commandBuffers[i];
    std::vector<VkSemaphore> wait(waitSemaphores);
    info.waitSemaphoreCount = waitCount;
    info.pWaitSemaphores = wait.data();
    info.pWaitDstStageMask = waitStages.data();
    info.signalSemaphoreCount = signalCount;
    info.pSignalSemaphores = signalSemaphores.data();
    
    
    Transform ubo = {};
    ubo.viewinvproj = win->camera.getView() * glm::inverse(win->camera.getProj());
    ubo.pos = win->camera.getPos();
    
    win->vkd->vkWaitForFences(win->device.logical, 1, &fence, true, 1000000000000000L);
    
    void* data;
    win->vkd->vkMapMemory(win->device.logical, uniformMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    win->vkd->vkUnmapMemory(win->device.logical, uniformMemory);
    
    win->vkd->vkResetFences(win->device.logical, 1, &fence);
    
    foAssert(win->vkd->vkQueueSubmit(win->device.compute, 1, &info, fence));

    postsync();
    
}


void Compute::cleanup() {
    for(uint32_t i = 0; i < images.size(); i++) {
        win->vkd->vkDestroyImageView(win->device.logical, imageViews[i], VK_NULL_HANDLE);
        win->vkd->vkDestroyImage(win->device.logical, images[i], VK_NULL_HANDLE);
    }
}

void Compute::reset() {
    cleanup();
    setup();
}


Compute::Compute::~Compute() {
    
    cleanup();
    
    win->vkd->vkDestroySemaphore(win->device.logical, transferSem, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyFence(win->device.logical, fence, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyCommandPool(win->device.logical, transferPool, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyBuffer(win->device.logical, ubo, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, uniformMemory, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, memory, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyCommandPool(win->device.logical, commandPool, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyPipeline(win->device.logical, pipeline, VK_NULL_HANDLE);
    
    //foAssert(win->vkd->vkFreeDescriptorSets(win->device.logical, descriptorPool, descriptorSet.size(), descriptorSet.data()));
    
    win->vkd->vkDestroyPipelineLayout(win->device.logical, pipelineLayout, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyDescriptorSetLayout(win->device.logical, descriptorSetLayout, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyDescriptorPool(win->device.logical, descriptorPool, VK_NULL_HANDLE);
    
}
