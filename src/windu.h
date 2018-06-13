#include <QWindow>
#include <QVulkanInstance>

class Windu : public QWindow {
public :
    Windu();
    ~Windu();
    virtual void resizeEvent(QResizeEvent *ev) override;
private:
    QVulkanInstance inst;
    VkSurfaceKHR surface;
    bool exposed = false;
    QSize size;
};
