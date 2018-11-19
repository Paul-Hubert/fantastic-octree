#ifndef PREMCUBES_H
#define PREMCUBES_H

#include <QVulkanInstance>

#include "helper.h"

class Windu;

class PreMCubes {
public :
    PreMCubes(Windu *win);
    ~PreMCubes();
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
    
private :
    
    Windu *win;
    
};
#endif

