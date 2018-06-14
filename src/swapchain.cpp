#include <QVulkanFunctions>
#include <iostream>

#include "windu.h"
#include "helper.h"

void Swapchain::init(Windu *win) {
    
    surface = QVulkanInstance::surfaceForWindow(win);
    if(surface == nullptr) printf("fuck you world\n");
    
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR =
    (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR =
    (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfaceFormatsKHR");
    if(vkGetPhysicalDeviceSurfaceFormatsKHR != nullptr) {
        uint32_t num;
        //vkGetPhysicalDeviceSurfaceFormatsKHR(win->device.physical, surface, &num, nullptr);
        //std::cout << capabilities.currentExtent.width;
    }
    
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR =
    (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfacePresentModesKHR");
    
    
}

Swapchain::~Swapchain() {
    
}
