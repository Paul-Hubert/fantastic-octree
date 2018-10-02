#ifndef CAMERA_H
#define CAMERA_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <qglobal.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    void init(int width, int height);
    void setup(int width, int height);
    void cleanup();
    void reset(int width, int height);
    void step(qint64 dt);
    ~Camera();
    glm::mat4 getViewProj();
    glm::mat4 getProj();
    glm::mat4 getView();
    glm::vec3 getPos();
    void mouseMoveEvent(QMouseEvent *ev);
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

    void poll(QKeyEvent *ev, bool value);
    
private:
    glm::mat4 proj, view;
    glm::vec3 pos;
    int width, height;
    
    double speed;
    double xangle = 0.0, yangle = 0.0;
    bool up = false, down = false, left = false, right = false, shift = false, space = false;
    
};
#endif
