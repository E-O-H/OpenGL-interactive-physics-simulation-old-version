#pragma once

#include "common_header.h"

#define DEFAULT_CAMERA_X     0.0
#define DEFAULT_CAMERA_Y     0.0
#define DEFAULT_CAMERA_Z     2.0
#define DEFAULT_CAMERA_YAW   0.0
#define DEFAULT_CAMERA_PITCH 0.0

#define DEFAULT_ORTHO_WIDTH  2.0
#define DEFAULT_PERSP_FOVX   PI / 3

// Class to represent camera
class Camera {
public:
    Vector3f position;
    Vector3f lookDirection, upDirection;
    bool perspective;
    float Z_far_limit, Z_near_limit, ortho_width, persp_FOVx;

    Matrix4f M_view, M_perspective, M_orthographic;

    void strafe(float del);
    void ascend(float del);
    void forward(float del);
    void look(float yaw, float pitch);

    // Updates the view matrix
    void update_view_matrix();
    // Updates the projection matrix
    void update_projection_matrix();

    Camera();
};