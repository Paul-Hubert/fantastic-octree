#include "resource_manager.h"

#include <iostream>
#include <math.h>
#include <vector>

#include "../windu.h"
#include "../helper.h"

void ResourceManager::init() {
    
}


void ResourceManager::allocateDensityBuffer(VkDeviceSize size) {
    
    density = {};
    density.size = size;
    density.offset = 0;
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    std::vector<uint32_t> qi;
    qi.push_back(win->device.c_i);
    if(win->device.g_i != win->device.c_i) qi.push_back(win->device.t_i);
    bufferInfo.sharingMode = qi.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = qi.size();
    bufferInfo.pQueueFamilyIndices = qi.data();
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.size = density.size;
    
    foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(density.buffer)));
    
    VkMemoryRequirements memreq;
    win->vkd->vkGetBufferMemoryRequirements(win->device.logical, density.buffer, &memreq);
    
    VkMemoryAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
    win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
    
    foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(density.memory)));
    
    foAssert(win->vkd->vkBindBufferMemory(win->device.logical, density.buffer, density.memory, 0));
    
}

void ResourceManager::allocateStagingDensityBuffer(VkDeviceSize size) {
    
    stagingDensity = {};
    stagingDensity.size = size;
    stagingDensity.offset = 0;
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &(win->device.t_i);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.size = stagingDensity.size;
    
    foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(stagingDensity.buffer)));
    
    VkMemoryRequirements memreq;
    win->vkd->vkGetBufferMemoryRequirements(win->device.logical, stagingDensity.buffer, &memreq);
    
    VkMemoryAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
    win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
    
    foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(stagingDensity.memory)));
    
    foAssert(win->vkd->vkBindBufferMemory(win->device.logical, stagingDensity.buffer, stagingDensity.memory, 0));
    
}

void ResourceManager::allocateCubesBuffer(VkDeviceSize size) {
    
    cubes = {};
    cubes.size = size;
    cubes.offset = 0;
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &(win->device.c_i);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.size = cubes.size;
    
    foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(cubes.buffer)));
    
    VkMemoryRequirements memreq;
    win->vkd->vkGetBufferMemoryRequirements(win->device.logical, cubes.buffer, &memreq);
    
    VkMemoryAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
    win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
    
    foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(cubes.memory)));
    
    foAssert(win->vkd->vkBindBufferMemory(win->device.logical, cubes.buffer, cubes.memory, 0));
    
}

void ResourceManager::allocateLookupBuffer(VkDeviceSize size) {
    
    lookup = {};
    lookup.size = size;
    lookup.offset = 0;
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &(win->device.c_i);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    bufferInfo.size = lookup.size;
    
    foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(lookup.buffer)));
    
    VkMemoryRequirements memreq;
    win->vkd->vkGetBufferMemoryRequirements(win->device.logical, lookup.buffer, &memreq);
    
    VkMemoryAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
    win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
    
    foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(lookup.memory)));
    
    foAssert(win->vkd->vkBindBufferMemory(win->device.logical, lookup.buffer, lookup.memory, 0));
    
}

void ResourceManager::allocateVertexBuffer(VkDeviceSize size) {
    
    vertex = {};
    vertex.size = size;
    vertex.offset = 0;
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    std::vector<uint32_t> qi;
    qi.push_back(win->device.c_i);
    if(win->device.g_i != win->device.c_i) qi.push_back(win->device.g_i);
    bufferInfo.sharingMode = qi.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = qi.size();
    bufferInfo.pQueueFamilyIndices = qi.data();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.size = size;
    
    foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(vertex.buffer)));
    
    VkMemoryRequirements memreq;
    win->vkd->vkGetBufferMemoryRequirements(win->device.logical, vertex.buffer, &memreq);
    
    VkMemoryAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
    win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &info.memoryTypeIndex);
    
    foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(vertex.memory)));
    
    foAssert(win->vkd->vkBindBufferMemory(win->device.logical, vertex.buffer, vertex.memory, 0));
    
}

void ResourceManager::allocateUniformBuffer(VkDeviceSize size) {
    
    uniform = {};
    uniform.size = size;
    uniform.offset = 0;
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &win->device.c_i;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.size = size;
    
    foAssert(win->vkd->vkCreateBuffer(win->device.logical, &bufferInfo, VK_NULL_HANDLE, &(uniform.buffer)));
    
    VkMemoryRequirements memreq;
    win->vkd->vkGetBufferMemoryRequirements(win->device.logical, uniform.buffer, &memreq);
    
    VkMemoryAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.allocationSize = (memreq.size/memreq.alignment + 1) * memreq.alignment * 4;
    win->device.getMemoryType(memreq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &info.memoryTypeIndex);
    
    foAssert(win->vkd->vkAllocateMemory(win->device.logical, &info, VK_NULL_HANDLE, &(uniform.memory)));
    
    foAssert(win->vkd->vkBindBufferMemory(win->device.logical, uniform.buffer, uniform.memory, 0));
    
}



Buffer ResourceManager::getDensityBuffer() {
    return density;
}

Buffer ResourceManager::getStagingDensityBuffer() {
    return stagingDensity;
}

Buffer ResourceManager::getCubesBuffer() {
    return cubes;
}

Buffer ResourceManager::getLookupBuffer() {
    return lookup;
}

Buffer ResourceManager::getVertexBuffer() {
    return vertex;
}

Buffer ResourceManager::getUniformBuffer() {
    return uniform;
}



ResourceManager::ResourceManager(Windu* win) {
    this->win = win;
}

ResourceManager::~ResourceManager() {
    
    win->vkd->vkDestroyBuffer(win->device.logical, stagingDensity.buffer, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, stagingDensity.memory, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyBuffer(win->device.logical, lookup.buffer, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, lookup.memory, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyBuffer(win->device.logical, cubes.buffer, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, cubes.memory, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyBuffer(win->device.logical, uniform.buffer, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, uniform.memory, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyBuffer(win->device.logical, density.buffer, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, density.memory, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyBuffer(win->device.logical, vertex.buffer, VK_NULL_HANDLE);
    
    win->vkd->vkFreeMemory(win->device.logical, vertex.memory, VK_NULL_HANDLE);
    
}

