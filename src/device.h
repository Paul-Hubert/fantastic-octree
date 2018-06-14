#include <QVulkanInstance>

class Device {
public :
    void init(QVulkanInstance &inst);
    ~Device();
    uint32_t getScore(QVulkanInstance &inst, VkPhysicalDevice &device);
    bool getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, uint32_t *index);
    
    VkPhysicalDevice physical;
    VkDevice logical;
    VkQueue graphics;
    VkQueue compute;
    VkQueue transfer;
    QVulkanInstance *instance;
    
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilies;
    std::vector<VkExtensionProperties> extensions;
};
