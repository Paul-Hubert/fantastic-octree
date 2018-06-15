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
    
    Windu *win;
    
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> images;
    VkFormat format;
    VkExtent2D extent;

    uint32_t NUM_FRAMES = 3;
    
private :
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &formats, VkFormat wantedFormat, VkColorSpaceKHR wantedColorSpace);
    VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> &presentModes, VkPresentModeKHR wantedMode);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR &capabilities);
};
