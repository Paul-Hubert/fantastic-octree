#include <stdio.h>
#include <QResizeEvent>
#include <iostream>
#include <QSurface>

#include "windu.h"
#include "helper.h"

Windu::Windu() : device(this), swap(this), compute(this), renderer(this), sync(this), size(1024, 768) {
    
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
    
    //resize(size);
    
}

Windu::~Windu() {
    vkd->vkDeviceWaitIdle(device.logical);
    printf("Destroying\n");
}

void Windu::start() {
    
    swap.getSurface();
    
    if(!loaded) {
        prepareGraph();
        
        vki = inst.functions();
        device.init();
        vkd = inst.deviceFunctions(device.logical);
    }
    
    swap.init();
    
    if(!loaded) {
        compute.init();
        renderer.init();
    } else {
        compute.reset();
        renderer.reset();
    }
    
    if(!loaded) {
        sync.init();
        loaded = true;
        requestUpdate();
    }
    
    timer.start();
}

void Windu::reset() {
    
}

void Windu::prepareGraph() {
    
    swap.signalTo(&compute, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    compute.signalTo(&renderer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    renderer.signalTo(&swap, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    
}

void Windu::render() {
    
    qint64 currentNano = timer.nsecsElapsed();
    qInfo() << (currentNano - lastNano)/1000000. << "ms since last frame."  << "fps:" << 1000000000./(currentNano - lastNano)<< endl;
    lastNano = currentNano;
    
    if(destroying) {
        destroy();
        return;
    }
    
    i = swap.swap();
    
    compute.render(i);
    
    renderer.render(i);
    
    sync.step();
    requestUpdate();
}

void Windu::exposeEvent(QExposeEvent *) {
    if (isExposed() && !loaded) {
        start();
    }
}

void Windu::resizeEvent(QResizeEvent *ev) {
    size = ev->size();
}

void Windu::keyPressEvent(QKeyEvent *ev) {
    if(ev->key() == Qt::Key_Escape) {
        destroying = true;
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
