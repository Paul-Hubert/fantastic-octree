#ifndef COMPUTE_H
#define COMPUTE_H

#include <QVulkanInstance>

class Windu;

class Compute {
public :
    Compute(Windu *win);
    ~Compute();
    void init();
    void setup();
    void cleanup();
    void reset();
    
    Windu *win;
    
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    
    std::vector<VkDescriptorSet> descriptorSet;
};

#endif
