#ifndef RAYMARCHER_H
#define RAYMARCHER_H

#include <vulkan/vulkan.hpp>
#include <QVulkanInstance>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "helper.h"

class Windu;


struct Ubo {
    glm::mat4 viewinvproj;
    glm::vec3 pos;
    float pad;
};

class Raymarcher {
public :
    Raymarcher(Windu *win);
    ~Raymarcher();
    void preinit();
    void init();
    void setup();
    void cleanup();
    void reset();
    void presubmit();
    
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

