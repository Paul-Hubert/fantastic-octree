#ifndef SYNC_H
#define SYNC_H

#include <vector>

#include "vulkan/vulkan_core.h"

class Windu;

class Sync {
public :
    Sync(Windu *win);
    ~Sync();
    void init();
    uint32_t makeSemaphore();
    VkSemaphore getSemaphore(uint32_t i);
    
    void step();
    
private :
    uint32_t frame = 0;
    uint32_t num = 0;
    uint32_t width;
    std::vector<VkSemaphore> semaphores;
    Windu *win;
};

#endif
