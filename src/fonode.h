#ifndef FONODE_H
#define FONODE_H

#include <vector>
#include <map>
#include <QVulkanInstance>

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
    bool prepareSignal(VkPipelineStageFlags stages, VkSemaphore sem);
    
    std::vector<foNode*> waitNodes, signalNodes;
    std::vector<VkSemaphore> waitSemaphores, signalSemaphores;
    std::map<foNode*, VkPipelineStageFlags> waitNodeStages;
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<uint32_t> semaphoreHandles; // handles to signalSemaphores in Sync.
    uint32_t waitCount, tempWaitCount = 0, signalCount;
    
    Sync *semaphores;
    
    
};
#endif
