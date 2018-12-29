#ifndef MCUBES_H
#define MCUBES_H

#include <vulkan/vulkan.hpp>
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
    
    void record(vk::CommandBuffer commandBuffer);
    
    vk::DescriptorPool descriptorPool;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;
    
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline pipeline;
    
    vk::BufferView lookupView;
    
private :
    
    Windu *win;
    
};
#endif

