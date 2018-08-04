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
    
    void* allocate(int size);
    void upload(int offset, int size);
    void deallocate();
    
    Windu *win;
    
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkShaderModule shaderModule;
    std::vector<VkDescriptorSet> descriptorSet;
    VkPipeline pipeline;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkDeviceSize size = 0;
    VkDeviceMemory memory;
    VkBuffer ubo;
    VkDeviceMemory uniformMemory;
    
    VkBuffer octreeBuffer, octreeHostBuffer;
    VkDeviceMemory octreeMemory, octreeHostMemory;
    
    VkCommandPool transferPool;
    
    VkSemaphore transferSem;
    
    VkFence fence;
    
    uint32_t t = 0;
};

#endif
