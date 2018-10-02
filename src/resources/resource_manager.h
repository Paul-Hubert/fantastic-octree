#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QVulkanFunctions>

class Windu;

struct Buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size, offset;
};

class ResourceManager {
public :
    void init();
    ResourceManager(Windu *windu);
    ~ResourceManager();
    
    
    void allocateDensityBuffer(VkDeviceSize size);
    void allocateStagingDensityBuffer(VkDeviceSize size);
    void allocateCubesBuffer(VkDeviceSize size);
    void allocateLookupBuffer(VkDeviceSize size);
    void allocateVertexBuffer(VkDeviceSize size);
    void allocateUniformBuffer(VkDeviceSize size);
    Buffer getDensityBuffer();
    Buffer getStagingDensityBuffer();
    Buffer getCubesBuffer();
    Buffer getLookupBuffer();
    Buffer getVertexBuffer();
    Buffer getUniformBuffer();
    
private :
    
    Windu* win;
    
    Buffer density, stagingDensity, cubes, lookup, vertex, uniform;
    
};

#endif
