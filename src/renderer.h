#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.hpp>
#include <QString>

#include "fonode.h"

class Windu;

struct Vertex {
    float position[4];
};

class Renderer : public foNode {
public:
    Renderer(Windu *win);
    ~Renderer();
    
    void preinit();
    void init();
    void setup();
    void cleanup();
    void reset();
    void render(uint32_t i);
    
    Windu *win;
    
private:
    
    void initDescriptors();
    
    void initPipeline();
    
    void initRest();
    
    
    VkRenderPass renderPass;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    VkFence fence;
    
};

#endif /* RENDERER_H */
