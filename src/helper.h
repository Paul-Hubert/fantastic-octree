#ifndef HELPER_H
#define HELPER_H

#include <QString>
#include <QByteArray>

#include <vulkan/vulkan_core.h>

void foAssert(VkResult result);

QByteArray foLoad(QString fileName);

void foWrite(QString fileName, const char* data, qint64 len);

#endif
