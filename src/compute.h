#ifndef COMPUTE_H
#define COMPUTE_H

#include <QVulkanInstance>

#include "fonode.h"

class Windu;

struct Chunk;

class Compute : public foNode {
public :
    Compute(Windu *win);
    ~Compute();
    void init();
    void setup();
    void cleanup();
    void reset();
    void render(uint32_t i);
    
    virtual bool isActive() override;
    
    void* startWriteDensity();
    void finishWriteDensity();
    
    void* startWriteCubes();
    void finishWriteCubes(int num);
    
private :
    
    Windu *win;
    
    void initDescriptors();
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    
    void initPipeline();
    VkPipelineLayout pipelineLayout;
    VkShaderModule shaderModule;
    VkPipeline pipeline;
    
    void initRest();
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkFence fence;
    
    VkBufferView lookupView;
    
    VkCommandPool transferPool;
    VkCommandBuffer transferCmd;
    VkSemaphore transferSem;
    
    uint32_t t = 0;
    
};

#endif
