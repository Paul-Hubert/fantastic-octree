#define _USE_MATH_DEFINES
#include <math.h>

#include "camera.h"

void Camera::init(int width, int height) {
    
    setup(width, height);
}

void Camera::setup(int width, int height) {
    this->width = width;
    this->height = height;
    proj = glm::perspective(glm::radians(60.0f), width / (float) height, 0.1f, 1000.0f);
}

void Camera::cleanup() {
    
}

void Camera::reset(int width, int height) {
    cleanup();
    setup(width, height);
}

Camera::~Camera() {
    
}

void Camera::step(qint64 dt) {
    if(left) {
        pos.x -= speed*sin(M_PI/2.0 + yangle)*dt;
        pos.z -= speed*cos(M_PI/2.0 + yangle)*dt;
    } if(right) {
        pos.x += speed*sin(M_PI/2.0 + yangle)*dt;
        pos.z += speed*cos(M_PI/2.0 + yangle)*dt;
    } if(up) {
        pos.x -= speed*sin(yangle)*dt;
        pos.z -= speed*cos(yangle)*dt;
    } if(down) {
        pos.x += speed*sin(yangle)*dt;
        pos.z += speed*cos(yangle)*dt;
    }
    
    if(space) pos.y += speed*dt;
    if(shift) pos.y -= speed*dt;
}

glm::mat4 Camera::getViewProj() {
    view = glm::translate(glm::mat4(1.0), pos);
    view = glm::rotate(view, (float) xangle, glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, (float) yangle, glm::vec3(0.0f, 1.0f, 0.0f));
    return glm::inverse(view) * proj;
}

void Camera::mouseMoveEvent(QMouseEvent *ev) {
    
    yangle += (ev->pos().x() - this->width/2.0) * (M_PI*0.1/180.);
    xangle -= (ev->pos().y() - this->height/2.0)* (M_PI*0.1/180.);
}

void Camera::keyPressEvent(QKeyEvent *ev) {
    bool value = true;
    switch(ev->key()) {
        case(Qt::Key_Z):
            this->up = value;
            break;
        case(Qt::Key_Q):
            this->left = value;
            break;
        case(Qt::Key_D):
            this->right = value;
            break;
        case(Qt::Key_S):
            this->down = value;
    }
}

void Camera::keyReleaseEvent(QKeyEvent *ev) {
    bool value = false;
    switch(ev->key()) {
        case(Qt::Key_Z):
            this->up = value;
            break;
        case(Qt::Key_Q):
            this->left = value;
            break;
        case(Qt::Key_D):
            this->right = value;
            break;
        case(Qt::Key_S):
            this->down = value;
    }
}
