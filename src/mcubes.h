#ifndef MCUBES_H
#define MCUBES_H

#include <QVulkanInstance>

#include "helper.h"

class Windu;

class MCubes {
public :
    MCubes(Windu *win);
    ~MCubes();
    void preinit();
    void init();
    void setup();
    void cleanup();
    void reset();
    
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    
    VkBufferView lookupView;
    
private :
    
    Windu *win;
    
};
#endif
