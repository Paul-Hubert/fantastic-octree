#ifndef FONODE_H
#define FONODE_H

#include <vulkan/vulkan.hpp>
#include <vector>
#include <map>
#include <QVulkanInstance>

#include "sync.h"

class foNode {
public :
    
    foNode* signalTo(foNode *waiter, vk::PipelineStageFlags stage);
    
protected :
    
    foNode* waitOn(foNode *signaler, vk::PipelineStageFlags stage);
    
    void prepare(Sync *semaphores);
    
    void sync();
    void postsync();
    
    virtual bool isActive() {return true;};
    
    bool prepareSignal(foNode *signaler, vk::Semaphore sem);
    bool prepareSignal(vk::PipelineStageFlags stages, vk::Semaphore sem);
    
    std::vector<foNode*> waitNodes, signalNodes;
    std::vector<vk::Semaphore> waitSemaphores, signalSemaphores;
    std::map<foNode*, vk::PipelineStageFlags> waitNodeStages;
    std::vector<vk::PipelineStageFlags> waitStages;
    std::vector<uint32_t> semaphoreHandles; // handles to signalSemaphores in Sync.
    uint32_t waitCount, tempWaitCount = 0, signalCount;
    
    Sync *semaphores;
    
    
};
#endif
