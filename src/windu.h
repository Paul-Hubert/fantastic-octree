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
    void start();
    
    QVulkanInstance inst;
    QVulkanFunctions* vki;
    QVulkanDeviceFunctions* vkd;
    Device device;
    Swapchain swap;
    
private :
    void getDevice();
    void select(VkPhysicalDevice *dev);
    int getScore(VkPhysicalDevice *dev);
    
    QSize size;
};
