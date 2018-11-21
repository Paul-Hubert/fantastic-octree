#ifndef HELPER_H
#define HELPER_H


#include <vulkan/vulkan.hpp>
#include <QString>
#include <QByteArray>

void foAssert(VkResult result);

QByteArray foLoad(QString fileName);

void foWrite(QString fileName, const char* data, qint64 len);

#endif
