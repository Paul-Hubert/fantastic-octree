#include <QVulkanInstance>

class Device {
public :
    void init(QVulkanInstance &inst);
    uint32_t getScore(QVulkanInstance &inst, VkPhysicalDevice &device);
    bool getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, uint32_t *index);
    VkDevice operator ~();
    VkPhysicalDevice physicalDevice();
    
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilies;
    std::vector<VkExtensionProperties> extensions;
private :
    VkPhysicalDevice physical;
    VkDevice logical;
};
