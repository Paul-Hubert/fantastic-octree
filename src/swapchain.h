#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <QVulkanInstance>

#include "fonode.h"

class Windu;

class Swapchain : public foNode {
public :
    Swapchain(Windu *win);
    void init();
    ~Swapchain();
    void getSurface();
    void reset();
    
    uint32_t swap();
    
    Windu *win;
    
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkFormat format;
    VkExtent2D extent;

    uint32_t NUM_FRAMES = 3;
    uint32_t current = 1000;
    
private :
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &formats, VkFormat wantedFormat, VkColorSpaceKHR wantedColorSpace);
    VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> &presentModes, VkPresentModeKHR wantedMode);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR &capabilities);
    
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;
    
    uint64_t last = 0;
    double frametime = 0.0, count = 0.0;
};

#endif
