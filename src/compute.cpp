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

Compute::Compute(Windu *win) : premcubes(win), mcubes(win) {
    this->win = win;
}

void Compute::preinit() {
    
    win->resman.allocateResource(FO_RESOURCE_CUBES_BUFFER, MAX_CUBES * sizeof(Cube));
    
    win->resman.allocateResource(FO_RESOURCE_STAGING_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_VERTEX_BUFFER, MAX_CUBES * 15 * sizeof(Vertex));
    
    win->resman.allocateResource(FO_RESOURCE_LOOKUP_BUFFER, 4096*sizeof(char));
    
    win->resman.allocateResource(FO_RESOURCE_INDIRECT_DISPATCH, sizeof(VkDispatchIndirectCommand));
    
    win->resman.allocateResource(FO_RESOURCE_INDIRECT_DRAW, sizeof(VkDrawIndirectCommand));
    
    premcubes.preinit();
    
    mcubes.preinit();
    
}


void Compute::init() {
    
    premcubes.init();
    
    mcubes.init();
    
    initRest();
    
    setup();
    
    prepare(&win->sync);
}

void Compute::setup() {
    
    premcubes.setup();
    
    mcubes.setup();
    
}









bool Compute::isActive() {
    return true;
}


void Compute::render(uint32_t i) {
    sync();

    t++;
    
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &commandBuffer;
    std::vector<VkSemaphore> wait(waitSemaphores);
    info.waitSemaphoreCount = waitCount;
    info.pWaitSemaphores = wait.data();
    info.pWaitDstStageMask = waitStages.data();
    info.signalSemaphoreCount = signalCount;
    info.pSignalSemaphores = signalSemaphores.data();
    
    win->vkd->vkWaitForFences(win->device.logical, 1, &fence, true, 1000000000000000L);
    
    
    win->vkd->vkResetFences(win->device.logical, 1, &fence);
    
    foAssert(win->vkd->vkQueueSubmit(win->device.compute, 1, &info, fence));

    postsync();
    
}






void* Compute::startWriteDensity() {
    
    FoBuffer* density = win->resman.getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER);
    
    void* ptr;
    foAssert(win->vkd->vkMapMemory(win->device.logical, density->memory, density->offset, density->size, 0, &ptr));
    
    return ptr;
}

void Compute::finishWriteDensity() {
    
    win->vkd->vkUnmapMemory(win->device.logical, win->resman.getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER)->memory);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    foAssert(win->vkd->vkBeginCommandBuffer(transferCmd, &beginInfo));
    
    VkBufferCopy copy = {};
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = win->resman.getBuffer(FO_RESOURCE_DENSITY_BUFFER)->size;
    
    win->vkd->vkCmdCopyBuffer(transferCmd, win->resman.getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER)->buffer, win->resman.getBuffer(FO_RESOURCE_DENSITY_BUFFER)->buffer, 1, &copy);
    
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

void * Compute::startWriteCubes() {
    
    FoBuffer* cubes = win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER);
    
    void* ptr;
    foAssert(win->vkd->vkMapMemory(win->device.logical, cubes->memory, cubes->offset, cubes->size, 0, &ptr));
    
    return ptr;
    
}

void Compute::finishWriteCubes(int num) {
    
    std::cout << num << std::endl;
    
    win->vkd->vkUnmapMemory(win->device.logical, win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER)->memory);
    
}




void Compute::initRest() {
    
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = win->device.c_i;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        
        foAssert(win->vkd->vkCreateCommandPool(win->device.logical, &poolInfo, VK_NULL_HANDLE, &commandPool));

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        foAssert(win->vkd->vkAllocateCommandBuffers(win->device.logical, &allocInfo, &commandBuffer));

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        foAssert(win->vkd->vkCreateFence(win->device.logical, &fenceInfo, VK_NULL_HANDLE, &fence));
    }
    
    
    {
        VkCommandPoolCreateInfo transferInfo = {};
        transferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        transferInfo.queueFamilyIndex = win->device.t_i;
        transferInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        
        foAssert(win->vkd->vkCreateCommandPool(win->device.logical, &transferInfo, VK_NULL_HANDLE, &transferPool));
        
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = transferPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        foAssert(win->vkd->vkAllocateCommandBuffers(win->device.logical, &allocInfo, &transferCmd));

        VkSemaphoreCreateInfo semInfo = {};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;        
        foAssert(win->vkd->vkCreateSemaphore(win->device.logical, &semInfo, VK_NULL_HANDLE, &transferSem));
    }
    
    {
        
        FoBuffer* lookup = win->resman.getBuffer(FO_RESOURCE_LOOKUP_BUFFER);
        void* ptr;
        QByteArray bytes = foLoad("src/resources/mclookup.bin");
        foAssert(win->vkd->vkMapMemory(win->device.logical, lookup->memory, lookup->offset, lookup->size, 0, &ptr));
        memcpy(ptr, bytes.constData(), lookup->size);
        win->vkd->vkUnmapMemory(win->device.logical, lookup->memory);
        
    }
    
    {
        
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        
        foAssert(win->vkd->vkBeginCommandBuffer(commandBuffer, &beginInfo));
        
        
        // PREMCUBES
        win->vkd->vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, premcubes.pipeline);
        win->vkd->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, premcubes.pipelineLayout, 0, 1, &(premcubes.descriptorSet), 0, 0);
        
        win->vkd->vkCmdDispatch(commandBuffer, CHUNK_SIZE/8, CHUNK_SIZE/8, CHUNK_SIZE/8); // TO CHANGE
        
        // CUBES MEMORY BARRIER
        FoBuffer* cubes = win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER);
        FoBuffer* indirect = win->resman.getBuffer(FO_RESOURCE_INDIRECT_DISPATCH);
        VkBufferMemoryBarrier bmb[] = {{},{}};
        bmb[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bmb[0].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        bmb[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        bmb[0].srcQueueFamilyIndex = win->device.c_i;
        bmb[0].dstQueueFamilyIndex = win->device.c_i;
        bmb[0].buffer = cubes->buffer;
        bmb[0].size = cubes->size;
        bmb[0].offset = 0;
        
        bmb[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bmb[1].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        bmb[1].dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        bmb[1].srcQueueFamilyIndex = win->device.c_i;
        bmb[1].dstQueueFamilyIndex = win->device.c_i;
        bmb[1].buffer = indirect->buffer;
        bmb[1].size = indirect->size;
        bmb[1].offset = 0;
        
        win->vkd->vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 2, &bmb[0], 0, NULL);
        
        // MCUBES
        win->vkd->vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mcubes.pipeline);
        win->vkd->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mcubes.pipelineLayout, 0, 1, &(mcubes.descriptorSet), 0, 0);
        
        win->vkd->vkCmdDispatchIndirect(commandBuffer, indirect->buffer, 0); // TO CHANGE
        
        foAssert(win->vkd->vkEndCommandBuffer(commandBuffer));
        
    }
    
}





void Compute::cleanup() {
    premcubes.cleanup();
    mcubes.cleanup();
}

void Compute::reset() {
    cleanup();
    setup();
}


Compute::Compute::~Compute() {
    
    cleanup();
    
    win->vkd->vkDestroySemaphore(win->device.logical, transferSem, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyCommandPool(win->device.logical, transferPool, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyFence(win->device.logical, fence, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyCommandPool(win->device.logical, commandPool, VK_NULL_HANDLE);
    
}
