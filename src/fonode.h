#ifndef FONODE_H
#define FONODE_H

#include <vector>
#include <map>
#include <vulkan/vulkan_core.h>

#include "sync.h"

class foNode {
public :
    
    foNode* signalTo(foNode *waiter, VkPipelineStageFlags stage);
    
protected :
    
    foNode* waitOn(foNode *signaler, VkPipelineStageFlags stage);
    
    void prepare(Sync *semaphores);
    
    void sync();
    
    virtual bool isActive() {return true;};
    
    bool prepareSignal(foNode *signaler, VkSemaphore sem);
    
    std::vector<foNode*> waitNodes, signalNodes;
    std::vector<VkSemaphore> waitSemaphores, signalSemaphores;
    std::map<foNode*, VkPipelineStageFlags> waitNodeStages;
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<uint32_t> semaphoreHandles; // handles to signalSemaphores in Sync.
    uint32_t waitCount, tempWaitCount, signalCount;
    
    Sync *semaphores;
    
    
};
#endif
