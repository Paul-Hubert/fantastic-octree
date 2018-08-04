#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

#include "camera.h"

void Camera::init(int width, int height) {
    speed = 0.002;
    setup(width, height);
}

void Camera::setup(int width, int height) {
    this->width = width;
    this->height = height;
    proj = glm::perspective(glm::radians(70.0f), width / (float) height, 0.1f, 1000.0f);
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
        pos.x += speed*sin(M_PI/2.0 + yangle)*dt;
        pos.z += speed*cos(M_PI/2.0 + yangle)*dt;
    } if(right) {
        pos.x -= speed*sin(M_PI/2.0 + yangle)*dt;
        pos.z -= speed*cos(M_PI/2.0 + yangle)*dt;
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
    return glm::inverse(getView()) * proj;
}

glm::mat4 Camera::getProj() {
    return proj;
}

glm::mat4 Camera::getView() {
    view = glm::mat4(1.0);
    view = glm::rotate(view, (float) yangle, glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, (float) xangle, glm::vec3(1.0f, 0.0f, 0.0f));
    return view;
}

glm::vec3 Camera::getPos() {
    return pos;
}

void Camera::mouseMoveEvent(QMouseEvent *ev) {
    yangle += (ev->pos().x() - this->width /2.0) * (M_PI*0.1/180.);
    xangle -= (ev->pos().y() - this->height/2.0) * (M_PI*0.1/180.);
}

void Camera::keyPressEvent(QKeyEvent *ev) {
    poll(ev, true);
}

void Camera::keyReleaseEvent(QKeyEvent *ev) {
    poll(ev, false);
}

void Camera::poll(QKeyEvent *ev, bool value) {
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
            break;
        case(Qt::Key_Space):
            this->space = value;
            break;
        case(Qt::Key_Shift):
            this->shift = value;
            break;
    }
}
