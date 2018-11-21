#ifndef PREMCUBES_H
#define PREMCUBES_H

#include <vulkan/vulkan.hpp>
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

