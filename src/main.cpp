#include <QLoggingCategory>
#include <QGuiApplication>

#include "windu.h"

Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));
    
    Windu win;
    
    win.start();
    VkSurfaceKHR surf = QVulkanInstance::surfaceForWindow(&win);
    if(surf == nullptr) printf("fuck you world\n");

    return app.exec();
}
