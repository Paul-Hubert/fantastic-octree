#include <iostream>

#include "helper.h"
#include "sync.h"
#include "windu.h"

Sync::Sync(Windu *win) {
    this->win = win;
}

void Sync::init() {
    width = win->swap.NUM_FRAMES;
    
    semaphores.resize(Snum * width);
    
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for(uint32_t i = 0; i < Snum; i++) {
        for(uint32_t j = 0; j < width; j++) {
            foAssert(win->vkd->vkCreateSemaphore(win->device.logical, &info, nullptr, &semaphores[i*width+j]));
        }
    }
}

uint32_t Sync::makeSemaphore() {
    Snum++;
    if(semaphores.size() > 0) {
        std::cout << "Runtime creation of semaphores\n";
        semaphores.resize(Snum*width);
        
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for(uint32_t i = 0; i < width; i++) {
            win->vkd->vkCreateSemaphore(win->device.logical, &info, nullptr, &semaphores[(Snum-1)*width+i]);
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
    info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    for(uint32_t i = 0; i < width; i++) {
        win->vkd->vkCreateFence(win->device.logical, &info, nullptr, &fences[(Fnum-1)*width+i]);
    }
    
    return Fnum - 1;
}


VkSemaphore Sync::getSemaphore(uint32_t i) {
    return semaphores[i*width+frame];
}

VkFence Sync::getFence(uint32_t i) {
    return fences[i*width+frame];
}

void Sync::step() {
    frame = (frame + 1)%width;
}

Sync::~Sync() {
    for(uint32_t i = 0; i < Snum; i++) {
        for(uint32_t j = 0; j < width; j++) {
            win->vkd->vkDestroySemaphore(win->device.logical, semaphores[i*width+j], nullptr);
        }
    }
    for(uint32_t i = 0; i < Fnum; i++) {
        for(uint32_t j = 0; j < width; j++) {
            win->vkd->vkDestroyFence(win->device.logical, fences[i*width+j], nullptr);
        }
    }
}
