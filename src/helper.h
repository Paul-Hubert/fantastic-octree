#ifndef HELPER_H
#define HELPER_H

#include <QVulkanInstance>
#include <QString>
#include <QByteArray>

void foAssert(VkResult result);

QByteArray load(QString fileName);

#endif