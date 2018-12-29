#ifndef TRANSFER_H
#define TRANSFER_H

#include <vulkan/vulkan.hpp>
#include <QMutex>
#include <QThread>

#include "fonode.h"

class Windu;

class Transfer : public foNode {
    
public:
    Transfer(Windu* win);
    ~Transfer();
    void preinit();
    void init();
    void render(uint32_t i);
    
    void record(std::function<void(vk::CommandBuffer)> f, foNode* waiter, vk::PipelineStageFlags stage);
    
    virtual bool isActive() override;
    
private:
    Windu* win;
    uint32_t size = 2;
    uint32_t index = 0;
    uint32_t fence_num = 0;
    bool hasSubmitted = true;
    
    std::vector<vk::CommandPool> pools;
    std::vector<vk::Fence> fences;
    std::vector<vk::Semaphore> semaphores;
    
};

#endif
