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

Compute::Compute(Windu *win) : mcubes(win) {
    this->win = win;
}

void Compute::preinit() {
    
    win->resman.allocateResource(FO_RESOURCE_STAGING_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_VERTEX_BUFFER, MAX_CUBES * 15 * sizeof(Vertex));
    
    win->resman.allocateResource(FO_RESOURCE_LOOKUP_BUFFER, 4096*sizeof(char));
    
    win->resman.allocateResource(FO_RESOURCE_INDIRECT_DRAW, sizeof(vk::DrawIndirectCommand));
    
    mcubes.preinit();
    
}


void Compute::init() {
    
    mcubes.init();
    
    initRest();
    
    setup();
    
    prepare(&win->sync);
}

void Compute::setup() {
    
    mcubes.setup();
    
}





bool Compute::isActive() {
    return isSubmitting;
}


void Compute::render(uint32_t i) {
    
    
    
    if(isSubmitting) {
        
        sync();
        
        std::vector<vk::Semaphore> wait(waitSemaphores);
        
        win->device.logical.waitForFences({fence}, true, 1000000000000000L);
        
        win->device.logical.resetFences({fence});
        
        win->device.compute.submit({ {waitCount, wait.data(), waitStages.data(), 1, &(computePool->buffer), signalCount, signalSemaphores.data()} }, fence);
        
        postsync();
        
        isSubmitting = false;
        
    }
    
    
}


void Compute::recorded() {
    isSubmitting = true;
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
        
        computePool = new ComputePool(win);
        
        computePool->moveToThread(&thread);
        connect(this, &Compute::record, computePool, &ComputePool::record);
        connect(computePool, &ComputePool::recorded, this, &Compute::recorded);
        connect(&thread, &QThread::finished, computePool, &QObject::deleteLater);
        connect(&thread, &QThread::finished, &thread, &QThread::deleteLater);
        thread.start();

        fence = win->device.logical.createFence({vk::FenceCreateFlagBits::eSignaled});
        
    }
    
    {
        
        transferPool = win->device.logical.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, win->device.t_i));
        
        transferCmd = (win->device.logical.allocateCommandBuffers(vk::CommandBufferAllocateInfo(transferPool, vk::CommandBufferLevel::ePrimary, 1)))[0];

        transferSem = win->device.logical.createSemaphore({});
        
    }
    
    {
        
        FoBuffer* lookup = win->resman.getBuffer(FO_RESOURCE_LOOKUP_BUFFER);
        QByteArray bytes = foLoad("src/resources/mclookup.bin");
        void* ptr = win->device.logical.mapMemory(lookup->memory, lookup->offset, lookup->size);
        memcpy(ptr, bytes.constData(), lookup->size);
        win->device.logical.unmapMemory(lookup->memory);
        
    }
    
    
    emit record(&mcubes);
    
}


ComputePool::ComputePool(Windu* win) {
    
    this->win = win;
    
    pool = win->device.logical.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, win->device.c_i));
        
    buffer = (win->device.logical.allocateCommandBuffers(vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, 1)))[0];
    
}

void ComputePool::record(MCubes* mcubes) {
    
    buffer.begin(vk::CommandBufferBeginInfo());
    
    // PREMCUBES
    buffer.bindPipeline(vk::PipelineBindPoint::eCompute, mcubes->pipeline);
    buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mcubes->pipelineLayout, 0, {mcubes->descriptorSet}, {});
    
    buffer.dispatch(CHUNK_SIZE/8, CHUNK_SIZE/8, CHUNK_SIZE/8);
    
    buffer.end();
    
    emit recorded();
    
}

ComputePool::~ComputePool() {
    
    win->device.logical.destroy(pool);
    
}



void Compute::cleanup() {
    mcubes.cleanup();
}

void Compute::reset() {
    cleanup();
    setup();
}


Compute::Compute::~Compute() {
    
    cleanup();
    
    win->device.logical.destroy(transferSem);
    
    win->device.logical.destroy(transferPool);
    
    win->device.logical.destroy(fence);
    
    thread.quit();
    thread.wait();
    
}
