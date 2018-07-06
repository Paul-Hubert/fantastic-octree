#ifndef HELPER_H
#define HELPER_H

#include <QVulkanInstance>
#include <QString>
#include <QByteArray>

#include <vulkan/vulkan_core.h>

void foAssert(VkResult result);

QByteArray foLoad(QString fileName);

VkShaderModule foCreateShaderFromFile(QString fileName);
#endif
