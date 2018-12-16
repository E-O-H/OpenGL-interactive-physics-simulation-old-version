#pragma once

#include "common_header.h"

class Mesh;
class Object;
extern vector<Mesh> meshes;          // list to store all model meshes
extern vector<Object> objects;       // list to store all objects in the scene

enum shading_t {
    WIREFRAME,
    FLAT,
    PHONG,
    DEBUG_NORMAL
};

struct Point {
    float x, y, z;
};

struct Point2d {
    float u, v;
};

struct Face {
    unsigned a, b, c;
};

// Class to represent a model mesh
class Mesh {
public:
    vector<Point> V;   // List of vertices
    vector<Face>  F;   // List of faces

    vector<Point> VN;   // List of vertex normals
    vector<Point> FN;   // List of face normals

    vector<Point2d> texCorrds; // List of vertex texture coords
    unsigned texture = -1;     // texture data ID; -1 stands for no texture

    float barycenterX, barycenterY, barycenterZ;

    Mesh() {};
};

// Class to represent an object
class Object {
public:
    Object(unsigned _model = 0);

    unsigned model;

    // rendering related attributes
    shading_t shading = PHONG;
    bool wireframe = false;
    unsigned int colorR = 255;
    unsigned int colorG = 255;
    unsigned int colorB = 255;
    float alpha = 1.0;
    float diffuse = 0.8;
    float specular = 300.0;
    int phongExp = 25;

    // initial transformation on the original model (not changed or used in physics simulation)
    double model_initial_translateX, model_initial_translateY, model_initial_translateZ;
    double model_initial_scale = 1.0;

    // model-world transformation
    double translateX = 0.0, translateY = 0.0, translateZ = 0.0;
    double rotateX = 0.0, rotateY = 0.0, rotateZ = 0.0;

    // attributes used only for physics calculation
    double translateX_last = 0.0, translateY_last = 0.0, translateZ_last = 0.0; // These are last recorded state
    double rotateX_last = 0.0, rotateY_last = 0.0, rotateZ_last = 0.0;          // for physics calculation
    double collision_radius = 1.0;
    double density = 1.0;          // For calculating mass
    double mass;                   // Derived from density and collision_radius
    bool COLLISION = false; // To check if the object is already in a collision (so it is not deemed as a new collision)
                            // Note since there is only one collision flag, 
                            // collision with multiple objects at the same time is not supported

    void updateMass();
};