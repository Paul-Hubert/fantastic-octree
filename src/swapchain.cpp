#include <QVulkanFunctions>
#include <iostream>
#include <vulkan/vulkan.h>

#include "windu.h"
#include "helper.h"

Swapchain::Swapchain(Windu *win) {
    this->win = win;
}

void Swapchain::init() {
    
    
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR> (win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR == nullptr) qDebug("nope");
    
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR> (win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfaceFormatsKHR"));
    if(vkGetPhysicalDeviceSurfaceFormatsKHR == nullptr) qDebug("nope");
    
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR =
    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR> (win->inst.getInstanceProcAddr("vkGetPhysicalDeviceSurfacePresentModesKHR"));
    if(vkGetPhysicalDeviceSurfaceFormatsKHR == nullptr) qDebug("nope");
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(win->device.physical, surface, &capabilities);
    
    uint32_t num;
    vkGetPhysicalDeviceSurfaceFormatsKHR(win->device.physical, surface, &num, nullptr);
    formats.resize(num);
    vkGetPhysicalDeviceSurfaceFormatsKHR(win->device.physical, surface, &num, formats.data());
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(win->device.physical, surface, &num, nullptr);
    presentModes.resize(num);
    vkGetPhysicalDeviceSurfacePresentModesKHR(win->device.physical, surface, &num, presentModes.data());
    
    
    
}

void Swapchain::reset() {
    
}

void Swapchain::getSurface() {
    surface = win->inst.surfaceForWindow(win);
    if(surface == VK_NULL_HANDLE) qDebug("hey your surface didn't work");
}

Swapchain::~Swapchain() {
    
}
