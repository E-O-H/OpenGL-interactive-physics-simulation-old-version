#include "camera.h"

void Camera::strafe(float del) {
    position += lookDirection.cross(upDirection).normalized() * del;
    update_view_matrix();
}

void Camera::ascend(float del) {
    position += upDirection * del;
    update_view_matrix();
}

void Camera::forward(float del) {
    position += lookDirection * del;
    update_view_matrix();
}

void Camera::look(float yaw, float pitch) {
    lookDirection = Vector3f(cos(pitch) * sin(yaw), sin(pitch), - cos(pitch) * cos(yaw));
    update_view_matrix();
}

void Camera::update_view_matrix() {
    // The three base vectors of the camera (view) coordinates frame
    Vector3f zBase = - lookDirection;
    Vector3f xBase = upDirection.cross(zBase).normalized();
    Vector3f yBase = zBase.cross(xBase);
    M_view << xBase.x(), xBase.y(), xBase.z(), -xBase.dot(position),
              yBase.x(), yBase.y(), yBase.z(), -yBase.dot(position),
              zBase.x(), zBase.y(), zBase.z(), -zBase.dot(position),
              0,         0,         0,         1;
}

void Camera::update_projection_matrix() {
    M_orthographic << 1 / ortho_width, 0, 0, 0,
                      0, 1 / ortho_width * RESOLUTION_Y / RESOLUTION_X, 0, 0,
                      0, 0, -2 / (Z_far_limit - Z_near_limit), -(Z_far_limit + Z_near_limit) / (Z_far_limit - Z_near_limit),
                      0, 0, 0, 1;
    M_perspective << atan(persp_FOVx / 2), 0, 0, 0,
                     0, atan(persp_FOVx / 2 * RESOLUTION_Y / RESOLUTION_X), 0, 0,
                     0, 0, -(Z_far_limit + Z_near_limit) / (Z_far_limit - Z_near_limit), -2 * Z_far_limit * Z_near_limit / (Z_far_limit - Z_near_limit),
                     0, 0, -1, 0;
};

Camera::Camera() : position(DEFAULT_CAMERA_X, DEFAULT_CAMERA_Y, DEFAULT_CAMERA_Z),
                   lookDirection(cos(DEFAULT_CAMERA_PITCH) * sin(DEFAULT_CAMERA_YAW), 
                                 sin(DEFAULT_CAMERA_PITCH),
                                 - cos(DEFAULT_CAMERA_PITCH) * cos(DEFAULT_CAMERA_YAW)), 
                   upDirection(0.0, 1.0, 0.0),
                   Z_far_limit(10000.0), Z_near_limit(1E-3),
                   ortho_width(DEFAULT_ORTHO_WIDTH), persp_FOVx(DEFAULT_PERSP_FOVX),
                   perspective(true) {
    update_view_matrix();
    update_projection_matrix();
}