#include <iterator>
#include <algorithm>
#include <iostream>

#include "fonode.h"

foNode* foNode::waitOn(foNode *signaler, VkPipelineStageFlags stage) {
    waitNodes.push_back(signaler);
    waitNodeStages[signaler] = stage;
    return this;
}

foNode* foNode::signalTo(foNode *waiter, VkPipelineStageFlags stage) {
    signalNodes.push_back(waiter);
    waiter->waitOn(this, stage);
    return this;
}

void foNode::prepare(Sync *semaphores) {
    this->semaphores = semaphores;
    waitSemaphores.resize(waitNodes.size());
    waitStages.resize(waitNodes.size());
    signalSemaphores.resize(signalNodes.size());
    semaphoreHandles.resize(signalNodes.size());
    for(uint32_t i = 0; i < signalNodes.size(); i++) {
        semaphoreHandles[i] = semaphores->makeSemaphore();
    }
    postsync();
}

void foNode::sync() {
    signalCount = 0;
    
    waitCount = tempWaitCount;
    tempWaitCount = 0;
    
    for(uint32_t i = 0; i < signalNodes.size(); i++) {
        VkSemaphore sem = semaphores->getSemaphore(semaphoreHandles[i]);
        if(signalNodes[i]->prepareSignal(this, sem)) {
            signalCount++;
            signalSemaphores.push_back(sem);
        }
    }
    
}

void foNode::postsync() {
    waitSemaphores.clear();
    waitStages.clear();
    signalSemaphores.clear();
}
bool foNode::prepareSignal(foNode *signaler, VkSemaphore sem) {
    if(isActive()) {
        tempWaitCount++;
        waitSemaphores.push_back(sem);
        waitStages.push_back(waitNodeStages[signaler]);
        return true;
    } else {
        return false;
    }
}

bool foNode::prepareSignal(VkPipelineStageFlags stages, VkSemaphore sem) {
    if(isActive()) {
        tempWaitCount++;
        waitSemaphores.push_back(sem);
        waitStages.push_back(stages);
        return true;
    } else {
        return false;
    }
}
