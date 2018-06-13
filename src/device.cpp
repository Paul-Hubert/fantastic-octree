#include "device.h"
#include <QVulkanFunctions>

void Device::init(QVulkanInstance &inst) {
    
    QVulkanFunctions *vk = inst.functions();
    
    uint32_t num;
    vk->vkEnumeratePhysicalDevices(inst.vkInstance(), &num, nullptr);
    assert(num > 0);
    std::vector<VkPhysicalDevice> p_devices(num);
    vk->vkEnumeratePhysicalDevices(inst.vkInstance(), &num, p_devices.data());
    
    uint32_t index = 1000, max = 0;
    for(uint32_t i = 0; i<num; i++) {
        uint32_t score = getScore(inst, p_devices[i]);
        if(score > max) {
            max = score;
            index = i;
        }
    }
    
    assert(index != 1000);
    physical =  p_devices[index]; // Found physical device
    
    VkPhysicalDeviceProperties properties;
    vk->vkGetPhysicalDeviceProperties(physical, &properties);
    
    VkPhysicalDeviceFeatures features;
    vk->vkGetPhysicalDeviceFeatures(physical, &features);
    
    uint32_t count;
    vk->vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vk->vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, queueFamilies.data());
    
    vk->vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr);
    
    std::vector<VkExtensionProperties> extensions(count);
    vk->vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, extensions.data());
    
    
}

uint32_t Device::getScore(QVulkanInstance &inst, VkPhysicalDevice &device) {
    return 1;
}
