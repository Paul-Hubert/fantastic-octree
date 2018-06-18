#include <QDebug>
#include <QFile>

#include "helper.h"

void foAssert(VkResult res) {
    if(res != VK_SUCCESS) {
        qDebug("Assert failed");
        assert(res == VK_SUCCESS);
    }
}

QByteArray* load(QString fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return nullptr;
    return &file.readAll();
}
