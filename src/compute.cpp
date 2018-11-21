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

Compute::Compute(Windu *win) : premcubes(win), mcubes(win) {
    this->win = win;
}

void Compute::preinit() {
    
    win->resman.allocateResource(FO_RESOURCE_CUBES_BUFFER, MAX_CUBES * sizeof(Cube));
    
    win->resman.allocateResource(FO_RESOURCE_STAGING_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_VERTEX_BUFFER, MAX_CUBES * 15 * sizeof(Vertex));
    
    win->resman.allocateResource(FO_RESOURCE_LOOKUP_BUFFER, 4096*sizeof(char));
    
    win->resman.allocateResource(FO_RESOURCE_INDIRECT_DISPATCH, sizeof(vk::DispatchIndirectCommand));
    
    win->resman.allocateResource(FO_RESOURCE_INDIRECT_DRAW, sizeof(vk::DrawIndirectCommand));
    
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
    
    std::vector<vk::Semaphore> wait(waitSemaphores);
    
    win->device.logical.waitForFences({fence}, true, 1000000000000000L);
    
    win->device.logical.resetFences({fence});
    
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
    
    transferCmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    
    transferCmd.copyBuffer(stagingDensity->buffer, density->buffer, {vk::BufferCopy(0,0,density->size)});
    
    transferCmd.end();
    
    this->prepareSignal(vk::PipelineStageFlagBits::eComputeShader, transferSem);
    
    win->device.transfer.submit({{0, nullptr, nullptr, 1, &transferCmd, 1, &transferSem}}, nullptr);
    
}

void * Compute::startWriteCubes() {
    
    FoBuffer* cubes = win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER);
    
    return win->device.logical.mapMemory(cubes->memory, cubes->offset, cubes->size);
    
}

void Compute::finishWriteCubes(int num) {
    
    std::cout << num << std::endl;
    
    win->device.logical.unmapMemory(win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER)->memory);
    
}




void Compute::initRest() {
    
    {
        
        commandPool = win->device.logical.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, win->device.c_i));
        
        commandBuffer = (win->device.logical.allocateCommandBuffers(vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1)))[0];

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
    
    {
        
        FoBuffer* cubes = win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER);
        FoBuffer* indirect = win->resman.getBuffer(FO_RESOURCE_INDIRECT_DISPATCH);
        
        commandBuffer.begin(vk::CommandBufferBeginInfo());
        
        // PREMCUBES
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, premcubes.pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, premcubes.pipelineLayout, 0, {premcubes.descriptorSet}, {});
        
        commandBuffer.dispatch(CHUNK_SIZE/8, CHUNK_SIZE/8, CHUNK_SIZE/8);
        
        /*
        // CUBES MEMORY BARRIER
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eAllCommands, {}, {},
                            {vk::BufferMemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead, win->device.c_i, win->device.c_i, cubes->buffer, 0, cubes->size),
                             vk::BufferMemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eIndirectCommandRead, win->device.c_i, win->device.c_i, indirect->buffer, 0, indirect->size)
                            }, {});
        
        // MCUBES
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mcubes.pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mcubes.pipelineLayout, 0, {mcubes.descriptorSet}, {});
        
        
        commandBuffer.dispatchIndirect(indirect->buffer, 0);
        */
        
        commandBuffer.end();
        
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
    
    win->device.logical.destroy(transferSem);
    
    win->device.logical.destroy(transferPool);
    
    win->device.logical.destroy(fence);
    
    win->device.logical.destroy(commandPool);
    
}
