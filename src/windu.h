#ifndef WINDU_H
#define WINDU_H

#include <vulkan/vulkan.hpp>
#include <QWindow>
#include <QVulkanInstance>
#include <QVulkanFunctions>
#include <QVulkanDeviceFunctions>
#include <QElapsedTimer>

#include "device.h"
#include "swapchain.h"
#include "compute.h"
#include "renderer.h"
#include "sync.h"
#include "camera.h"
#include "transfer.h"
#include "resources/resource_manager.h"

class Terrain;

class Windu : public QWindow {
public :
    Windu();
    ~Windu();

    void render();

    virtual void resizeEvent(QResizeEvent *ev) override;
    virtual void exposeEvent(QExposeEvent *ev) override;
    virtual void keyPressEvent(QKeyEvent *ev) override;
    virtual void keyReleaseEvent(QKeyEvent *ev) override;
    virtual void mouseMoveEvent(QMouseEvent *ev) override;
    virtual bool event(QEvent *e) override;
    void start();
    void reset();

    void prepareGraph();

    QVulkanInstance inst;
    QVulkanFunctions* vki;
    QVulkanDeviceFunctions* vkd;

    Device device;
    Swapchain swap;
    Transfer transfer;
    Compute compute;
    Renderer renderer;
    Sync sync;
    Camera camera;
    ResourceManager resman;
    
    Terrain* terrain;

    QSize size;

private :
    bool destroying = false;
    bool loaded = false;
    uint32_t i = 0, time = 0;

    QElapsedTimer timer;
    qint64 lastNano = 0, lastHun = 0;

public slots:
    void rend();
    
};

#endif
