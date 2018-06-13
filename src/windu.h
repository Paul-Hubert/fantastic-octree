#include <QWindow>
#include <QVulkanInstance>
#include <QVulkanFunctions>
#include <QVulkanDeviceFunctions>
#include "device.h"

class Windu : public QWindow {
public :
    Windu();
    ~Windu();
    virtual void resizeEvent(QResizeEvent *ev) override;
    virtual void keyPressEvent(QKeyEvent *ev) override;
private :
    void getDevice();
    int getScore(VkPhysicalDevice *dev);
    void select(VkPhysicalDevice *dev);
    void start();
    QVulkanInstance inst;
    QVulkanFunctions* vki;
    QVulkanDeviceFunctions* vkd;
    Device device;
    VkSurfaceKHR surface;
    bool exposed = false;
    QSize size;
};
