#include "object_class.h"
#include <random>

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

    // use the farthest reaching vertex as the default collision_radius
    collision_radius = meshes[_model].maxRadius * model_initial_scale;

    // randomize initial rotate speed
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disInt1(1, 3);
    std::uniform_int_distribution<> disInt2(0, 1);
    std::uniform_real_distribution<> disRe1(0.01, 0.1);
    std::uniform_real_distribution<> disRe2(0.001, 0.01);
    std::uniform_real_distribution<> disRe3(0.0001, 0.001);
    int order = disInt1(gen);
    switch (order) {
    case 1:
        rotateY_last = disRe1(gen) * (disInt2(gen) ? 1 : -1);
        break;
    case 2:
        rotateY_last = disRe2(gen) * (disInt2(gen) ? 1 : -1);
        break;
    case 3:
        rotateY_last = disRe3(gen) * (disInt2(gen) ? 1 : -1);
        break;
    default:
        break;
    }

    updateMass();
}

void Object::updateMass() {
    mass = collision_radius * collision_radius * collision_radius * 4.0 / 3.0 * PI;
}