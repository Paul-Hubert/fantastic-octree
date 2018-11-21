#ifndef SYNC_H
#define SYNC_H

#include <vulkan/vulkan.hpp>
#include <vector>

class Windu;

class Sync {
public :
    Sync(Windu *win);
    ~Sync();
    void init();
    uint32_t makeSemaphore();
    vk::Semaphore getSemaphore(uint32_t i);
    uint32_t makeFence(bool signaled);
    vk::Fence getFence(uint32_t i);
    
    void step();

private :
    uint32_t frame = 0;
    uint32_t Snum = 0;
    uint32_t Fnum = 0;
    uint32_t width;
    std::vector<vk::Semaphore> semaphores;
    std::vector<vk::Fence> fences;
    Windu *win;
};

#endif
