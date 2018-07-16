#include <QLoggingCategory>
#include <QGuiApplication>

#include "windu.h"
#include "helper.h"

Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

    Windu* win = new Windu();

    win->showFullScreen();

    int ret = app.exec();
    delete(win);
    return ret;
}
