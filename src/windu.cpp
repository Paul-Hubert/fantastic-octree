#include <stdio.h>
#include <QResizeEvent>
#include <iostream>
#include <QSurface>

#include "windu.h"
#include "helper.h"

Windu::Windu() : device(this), swap(this), compute(this), size(1024, 768) {
    
    setSurfaceType(SurfaceType::VulkanSurface);
    
    inst.setLayers(QByteArrayList()
                   << "VK_LAYER_GOOGLE_threading"
                   << "VK_LAYER_LUNARG_parameter_validation"
                   << "VK_LAYER_LUNARG_object_tracker"
                   << "VK_LAYER_LUNARG_standard_validation"
                   << "VK_LAYER_LUNARG_image"
                   << "VK_LAYER_LUNARG_swapchain"
                   << "VK_LAYER_GOOGLE_unique_objects");
    
    if (!inst.create())
        qFatal("Failed to create Vulkan instance: %d", inst.errorCode());
    
    setVulkanInstance(&inst);
    
    resize(size);
    
}

Windu::~Windu() {
    printf("Destroying\n");
}

void Windu::start() {
    
    swap.getSurface();
    
    if(!loaded) {
        vki = inst.functions();
        device.init();
        vkd = inst.deviceFunctions(device.logical);
    }
    
    swap.init();
    
    if(!loaded) compute.init();
    else compute.reset();
    
    loaded = true;
}

void Windu::reset() {
    
}

void Windu::render() {
    
}

void Windu::exposeEvent(QExposeEvent *) {
    if (isExposed()) {
        start();
    } else {
        reset();
    }
}

void Windu::resizeEvent(QResizeEvent *ev) {
    size = ev->size();
}

void Windu::keyPressEvent(QKeyEvent *ev) {
    if(ev->key() == Qt::Key_Escape) {
        this->destroy();
    }
}

bool Windu::event(QEvent *e) {
    switch (e->type()) {
        case QEvent::UpdateRequest:
            render();
            break;
        // The swapchain must be destroyed before the surface as per spec. This is
        // not ideal for us because the surface is managed by the QPlatformWindow
        // which may be gone already when the unexpose comes, making the validation
        // layer scream. The solution is to listen to the PlatformSurface events.
        case QEvent::PlatformSurface:
            if (static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
                swap.reset();
            }
            break;
        default:
            break;
    }
    return QWindow::event(e);
}
