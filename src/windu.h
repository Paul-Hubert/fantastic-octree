#ifndef WINDU_H
#define WINDU_H

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
    Compute compute;
    Renderer renderer;
    Sync sync;
    Camera camera;

    QSize size;

private :
    bool destroying = false;
    bool loaded = false;
    uint32_t i = 0, time = 0;

    QElapsedTimer timer;
    qint64 lastNano = 0, lastHun = 0;
};

#endif
