#pragma once

#include <QWindow>
#include <QVulkanInstance>
#include <QVulkanFunctions>
#include <QVulkanDeviceFunctions>

#include "device.h"
#include "swapchain.h"

class Windu : public QWindow {
public :
    Windu();
    ~Windu();
    virtual void resizeEvent(QResizeEvent *ev) override;
    virtual void keyPressEvent(QKeyEvent *ev) override;
    QVulkanInstance inst;
    QVulkanFunctions* vki;
    QVulkanDeviceFunctions* vkd;
    Device device;
    Swapchain swap;
private :
    void getDevice();
    int getScore(VkPhysicalDevice *dev);
    void select(VkPhysicalDevice *dev);
    void start();
    QSize size;
};
