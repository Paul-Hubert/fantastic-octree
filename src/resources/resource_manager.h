#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "vulkan/vulkan.hpp"
#include <QVulkanFunctions>
#include <set>

class Windu;

struct FoBuffer {
    vk::Buffer buffer;
    vk::DeviceMemory memory;
    vk::DeviceSize size, offset;
};

struct FoImage {
    vk::Image image;
    vk::Format format;
    vk::DeviceMemory memory;
    vk::DeviceSize size, offset;
};

typedef enum FoResourceName {
    FO_RESOURCE_DENSITY_BUFFER = 0,
    FO_RESOURCE_STAGING_DENSITY_BUFFER = 1,
    FO_RESOURCE_LOOKUP_BUFFER = 2,
    FO_RESOURCE_VERTEX_BUFFER = 3,
    FO_RESOURCE_UNIFORM_BUFFER = 4,
    FO_RESOURCE_INDIRECT_DRAW = 5
} FoResourceName;

#define FO_RESOURCE_BUFFER_COUNT 6
#define FO_RESOURCE_IMAGE_COUNT 0

class ResourceManager {
public :
    void init();
    ResourceManager(Windu *windu);
    ~ResourceManager();
    
    void allocateResource(FoResourceName name, vk::DeviceSize size);
    FoBuffer* getBuffer(FoResourceName name);
    FoImage* getImage(FoResourceName name);
    
private :
    
    Windu* win;
    
    FoBuffer buffers[FO_RESOURCE_BUFFER_COUNT];
    FoImage images[FO_RESOURCE_IMAGE_COUNT];
    
    void initBuffer(FoResourceName name, std::set<uint32_t> si, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags);
    
};

#endif
