#ifndef RENDERER_H
#define RENDERER_H

#include <QString>

#include "vulkan/vulkan.h"
#include "fonode.h"

class Windu;

class Renderer : public foNode {
public:
    Renderer(Windu *win);
    ~Renderer();
    
    void init();
    void setup();
    void cleanup();
    void reset();
    void render(uint32_t i);
    
    Windu *win;
    
private:
    VkShaderModule createShaderFromFile(QString fileName);
};

#endif /* RENDERER_H */

