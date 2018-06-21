#include <iostream>

#include "helper.h"
#include "sync.h"
#include "windu.h"

Sync::Sync(Windu *win) {
    this->win = win;
}

void Sync::init() {
    width = win->swap.NUM_FRAMES;
    
    semaphores.resize(num * width);
    
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for(uint32_t i = 0; i < num; i++) {
        for(uint32_t j = 0; j < width; j++) {
            foAssert(win->vkd->vkCreateSemaphore(win->device.logical, &info, nullptr, &semaphores[i*width+j]));
        }
    }
}

uint32_t Sync::makeSemaphore() {
    num++;
    if(semaphores.size() > 0) {
        std::cout << "Runtime creation of semaphores\n";
        semaphores.resize(num);
        
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for(uint32_t i = 0; i < width; i++) {
            win->vkd->vkCreateSemaphore(win->device.logical, &info, nullptr, &semaphores[(num-1)*width+i]);
        }
    }
    return num - 1;
}

VkSemaphore Sync::getSemaphore(uint32_t i) {
    return semaphores[i*width+frame];
}

void Sync::step() {
    frame = (frame + 1)%width;
}

Sync::~Sync() {
    for(uint32_t i = 0; i < num; i++) {
        for(uint32_t j = 0; j < width; j++) {
            win->vkd->vkDestroySemaphore(win->device.logical, semaphores[i*width+j], nullptr);
        }
    }
}
