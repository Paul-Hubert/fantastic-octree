#include "transfer.h"

#include <QMutexLocker>

#include "windu.h"


Transfer::Transfer(Windu* win) {
    this->win = win;
}

void Transfer::preinit() {
    
    pools.resize(size);
    for(uint32_t i = 0; i<pools.size(); i++) {
        pools[i] = win->device.logical.createCommandPool({vk::CommandPoolCreateFlagBits::eTransient});
    }
    
}

void Transfer::init() {
    
    prepare(&win->sync);
    
}


bool Transfer::isActive() {
    return true;
}

void Transfer::render(uint32_t i) {
    
    sync();
    
    postsync();
    reset();
    
    if(fence_num > 0) {
        win->device.logical.waitForFences(fence_num, fences.data(), true, 10000000L);
        fence_num = 0;
        win->device.logical.resetCommandPool(pools[index], {});
        index = (index + 1)%size;
    }
    
}


void Transfer::record(std::function<void(vk::CommandBuffer)> f, foNode* waiter, vk::PipelineStageFlags stage) {
    vk::CommandBuffer buffer = (win->device.logical.allocateCommandBuffers(vk::CommandBufferAllocateInfo(pools[index], vk::CommandBufferLevel::ePrimary, 1)))[0];
    fence_num++;
    if(fence_num > fences.size()) {
        fences.push_back(win->device.logical.createFence({}));
        semaphores.push_back(win->device.logical.createSemaphore({}));
    }
    vk::Fence fence = fences[fence_num-1];
    buffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    f(buffer);
    buffer.end();
    
    waiter->prepareSignal(stage, semaphores[fence_num-1]);
    
    win->device.transfer.submit({vk::SubmitInfo(0, nullptr, nullptr, 1, &buffer, 1, &semaphores[fence_num-1])}, fence);
    
}


Transfer::~Transfer() {
    for(uint32_t i = 0; i<pools.size(); i++) {
        win->device.logical.destroy(pools[i]);
    }
    for(uint32_t i = 0; i<fences.size(); i++) {
        win->device.logical.destroy(fences[i]);
    }
    for(uint32_t i = 0; i<semaphores.size(); i++) {
        win->device.logical.destroy(semaphores[i]);
    }
}
