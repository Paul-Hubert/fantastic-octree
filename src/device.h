#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.hpp>
#include <QVulkanInstance>

class Windu;

class Device {
public :
    Device(Windu *win);
    void init();
    ~Device();
    uint32_t getScore(vk::PhysicalDevice &device);
    bool getMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties, uint32_t *index);
    
    Windu *win;
    
    vk::PhysicalDevice physical;
    vk::Device logical;
    vk::Queue graphics, compute, transfer;
    uint32_t g_i = 0, c_i = 0, t_i = 0;
    
    vk::PhysicalDeviceFeatures requiredFeatures;
    std::vector<const char*> requiredExtensions;
    
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    vk::PhysicalDeviceMemoryProperties memoryProperties;
    std::vector<vk::QueueFamilyProperties> queueFamilies;
    std::vector<vk::ExtensionProperties> extensions;
};

#endif
