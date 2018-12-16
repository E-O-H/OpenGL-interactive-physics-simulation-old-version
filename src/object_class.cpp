#include "object_class.h"

Object::Object(unsigned _model) : model(_model) {
    model_initial_translateX = -meshes[_model].barycenterX;
    model_initial_translateY = -meshes[_model].barycenterY;
    model_initial_translateZ = -meshes[_model].barycenterZ;
    switch (_model) {
    case 0:
        model_initial_scale = 1.0;
        break;
    case 1:
        model_initial_scale = 0.2;
        break;
    case 2:
        model_initial_scale = 10.0;
        break;
    case 3:
        model_initial_scale = 0.5;
        break;
    default:
        break;
    }
    updateMass();
}

void Object::updateMass() {
    mass = collision_radius * collision_radius * collision_radius * 4.0 / 3.0 * PI;
}