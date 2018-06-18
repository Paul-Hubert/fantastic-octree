#ifndef RENDERER_H
#define RENDERER_H

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    void init();
    
private:
    VkShaderModule createShaderFromFile(QString fileName);
};

#endif /* RENDERER_H */

