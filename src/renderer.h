#ifndef RENDERER_H
#define RENDERER_H

class Windu;

class Renderer {
public:
    Renderer(Windu *win);
    ~Renderer();
    
    void init();
    void setup();
    void cleanup();
    void reset();
    
    Windu *win;
    
private:
    VkShaderModule createShaderFromFile(QString fileName);
};

#endif /* RENDERER_H */

