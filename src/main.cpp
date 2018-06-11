#include <QGuiApplication>
#include <QVulkanInstance>
#include <QLoggingCategory>
#include "renderer.h"

Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

    QVulkanInstance inst;

#ifndef Q_OS_ANDROID
    inst.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");
#else
    inst.setLayers(QByteArrayList()
                   << "VK_LAYER_GOOGLE_threading"
                   << "VK_LAYER_LUNARG_parameter_validation"
                   << "VK_LAYER_LUNARG_object_tracker"
                   << "VK_LAYER_LUNARG_core_validation"
                   << "VK_LAYER_LUNARG_image"
                   << "VK_LAYER_LUNARG_swapchain"
                   << "VK_LAYER_GOOGLE_unique_objects");
#endif

    if (!inst.create())
        qFatal("Failed to create Vulkan instance: %d", inst.errorCode());

    VulkanWindow w;
    w.setVulkanInstance(&inst);
    w.resize(1024, 768);
    w.show();

    return app.exec();
}
