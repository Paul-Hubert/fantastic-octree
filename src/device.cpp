#include "device.h"

#include <QVulkanFunctions>
#include <iostream>
#include <set>
#include <string>
#include <QMessageLogger>

#include "windu.h"
#include "helper.h"
#include "loader.inl"

Device::Device(Windu *win) {
    this->win = win;
}

void Device::init() {
    
    QVulkanInstance *inst = &(win->inst);
    
    requiredFeatures = vk::PhysicalDeviceFeatures();
    requiredFeatures.geometryShader = true;
    // HERE : enable needed features (if present in 'features')
    
    requiredExtensions = {};
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    // HERE : enable needed extensions (if present in 'extensions')
    
    
    auto instance = static_cast<vk::Instance>(inst->vkInstance());
    std::vector<vk::PhysicalDevice> p_devices = instance.enumeratePhysicalDevices();
    
    // Rate each device and pick the first best in the list, if its score is > 0
    uint32_t index = 1000, max = 0;
    for(uint32_t i = 0; i<p_devices.size(); i++) {
        uint32_t score = getScore(p_devices[i]);
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
    properties = physical.getProperties();
    features = physical.getFeatures();
    memoryProperties = physical.getMemoryProperties();
    
    queueFamilies = physical.getQueueFamilyProperties();
    extensions = physical.enumerateDeviceExtensionProperties();
    
    // Prepare queue choice data : GRAPHICS / COMPUTE / TRANSFER
    uint32_t g_j = 0, c_j = 0, t_j = 0, countF = 0;
    
    std::vector<float> priorities(3); priorities[0] = 0.0f; priorities[1] = 0.0f; priorities[2] = 0.0f; 
    
    std::vector<vk::DeviceQueueCreateInfo> pqinfo(3); // Number of queues
    
    
    // Gets the first available queue family that supports graphics and presentation
    g_i = 1000;
    for(uint32_t i = 0; i < queueFamilies.size(); i++) {
        if(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics && inst->supportsPresent(physical, i, win)) {
            g_i = i;
            countF++;
            pqinfo[0] = {{}, i, 1, priorities.data()};
            break;
        }
    }
    
    if(g_i == 1000) {
        qCritical() << "Could not retrieve queue family" << endl;
        exit(3);
    }
    
    // Gets a compute queue family different from graphics family if possible, then different queue index if possible, else just the same queue.
    c_i = 1000;
    for(uint32_t i = 0; i < queueFamilies.size(); i++) {
        if(queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) {
            c_i = i; c_j = 0;
            if(c_i != g_i) {
                countF++;
                pqinfo[1] = {{}, i, 1, priorities.data()};
                break;
            }
            while(c_j == g_j && queueFamilies[i].queueCount > c_j + 1) c_j++;
        }
    }
    
    
    if(c_i == 1000) {
        qCritical() << "could not get compute queue" << endl;
        exit(4);
    }
    
    
    if(c_i == g_i && c_j != g_j) pqinfo[0].queueCount++; // If the same queue family but different queue index, create one more queue from the queue family.
    
    
    // Gets a transfer queue family different from graphics and compute family if possible, then different queue index if possible, else just the same queue.
    t_i = 1000;
    for(uint32_t i = 0; i < queueFamilies.size(); i++) { 
        t_i = i; t_j = 0;
        if(t_i != g_i && t_i != c_i) {
            countF++;
            pqinfo[2] = {{}, i, 1, priorities.data()};
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
    
    logical = physical.createDevice(vk::DeviceCreateInfo({}, countF, pqinfo.data(), 0, nullptr, requiredExtensions.size(), requiredExtensions.data(), &requiredFeatures));
    
    graphics = logical.getQueue(g_i, g_j);
    
    if(c_i == g_i && c_j == g_j) {
        compute = graphics;
    } else {
        compute = logical.getQueue(c_i, c_j);
    }
    
    if(t_i == g_i && t_j == g_j) {
        transfer = graphics;
    } else if(t_i == c_i && t_j ==c_j) {
        transfer = compute;
    } else {
        transfer = logical.getQueue(t_i, t_j);
    }
    
}

    


uint32_t Device::getScore(vk::PhysicalDevice &device) {
    // Get device properties
    
    uint32_t score = 1;
    
    properties = device.getProperties();
    features = device.getFeatures();
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
    std::vector<vk::ExtensionProperties> extensions = device.enumerateDeviceExtensionProperties();
    
    if(features.geometryShader) score++; // supports geometry shaders
    if(properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score++; // is a dedicated graphics card
    if(queueFamilies.size() > 1) score ++; // has more than one queue family
    
    std::set<std::string> required(requiredExtensions.begin(), requiredExtensions.end());
    for(const auto &ext : extensions) {
        required.erase(ext.extensionName);
    }
    
    if(!required.empty()) score = 0;
    
    auto prop2 = device.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceSubgroupProperties>();
    auto subgroup = prop2.get<vk::PhysicalDeviceSubgroupProperties>();
    
    if(subgroup.subgroupSize < 4) score = 0;
    if(!(subgroup.supportedOperations & vk::SubgroupFeatureFlagBits::eArithmetic)) score = 0;

    std::cout << properties.deviceName << " : " << score << "\n";
    return score;
}

bool Device::getMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties, uint32_t *index) {
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
    logical.destroy();
}
