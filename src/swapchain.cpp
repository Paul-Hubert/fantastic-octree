#include <QVulkanFunctions>
#include <iostream>
#include <vulkan/vulkan.h>

#include "windu.h"
#include "helper.h"
#include "loader.inl"

Swapchain::Swapchain(Windu *win) {
    this->win = win;
}

void Swapchain::init() {
    
    INST_LOAD(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
    INST_LOAD(vkGetPhysicalDeviceSurfaceFormatsKHR)
    INST_LOAD(vkGetPhysicalDeviceSurfacePresentModesKHR)
    
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(win->device.physical, surface, &capabilities);
    
    uint32_t num;
    vkGetPhysicalDeviceSurfaceFormatsKHR(win->device.physical, surface, &num, nullptr);
    formats.resize(num);
    vkGetPhysicalDeviceSurfaceFormatsKHR(win->device.physical, surface, &num, formats.data());
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(win->device.physical, surface, &num, nullptr);
    presentModes.resize(num);
    vkGetPhysicalDeviceSurfacePresentModesKHR(win->device.physical, surface, &num, presentModes.data());
    
    VkSurfaceFormatKHR surfaceformat = chooseSwapSurfaceFormat(formats, VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes, VK_PRESENT_MODE_FIFO_KHR);
    extent = chooseSwapExtent(capabilities);
    format = surfaceformat.format;
    
    NUM_FRAMES = std::max(capabilities.minImageCount, NUM_FRAMES);
    if (capabilities.maxImageCount > 0 && NUM_FRAMES > capabilities.maxImageCount) {
        NUM_FRAMES = capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = NUM_FRAMES;
    createInfo.imageFormat = surfaceformat.format;
    createInfo.imageColorSpace = surfaceformat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = (win->device.g_i != win->device.c_i) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    if(capabilities.supportedUsageFlags & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) createInfo.imageUsage |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    else qDebug("can't write to swapchain");
    qInfo() << createInfo.flags << endl;
    
    createInfo.queueFamilyIndexCount = 1 + win->device.g_i == win->device.c_i;
    uint32_t qi[createInfo.queueFamilyIndexCount];
    qi[0] = win->device.g_i;
    if(win->device.g_i == win->device.c_i) qi[1] = win->device.c_i;
    createInfo.pQueueFamilyIndices = qi;
    
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = swapchain;
    
    VkSwapchainKHR newSwapchain;
    
    DEV_LOAD(vkCreateSwapchainKHR)
    
    foAssert(vkCreateSwapchainKHR(win->device.logical, &createInfo, nullptr, &newSwapchain));
    
    swapchain = newSwapchain;
    
    if(createInfo.oldSwapchain != VK_NULL_HANDLE) {
        DEV_LOAD(vkDestroySwapchainKHR)
        vkDestroySwapchainKHR(win->device.logical, createInfo.oldSwapchain, nullptr);
        for (auto imageView : imageViews) {
            win->vkd->vkDestroyImageView(win->device.logical, imageView, nullptr);
        }
    }
    
    DEV_LOAD(vkGetSwapchainImagesKHR)
    
    vkGetSwapchainImagesKHR(win->device.logical, swapchain, &num, nullptr);
    images.resize(num);
    foAssert(vkGetSwapchainImagesKHR(win->device.logical, swapchain, &num, images.data()));
    
    imageViews.resize(num);
    for(uint32_t i = 0; i < num; i++) {
        
        VkImageViewCreateInfo vInfo = {};
        vInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vInfo.image = images[i];
        vInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vInfo.format = format;
        
        vInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        vInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        vInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        vInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        
        vInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vInfo.subresourceRange.baseMipLevel = 0;
        vInfo.subresourceRange.levelCount = 1;
        vInfo.subresourceRange.baseArrayLayer = 0;
        vInfo.subresourceRange.layerCount = 1;
        
        foAssert(win->vkd->vkCreateImageView(win->device.logical, &vInfo, nullptr, &imageViews[i]));
        
    }
    
}

void Swapchain::reset() {
    
    for (auto imageView : imageViews) {
        win->vkd->vkDestroyImageView(win->device.logical, imageView, nullptr);
    }
    
    DEV_LOAD(vkDestroySwapchainKHR)
    vkDestroySwapchainKHR(win->device.logical, swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;
    
}

void Swapchain::getSurface() {
    surface = win->inst.surfaceForWindow(win);
    if(surface == VK_NULL_HANDLE) qDebug("hey your surface didn't work");
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &formats, VkFormat wantedFormat, VkColorSpaceKHR wantedColorSpace) {
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        return {wantedFormat, wantedColorSpace}; // Just give the format you want
    }

    for (const auto& availableFormat : formats) {
        if (availableFormat.format == wantedFormat && availableFormat.colorSpace == wantedColorSpace) { // Look for the wanted format
            return availableFormat;
        }
    }

    return formats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(std::vector<VkPresentModeKHR> &presentModes, VkPresentModeKHR wantedMode) {

    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == wantedMode) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {(uint32_t) win->size.rwidth(), (uint32_t) win->size.rheight()};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

Swapchain::~Swapchain() {
    
}
