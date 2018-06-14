#include "swapchain.h"
#include <QVulkanFunctions>
#include "helper.h"
#include <iostream>

void Swapchain::init(Windu *win) {
    
    surface = win->inst.surfaceForWindow(win);
    
}

Swapchain::~Swapchain() {
    
}
