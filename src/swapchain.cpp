#include <QVulkanFunctions>
#include <iostream>

#include "windu.h"
#include "helper.h"

void Swapchain::init(Windu *win) {
    
    surface = win->inst.surfaceForWindow(win);
    
}

Swapchain::~Swapchain() {
    
}
