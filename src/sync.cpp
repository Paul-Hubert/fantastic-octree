#include "sync.h"

#include <iostream>

#include "helper.h"
#include "windu.h"

Sync::Sync(Windu *win) {
    this->win = win;
}

void Sync::init() {
    width = win->swap.NUM_FRAMES;
    
    semaphores.resize(Snum * width);
    
    for(uint32_t i = 0; i < Snum; i++) {
        for(uint32_t j = 0; j < width; j++) {
            semaphores[i*width+j] = win->device.logical.createSemaphore({});
        }
    }
}

uint32_t Sync::makeSemaphore() {
    Snum++;
    if(semaphores.size() > 0) {
        std::cout << "Runtime creation of semaphores\n";
        semaphores.resize(Snum*width);
        
        for(uint32_t i = 0; i < width; i++) {
            semaphores[(Snum-1)*width+i] = win->device.logical.createSemaphore({});
        }
    }
    return Snum - 1;
}

uint32_t Sync::makeFence(bool signaled) {
    Fnum++;
    
    width = win->swap.NUM_FRAMES;
    
    fences.resize(Fnum*width);
    
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk::FenceCreateFlags flags = signaled ? static_cast<vk::FenceCreateFlags>(0) : vk::FenceCreateFlagBits::eSignaled;
    for(uint32_t i = 0; i < width; i++) {
        fences[(Fnum-1)*width+i] = win->device.logical.createFence({flags});
    }
    
    return Fnum - 1;
}


vk::Semaphore Sync::getSemaphore(uint32_t i)
{
    return semaphores[i*width+frame];
}

vk::Fence Sync::getFence(uint32_t i) {
    return fences[i*width+frame];
}

void Sync::step() {
    frame = (frame + 1)%width;
}

Sync::~Sync() {
    for(uint32_t i = 0; i < Snum; i++) {
        for(uint32_t j = 0; j < width; j++) {
            win->device.logical.destroy(semaphores[i*width+j]);
        }
    }
    for(uint32_t i = 0; i < Fnum; i++) {
        for(uint32_t j = 0; j < width; j++) {
            win->device.logical.destroy(fences[i*width+j]);
        }
    }
}
