#include "resource_manager.h"

#include "../windu.h"

#include <iostream>
#include <math.h>
#include <vector>
#include <set>

#include "../helper.h"

void ResourceManager::allocateResource(FoResourceName name, vk::DeviceSize size) {
    
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

void ResourceManager::initBuffer(FoResourceName name, std::set<uint32_t> si, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags) {
    
    FoBuffer* buffer = getBuffer(name);
    
    std::vector<uint32_t> qi(si.begin(), si.end());
    buffer->buffer = win->device.logical.createBuffer({{}, buffer->size, bufferUsageFlags, qi.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive, static_cast<uint32_t>(qi.size()), qi.data() });
    
    vk::MemoryRequirements memreq = win->device.logical.getBufferMemoryRequirements(buffer->buffer);
    
    vk::MemoryAllocateInfo info((memreq.size/memreq.alignment + 1) * memreq.alignment * 4, 0);
    win->device.getMemoryType(memreq.memoryTypeBits, memoryPropertyFlags, &info.memoryTypeIndex);
    
    buffer->memory = win->device.logical.allocateMemory(info);
    
    win->device.logical.bindBufferMemory(buffer->buffer, buffer->memory, 0);
    
}

void ResourceManager::init() {
    
    {
        initBuffer(FO_RESOURCE_DENSITY_BUFFER,
                   {win->device.g_i, win->device.c_i},
                   vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
                   vk::MemoryPropertyFlagBits::eDeviceLocal);
    }
    
    {
        initBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER,
                   {win->device.t_i},
                   vk::BufferUsageFlagBits::eTransferSrc,
                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }
    
    {
        initBuffer(FO_RESOURCE_LOOKUP_BUFFER,
                   {win->device.c_i},
                   vk::BufferUsageFlagBits::eUniformTexelBuffer,
                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }

    {
        initBuffer(FO_RESOURCE_VERTEX_BUFFER,
                   {win->device.g_i, win->device.c_i},
                   vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,
                   vk::MemoryPropertyFlagBits::eDeviceLocal);
    }

    {
        initBuffer(FO_RESOURCE_UNIFORM_BUFFER,
                   {win->device.g_i},
                   vk::BufferUsageFlagBits::eUniformBuffer,
                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }
    
    {
        initBuffer(FO_RESOURCE_INDIRECT_DRAW,
                   {win->device.g_i, win->device.c_i},
                   vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer,
                   vk::MemoryPropertyFlagBits::eDeviceLocal);
    }
    
    {
        initBuffer(FO_RESOURCE_RAY_UBO,
                   {win->device.c_i},
                   vk::BufferUsageFlagBits::eUniformBuffer,
                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
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

