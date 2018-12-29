#include "windu.h"

#include <stdio.h>
#include <QResizeEvent>
#include <iostream>
#include <QSurface>

#include "helper.h"
#include "terrain.h"

Windu::Windu() : device(this), swap(this), transfer(this), compute(this), renderer(this), sync(this), resman(this), size(1024, 768) {

    setSurfaceType(SurfaceType::VulkanSurface);

    inst.setLayers(QByteArrayList()
                   << "VK_LAYER_GOOGLE_threading"
                   << "VK_LAYER_LUNARG_parameter_validation"
                   << "VK_LAYER_LUNARG_object_tracker"
                   << "VK_LAYER_LUNARG_standard_validation"
                   << "VK_LAYER_LUNARG_image"
                   << "VK_LAYER_LUNARG_swapchain"
                   << "VK_LAYER_GOOGLE_unique_objects");

    inst.setExtensions(QByteArrayList()
                       << "VK_KHR_get_physical_device_properties2");

    if (!inst.create())
        qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

    setVulkanInstance(&inst);

    //setMouseTracking(true); // E.g. set in your constructor of your widget.

    //resize(size);

}

Windu::~Windu() {
    device.logical.waitIdle();
    delete(terrain);
    printf("Destroying\n");
}

void Windu::start() {

    swap.getSurface();

    if(!loaded) {

        vki = inst.functions();
        device.init();
        vkd = inst.deviceFunctions(device.logical);
        
        prepareGraph();
    }

    swap.init();

    if(!loaded) {
        
        // Specify resources to be allocated
        compute.preinit();
        renderer.preinit();
        transfer.preinit();
        
        // Allocate memory
        resman.init();
        
        camera.init(swap.extent.width, swap.extent.height);
        
        compute.init();
        renderer.init();
        
        terrain = new Terrain();
        terrain->init(&compute);
        transfer.init();
        
        sync.init();
        loaded = true;
        timer.start();
        requestUpdate();
        
    } else {
        camera.reset(swap.extent.width, swap.extent.height);
        compute.reset();
        renderer.reset();
    }

}

void Windu::reset() {
    
}

void Windu::prepareGraph() {

    compute.signalTo(&renderer, vk::PipelineStageFlagBits::eDrawIndirect | vk::PipelineStageFlagBits::eVertexInput);

    swap.signalTo(&renderer, vk::PipelineStageFlagBits::eColorAttachmentOutput);
    
    renderer.signalTo(&swap, vk::PipelineStageFlagBits::eTopOfPipe);

}

void Windu::render() {
    if(destroying) {
        destroy();
        return;
    }

    qint64 currentNano = timer.nsecsElapsed();

    camera.step((currentNano - lastNano)/1000000.);
    lastNano = currentNano;

    time++;
    if(time>=100) {
        double frametime = (currentNano - lastHun)/time/1000000.;
        qInfo() << frametime << "ms since last frame. fps:" << 1000./frametime<< endl;
        lastHun = currentNano;
        time = 0;
    }

    i = swap.acquire();
    
    transfer.render(i);

    terrain->step(1);
    
    compute.render(i);

    renderer.render(i);

    swap.present();
    
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
    camera.keyPressEvent(ev);
}

void Windu::keyReleaseEvent(QKeyEvent *ev) {
    camera.keyReleaseEvent(ev);
}

// Implement in your widget
void Windu::mouseMoveEvent(QMouseEvent *ev) {
    camera.mouseMoveEvent(ev);
    QCursor c = cursor();
    c.setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
    c.setShape(Qt::BlankCursor);
    setCursor(c);
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
                qInfo("Swapchain resetting");
                swap.reset();
            }
            break;
        default:
            break;
    }
    return QWindow::event(e);
}
