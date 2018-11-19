#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QVulkanFunctions>

class Windu;

struct FoBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size, offset;
};

struct FoImage {
    VkImage image;
    VkFormat format;
    VkDeviceMemory memory;
    VkDeviceSize size, offset;
};

typedef enum FoResourceName {
    FO_RESOURCE_DENSITY_BUFFER = 0,
    FO_RESOURCE_STAGING_DENSITY_BUFFER = 1,
    FO_RESOURCE_CUBES_BUFFER = 2,
    FO_RESOURCE_LOOKUP_BUFFER = 3,
    FO_RESOURCE_VERTEX_BUFFER = 4,
    FO_RESOURCE_UNIFORM_BUFFER = 5,
    FO_RESOURCE_INDIRECT_DISPATCH = 6,
    FO_RESOURCE_INDIRECT_DRAW = 7
} FoResourceName;

#define FO_RESOURCE_BUFFER_COUNT 8
#define FO_RESOURCE_IMAGE_COUNT 0

class ResourceManager {
public :
    void init();
    ResourceManager(Windu *windu);
    ~ResourceManager();
    
    void allocateResource(FoResourceName name, VkDeviceSize size);
    FoBuffer* getBuffer(FoResourceName name);
    FoImage* getImage(FoResourceName name);
    
private :
    
    Windu* win;
    
    FoBuffer buffers[FO_RESOURCE_BUFFER_COUNT];
    FoImage images[FO_RESOURCE_IMAGE_COUNT];
    
};

#endif
