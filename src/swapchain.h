#pragma once
#include <vulkan/vulkan.h>
class Windu;

class Swapchain {
public :
    void init(Windu *win);
    ~Swapchain();
    uint32_t NUM_FRAMES = 2;
    
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    
    VkSurfaceKHR surface;
};
