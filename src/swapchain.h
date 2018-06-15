#pragma once

#include <QVulkanInstance>

class Windu;

class Swapchain {
public :
    Swapchain(Windu *win);
    void init();
    ~Swapchain();
    void getSurface();
    void reset();
    uint32_t NUM_FRAMES = 2;
    
    Windu *win;
    
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    
    VkSurfaceKHR surface;
};
