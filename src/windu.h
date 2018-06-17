#ifndef WINDU_H
#define WINDU_H

#include <QWindow>
#include <QVulkanInstance>
#include <QVulkanFunctions>
#include <QVulkanDeviceFunctions>

#include "device.h"
#include "swapchain.h"
#include "compute.h"

class Windu : public QWindow {
public :
    Windu();
    ~Windu();
    
    void render();
    
    virtual void resizeEvent(QResizeEvent *ev) override;
    virtual void exposeEvent(QExposeEvent *ev) override;
    virtual void keyPressEvent(QKeyEvent *ev) override;
    virtual bool event(QEvent *e) override;
    void start();
    void reset();
    
    
    QVulkanInstance inst;
    QVulkanFunctions* vki;
    QVulkanDeviceFunctions* vkd;
    
    Device device;
    Swapchain swap;
    Compute compute;
    
    QSize size;
    
private :
    void getDevice();
    void select(VkPhysicalDevice *dev);
    int getScore(VkPhysicalDevice *dev);
    
    
    bool loaded = false;
};

#endif
