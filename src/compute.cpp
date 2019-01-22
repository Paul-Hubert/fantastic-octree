#include "compute.h"

#include <QVulkanDeviceFunctions>
#include <iostream>
#include <math.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "windu.h"
#include "terrain.h"
#include "helper.h"

Compute::Compute(Windu *win) : pass(win) {
    this->win = win;
}

void Compute::preinit() {
    
    win->resman.allocateResource(FO_RESOURCE_STAGING_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_VERTEX_BUFFER, sizeof(Vertex) * 3 * 800 * 800); //  * MAX_CUBES * 15);
    
    win->resman.allocateResource(FO_RESOURCE_LOOKUP_BUFFER, 4096*sizeof(char));
    
    win->resman.allocateResource(FO_RESOURCE_INDIRECT_DRAW, sizeof(vk::DrawIndirectCommand));
    
    pass.preinit();
    
}


void Compute::init() {
    
    pass.init();
    
    initRest();
    
    setup();
    
    prepare(&win->sync);
}

void Compute::setup() {
    
    pass.setup();
    
}





bool Compute::isActive() {
    return true;
}


void Compute::render(uint32_t i) {
    
    sync();
        
    std::vector<vk::Semaphore> wait(waitSemaphores);
    
    win->device.logical.waitForFences({fence}, true, 1000000000000000L);
    
    win->device.logical.resetFences({fence});
    
    pass.presubmit();
    
    win->device.compute.submit({ {waitCount, wait.data(), waitStages.data(), 1, &commandBuffer, signalCount, signalSemaphores.data()} }, fence);
    
    postsync();
    
}




void* Compute::startWriteDensity() {
    
    FoBuffer* density = win->resman.getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER);
    
    return win->device.logical.mapMemory(density->memory, density->offset, density->size);
}

void Compute::finishWriteDensity() {
    
    FoBuffer* stagingDensity = win->resman.getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER);
    FoBuffer* density = win->resman.getBuffer(FO_RESOURCE_DENSITY_BUFFER);
    
    win->device.logical.unmapMemory(stagingDensity->memory);
    
    std::function<void(vk::CommandBuffer)>lambda = [&](vk::CommandBuffer buffer) {
        buffer.copyBuffer(stagingDensity->buffer, density->buffer, {vk::BufferCopy(0,0,density->size)});
    };
    
    win->transfer.record(lambda, this, vk::PipelineStageFlagBits::eComputeShader);
    
}




void Compute::initRest() {
    
    {
        
        fence = win->device.logical.createFence({vk::FenceCreateFlagBits::eSignaled});
        
    }
    
    {
        
        FoBuffer* lookup = win->resman.getBuffer(FO_RESOURCE_LOOKUP_BUFFER);
        QByteArray bytes = foLoad("src/resources/mclookup.bin");
        void* ptr = win->device.logical.mapMemory(lookup->memory, lookup->offset, lookup->size);
        memcpy(ptr, bytes.constData(), lookup->size);
        win->device.logical.unmapMemory(lookup->memory);
        
    }
    
    {
        commandPool = win->device.logical.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, win->device.c_i));
            
        commandBuffer = (win->device.logical.allocateCommandBuffers(vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1)))[0];
        
        commandBuffer.begin(vk::CommandBufferBeginInfo());
        
        pass.record(commandBuffer);
        
        commandBuffer.end();
    }
    
}


void Compute::cleanup() {
    pass.cleanup();
}

void Compute::reset() {
    cleanup();
    setup();
}


Compute::Compute::~Compute() {
    
    cleanup();
    
    win->device.logical.destroy(commandPool);
    
    win->device.logical.destroy(fence);
    
}
