#include <QLoggingCategory>
#include <QFile>

#include "helper.h"

void foAssert(VkResult res) {
    if(res != VK_SUCCESS) {
        qDebug("Assert failed");
        assert(res == VK_SUCCESS);
    }
}

QByteArray foLoad(QString fileName) {
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not load " << fileName << ". Returning empty byte array." << endl;
        return QByteArray();
    }
    return file.readAll();
}

void foWrite(QString fileName, const char* data, qint64 len) {
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not write to : " << fileName << endl;
        return;
    }
    qInfo() << "Wrote : " << file.write(data, len) << endl;;
    file.close();
}
