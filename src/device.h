#include <QVulkanInstance>

class Device {
public :
    void init(QVulkanInstance &inst);
    uint32_t getScore(QVulkanInstance &inst, VkPhysicalDevice &device);
    VkPhysicalDevice physical;
    VkDevice logical;
    operator VkDevice() { return logical; };
};
