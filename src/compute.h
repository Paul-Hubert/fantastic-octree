#ifndef COMPUTE_H
#define COMPUTE_H

#include <vulkan/vulkan.hpp>
#include <QVulkanInstance>
#include <QThread>

#include "fonode.h"
#include "mcubes.h"

class Windu;

struct Chunk;

class Compute : public foNode {
    
public :
    Compute(Windu *win);
    ~Compute();
    void preinit();
    void init();
    void setup();
    void cleanup();
    void reset();
    void render(uint32_t i);
    
    virtual bool isActive() override;
    
    void* startWriteDensity();
    void finishWriteDensity();
    
private :
    
    Windu *win;
    MCubes mcubes;
    
    void initRest();
    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;
    vk::Fence fence;
    
    uint32_t t = 0;
    
};

#endif
