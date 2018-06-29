#ifndef COMPUTE_H
#define COMPUTE_H

#include <QVulkanInstance>

#include "fonode.h"

class Windu;

class Compute : public foNode {
public :
    Compute(Windu *win);
    ~Compute();
    void init();
    void setup();
    void cleanup();
    void reset();
    void render(uint32_t i);
    
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
    
    VkFence fence;
    
    uint32_t t = 0;
};

#endif
