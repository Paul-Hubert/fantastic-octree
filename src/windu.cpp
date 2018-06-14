#include "windu.h"
#include <stdio.h>
#include <QResizeEvent>
#include "helper.h"

Windu::Windu() : size(1024, 768) {

    inst.setLayers(QByteArrayList()
                   << "VK_LAYER_GOOGLE_threading"
                   << "VK_LAYER_LUNARG_parameter_validation"
                   << "VK_LAYER_LUNARG_object_tracker"
                   << "VK_LAYER_LUNARG_core_validation"
                   << "VK_LAYER_LUNARG_image"
                   << "VK_LAYER_LUNARG_swapchain"
                   << "VK_LAYER_GOOGLE_unique_objects");

    if (!inst.create())
        qFatal("Failed to create Vulkan instance: %d", inst.errorCode());
    vki = inst.functions();
    
    device.init(inst);
    
    start();
}

Windu::~Windu() {
    printf("Destroying");
}

void Windu::start() {
    surface = QVulkanInstance::surfaceForWindow(this);
    setVulkanInstance(&inst);
    resize(size);
    show();
}

void Windu::resizeEvent(QResizeEvent *ev) {
    size = ev->size();
}

void Windu::keyPressEvent(QKeyEvent *ev) {
    if(ev->key() == Qt::Key_Escape) {
        this->destroy();
    }
}
