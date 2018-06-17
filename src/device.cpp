#include <QVulkanFunctions>
#include <iostream>
#include <set>
#include <string>
#include <QMessageLogger>

#include "device.h"
#include "windu.h"
#include "helper.h"

Device::Device(Windu *win) {
    this->win = win;
}

void Device::init() {
    
    QVulkanInstance *inst = &(win->inst);
    
    requiredFeatures = {};
    requiredFeatures.geometryShader = true;
    // HERE : enable needed features (if present in 'features')
    
    requiredExtensions = {};
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    // HERE : enable needed extensions (if present in 'extensions')
    
    QVulkanFunctions *vk = inst->functions();
    
    uint32_t num;
    vk->vkEnumeratePhysicalDevices(inst->vkInstance(), &num, nullptr);
    
    if(num <= 0) {
        qCritical << "No vulkan enabled device found" << endl;
        exit(6);
    }
    
    std::vector<VkPhysicalDevice> p_devices(num);
    vkAssert(vk->vkEnumeratePhysicalDevices(inst->vkInstance(), &num, p_devices.data())); // Retrieve list of available physical devices
    
    // Rate each device and pick the first best in the list, if its score is > 0
    uint32_t index = 1000, max = 0;
    for(uint32_t i = 0; i<num; i++) {
        uint32_t score = getScore(inst, p_devices[i]);
        if(score > max) { // Takes only a score higher than the last (implicitely higher than 0)
            max = score;
            index = i;
        }
    }
    
    if(index == 1000) {  // if no suitable device is found just take down the whole place
        qCritical() << "No suitable vulkan device found. Please check your driver and hardware." << endl;
        exit(1);
    }
    
    physical =  p_devices[index]; // Found physical device
    
    
    // Get device properties
    vk->vkGetPhysicalDeviceProperties(physical, &properties);
    
    vk->vkGetPhysicalDeviceFeatures(physical, &features);
    
    vk->vkGetPhysicalDeviceMemoryProperties(physical, &memoryProperties);
    
    uint32_t count;
    vk->vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, nullptr);
    queueFamilies.resize(count);
    vk->vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, queueFamilies.data());
    
    vk->vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr);
    extensions.resize(count);
    vk->vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, extensions.data());
    
    
    
    // Prepare queue choice data : GRAPHICS / COMPUTE / TRANSFER
    uint32_t g_j = 0, c_j = 0, t_j = 0, countF = 0;
    
    std::vector<float> priorities(3); priorities[0] = 0.0f; priorities[1] = 0.0f; priorities[2] = 0.0f; 
    
    std::vector<VkDeviceQueueCreateInfo> pqinfo(3); // Number of queues
    
    
    // Gets the first available queue family that supports graphics and presentation
    g_i = 1000;
    for(int i = 0; i < static_cast<int>(queueFamilies.size()); i++) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && inst->supportsPresent(physical, i, win)) {
            g_i = i;
            countF++;
            VkDeviceQueueCreateInfo qinfo = {};
            qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qinfo.queueFamilyIndex = i;
            qinfo.queueCount = 1;
            qinfo.pQueuePriorities = priorities.data();
            pqinfo[0] = qinfo;
            break;
        }
    }
    
    if(g_i == 1000) {
        qCritical << "Could not retrieve queue family" << endl;
        exit(3);
    }
    
    // Gets a compute queue family different from graphics family if possible, then different queue index if possible, else just the same queue.
    c_i = 1000;
    for(int i = 0; i < static_cast<int>(queueFamilies.size()); i++) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            c_i = i; c_j = 0;
            if(c_i != g_i) {
                countF++;
                VkDeviceQueueCreateInfo qinfo = {};
                qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                qinfo.queueFamilyIndex = i;
                qinfo.queueCount = 1;
                qinfo.pQueuePriorities = priorities.data();
                pqinfo[1] = qinfo;
                break;
            }
            while(c_j == g_j && queueFamilies[i].queueCount > c_j + 1) c_j++;
        }
    }
    
    
    if(c_i == 1000) {
        qCritical << "could not get compute queue" << endl;
        exit(4);
    }
    
    
    if(c_i == g_i && c_j != g_j) pqinfo[0].queueCount++; // If the same queue family but different queue index, create one more queue from the queue family.
    
    
    // Gets a transfer queue family different from graphics and compute family if possible, then different queue index if possible, else just the same queue.
    t_i = 1000;
    for(int i = 0; i < static_cast<int>(queueFamilies.size()); i++) { 
        t_i = i; t_j = 0;
        if(t_i != g_i && t_i != c_i) {
            countF++;
            VkDeviceQueueCreateInfo qinfo = {};
            qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qinfo.queueFamilyIndex = i;
            qinfo.queueCount = 1;
            qinfo.pQueuePriorities = priorities.data();
            pqinfo[2] = qinfo;
            break;
        }
        while(((t_i == g_i && t_j == g_j) || (t_i == c_i && t_j == c_j)) && queueFamilies[i].queueCount > t_j + 1) t_j++;
    }
    
    if(t_i == 1000) {
        qCritical() << "Could not get transfer queue family" << endl;
        exit(5);
    }
    
    if(t_i == g_i && t_j != g_j) {pqinfo[0].queueCount++;}
    else if(c_i == t_i && c_j != t_j) {pqinfo[1].queueCount++;}
    
    
    for(const auto &ext : extensions) {
        if(strcmp(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, ext.extensionName) == 0) requiredExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
    
    
    // Create Device
    
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = countF; // Number of queue families involved.
    deviceInfo.pQueueCreateInfos = pqinfo.data(); // Number of queues per queue family (and other data)
    deviceInfo.pEnabledFeatures = &requiredFeatures;
    deviceInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    deviceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    
    vkAssert(vk->vkCreateDevice(physical, &deviceInfo, nullptr, &logical)); // Create logical device
    
    QVulkanDeviceFunctions *vkd = inst->deviceFunctions(logical);
    vkd->vkGetDeviceQueue(logical, g_i, g_j, &graphics);
    
    if(c_i == g_i && c_j == g_j) {
        compute = graphics;
    } else {
        vkd->vkGetDeviceQueue(logical, c_i, c_j, &compute);
    }
    
    if(t_i == g_i && t_j == g_j) {
        transfer = graphics;
    } else if(t_i == c_i && t_j ==c_j) {
        transfer = compute;
    } else {
        vkd->vkGetDeviceQueue(logical, t_i, t_j, &transfer);
    }
    
}

    


uint32_t Device::getScore(QVulkanInstance *inst, VkPhysicalDevice &device) {
    // Get device properties
    
    uint32_t score = 1;
    inst->functions()->vkGetPhysicalDeviceProperties(device, &properties);
    inst->functions()->vkGetPhysicalDeviceFeatures(device, &features);
    uint32_t numberOfQueueFamilies;
    inst->functions()->vkGetPhysicalDeviceQueueFamilyProperties(device, &numberOfQueueFamilies, nullptr);
    uint32_t count;
    inst->functions()->vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    extensions.resize(count);
    inst->functions()->vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
    
    if(features.geometryShader) score++; // supports geometry shaders
    if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score++; // is a dedicated graphics card
    if(numberOfQueueFamilies > 1) score ++; // has more than one queue family
    
    std::set<std::string> required(requiredExtensions.begin(), requiredExtensions.end());
    for(const auto &ext : extensions) {
        required.erase(ext.extensionName);
    }
    
    if(!required.empty()) score = 0;
    if(numberOfQueueFamilies < 1) score = 0; // eliminatory
    std::cout << properties.deviceName << " : " << score << "\n";
    return score;
}

bool Device::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, uint32_t *index) {
    for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if((typeBits & 1) == 1) {
            if((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                *index = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    return true;
}

Device::~Device() {
    win->inst.deviceFunctions(logical)->vkDestroyDevice(logical, nullptr);
}
