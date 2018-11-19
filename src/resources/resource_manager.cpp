#include "resource_manager.h"

#include <iostream>
#include <math.h>
#include <vector>

#include "../windu.h"
#include "../helper.h"

void ResourceManager::allocateResource(FoResourceName name, VkDeviceSize size) {
    
    if(name < FO_RESOURCE_BUFFER_COUNT) { // IT'S A BUFFER
        FoBuffer buffer = {};
        buffer.size = size;
        buffers[name] = buffer;
    } else { // IT'S AN IMAGE
        FoImage image = {};
        image.size = size;
        images[name] = image;
    }
    
}

FoBuffer* ResourceManager::getBuffer(FoResourceName name) {
    return &buffers[name];
}

FoImage* ResourceManager::getImage(FoResourceName name) {
    return &images[name - FO_RESOURCE_BUFFER_COUNT];
}

void ResourceManager::init() {
    
    {
    
        FoBuffer* density = getBuffer(FO_RESOURCE_DENSITY_BUFFER);
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        std::vector<uint32_t> qi;
        qi.push_back(win->device.c_i);
        if(win->device.g_i != win->device.c_i) qi.push_back(win->device.t_i);
        bufferInfo.sharingMode = qi.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = qi.size();
        bufferInfo.pQueueFamilyIndices = qi.data();
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.size = density->size;
        
        foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(density->buffer)));
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, density->buffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(density->memory)));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, density->buffer, density->memory, 0));
        
    }
    
    {
        
        FoBuffer* stagingDensity = getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER);
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = &(win->device.t_i);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.size = stagingDensity->size;
        
        foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(stagingDensity->buffer)));
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, stagingDensity->buffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(stagingDensity->memory)));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, stagingDensity->buffer, stagingDensity->memory, 0));
        
    }
    
    {
        
        FoBuffer* cubes = getBuffer(FO_RESOURCE_CUBES_BUFFER);
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = &(win->device.c_i);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.size = cubes->size;
        
        foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(cubes->buffer)));
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, cubes->buffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(cubes->memory)));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, cubes->buffer, cubes->memory, 0));
        
    }
    
    {
        
        FoBuffer* lookup = getBuffer(FO_RESOURCE_LOOKUP_BUFFER);
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = &(win->device.c_i);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        bufferInfo.size = lookup->size;
        
        foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(lookup->buffer)));
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, lookup->buffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(lookup->memory)));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, lookup->buffer, lookup->memory, 0));
        
    }

    {
        
        FoBuffer* vertex = getBuffer(FO_RESOURCE_VERTEX_BUFFER);
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        std::vector<uint32_t> qi;
        qi.push_back(win->device.c_i);
        if(win->device.g_i != win->device.c_i) qi.push_back(win->device.g_i);
        bufferInfo.sharingMode = qi.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = qi.size();
        bufferInfo.pQueueFamilyIndices = qi.data();
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.size = vertex->size;
        
        foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(vertex->buffer)));
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, vertex->buffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(vertex->memory)));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, vertex->buffer, vertex->memory, 0));
        
    }

    {
    
        FoBuffer* uniform = getBuffer(FO_RESOURCE_UNIFORM_BUFFER);
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = &win->device.c_i;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.size = uniform->size;
        
        foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(uniform->buffer)));
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, uniform->buffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(uniform->memory)));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, uniform->buffer, uniform->memory, 0));
        
    }
    
    {
    
        FoBuffer* dispatch = getBuffer(FO_RESOURCE_INDIRECT_DISPATCH);
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.queueFamilyIndexCount = 1;
        bufferInfo.pQueueFamilyIndices = &win->device.c_i;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.size = dispatch->size;
        
        foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(dispatch->buffer)));
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, dispatch->buffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(dispatch->memory)));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, dispatch->buffer, dispatch->memory, 0));
        
    }
    
    {
    
        FoBuffer* dispatch = getBuffer(FO_RESOURCE_INDIRECT_DRAW);
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        std::vector<uint32_t> qi;
        qi.push_back(win->device.c_i);
        if(win->device.g_i != win->device.c_i) qi.push_back(win->device.g_i);
        bufferInfo.sharingMode = qi.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = qi.size();
        bufferInfo.pQueueFamilyIndices = qi.data();
        bufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferInfo.size = dispatch->size;
        
        foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(dispatch->buffer)));
        
        VkMemoryRequirements memreq;
        win->vkd->vkGetBufferMemoryRequirements(win->device.logical, dispatch->buffer, &memreq);
        
        VkMemoryAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
        win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
        
        foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(dispatch->memory)));
        
        foAssert(win->vkd->vkBindBufferMemory(win->device.logical, dispatch->buffer, dispatch->memory, 0));
        
    }
    
}


ResourceManager::ResourceManager(Windu* win) {
    this->win = win;
}

ResourceManager::~ResourceManager() {
    
    for(int i = 0; i<FO_RESOURCE_BUFFER_COUNT; i++) {
        win->vkd->vkDestroyBuffer(win->device.logical, buffers[i].buffer, VK_NULL_HANDLE);
        win->vkd->vkFreeMemory(win->device.logical, buffers[i].memory, VK_NULL_HANDLE);
    }
    
}

