#ifndef DEVICE_H
#define DEVICE_H

#include <QVulkanInstance>

class Windu;

class Device {
public :
    Device(Windu *win);
    void init();
    ~Device();
    uint32_t getScore(QVulkanInstance *inst, VkPhysicalDevice &device);
    bool getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, uint32_t *index);
    
    Windu *win;
    
    VkPhysicalDevice physical;
    VkDevice logical;
    VkQueue graphics;
    VkQueue compute;
    VkQueue transfer;
    
    VkPhysicalDeviceFeatures requiredFeatures;
    std::vector<const char*> requiredExtensions;
    
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilies;
    std::vector<VkExtensionProperties> extensions;
};

#endif