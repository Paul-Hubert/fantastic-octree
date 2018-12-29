#ifndef COMPUTE_H
#define COMPUTE_H

#include <vulkan/vulkan.hpp>
#include <QVulkanInstance>
#include <QThread>

#include "fonode.h"
#include "mcubes.h"

class Windu;

class ComputePool : public QObject {
    
    Q_OBJECT
    
public:
    ComputePool(Windu* win);
    ~ComputePool();
    
    vk::CommandPool pool;
    vk::CommandBuffer buffer;

private:
    Windu* win;
    
public slots:
    void record(MCubes* cubes);

signals:
    void recorded();

};

struct Chunk;

class Compute : public QObject, public foNode {
    Q_OBJECT
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
    
public slots:
    void recorded();

signals:
    void record(MCubes* mcubes);
    
private :
    
    Windu* win;
    
    MCubes mcubes;
    
    void initRest();
    QThread thread;
    ComputePool* computePool;
    
    vk::Fence fence;
    
    vk::CommandPool transferPool;
    vk::CommandBuffer transferCmd;
    vk::Semaphore transferSem;
    
    bool isSubmitting = false;
    
};

#endif
