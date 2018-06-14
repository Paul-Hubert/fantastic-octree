#include <stdio.h>
#include <QResizeEvent>
#include <iostream>

#include "windu.h"
#include "helper.h"

Windu::Windu() : size(1024, 768) {
    
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
    
    
    
    vki = inst.functions();
    device.init(inst);
    vkd = inst.deviceFunctions(device.logical);
}

Windu::~Windu() {
    printf("Destroying\n");
}

void Windu::start() {
    resize(size);
    
    show();
    
    swap.init(this);

}

void Windu::resizeEvent(QResizeEvent *ev) {
    size = ev->size();
}

void Windu::keyPressEvent(QKeyEvent *ev) {
    if(ev->key() == Qt::Key_Escape) {
        this->destroy();
    }
}
