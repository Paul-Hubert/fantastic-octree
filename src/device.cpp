#include "device.h"
#include <QVulkanFunctions>
#include "helper.h"

void Device::init(QVulkanInstance &inst) {
    
    instance = &inst;
    
    QVulkanFunctions *vk = inst.functions();
    
    uint32_t num;
    vk->vkEnumeratePhysicalDevices(inst.vkInstance(), &num, nullptr);
    assert(num > 0);
    std::vector<VkPhysicalDevice> p_devices(num);
    vkAssert(vk->vkEnumeratePhysicalDevices(inst.vkInstance(), &num, p_devices.data()));
    
    uint32_t index = 1000, max = 0;
    for(uint32_t i = 0; i<num; i++) {
        uint32_t score = getScore(inst, p_devices[i]);
        if(score > max) {
            max = score;
            index = i;
        }
    }
    
    assert(index != 1000);
    physical =  p_devices[index]; // Found physical device
    
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
    
    uint32_t g_i = 0, g_j = 0, c_i = 0, c_j = 0, t_i = 0, t_j = 0, countF = 0;
    
    std::vector<float> priorities(3); priorities[0] = 0.0f; priorities[1] = 0.0f; priorities[2] = 0.0f; 
    
    std::vector<VkDeviceQueueCreateInfo> pqinfo(3); // Number of queues
    
    for(int i = 0; i < static_cast<int>(queueFamilies.size()); i++) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
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
    
    for(int i = 0; i < static_cast<int>(queueFamilies.size()); i++) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            c_i = i;
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
    
    if(c_i == g_i && c_j != g_j) pqinfo[0].queueCount++;
    
    for(int i = 0; i < static_cast<int>(queueFamilies.size()); i++) {
        t_i = i;
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
        while((t_j == g_j || t_j == c_j) && queueFamilies[i].queueCount > t_j + 1) t_j++;
    }
    
    if(t_i == g_i && t_j != g_j) {pqinfo[0].queueCount++;}
    else if(c_i == t_i && c_j != t_j) {pqinfo[1].queueCount++;}
    
    VkPhysicalDeviceFeatures enabledFeatures = {};
    // HERE : enable needed features (if present in 'features')
    
    std::vector<const char*> extensions = {};
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = countF; // Number of queue families involved.
    deviceInfo.pQueueCreateInfos = pqinfo.data();
    deviceInfo.pEnabledFeatures = &enabledFeatures;
    deviceInfo.enabledExtensionCount = (uint32_t) extensions.size();
    deviceInfo.ppEnabledExtensionNames = extensions.data();
    
    vkAssert(vk->vkCreateDevice(physical, &deviceInfo, nullptr, &logical)); // Create logical device
    
    QVulkanDeviceFunctions *vkd = inst.deviceFunctions(logical);
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

uint32_t Device::getScore(QVulkanInstance &inst, VkPhysicalDevice &device) {
    return 1;
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
    instance->deviceFunctions(logical)->vkDestroyDevice(logical, nullptr);
}
