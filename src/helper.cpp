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

VkShaderModule foCreateShaderFromFile(VkDevice* device, QString fileName) {
    QByteArray bytes = foLoad(fileName);
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytes.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes.constData());
    VkShaderModule shaderModule;
    if(vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        
    }
}