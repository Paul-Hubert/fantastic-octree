#include <QDebug>

#include "helper.h"

void foAssert(VkResult res) {
    if(res != VK_SUCCESS) {
        qDebug("Assert failed");
        assert(res == VK_SUCCESS);
    }
}


