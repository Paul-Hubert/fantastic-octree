#include "windu.h"
#include <vulkan/vulkan.h>
#include <QVulkanFunctions>
#include "helper.h"
#include <iostream>

void Swapchain::init(Windu *win) {
    
    surface = win->inst.surfaceForWindow(win);
    
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR =
    (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR =
    (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfaceFormatsKHR");
    if(vkGetPhysicalDeviceSurfaceFormatsKHR != nullptr) {
        std::cout << "efefef\n";
        uint32_t num;
        vkGetPhysicalDeviceSurfaceFormatsKHR(win->device.physical, surface, &num, nullptr);
        //std::cout << capabilities.currentExtent.width;
    }
    
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR =
    (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfacePresentModesKHR");
    
    
}

Swapchain::~Swapchain() {
    
}
