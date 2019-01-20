#include "common_header.h"
#include "object_class.h"
#include <iostream>
#include <fstream>

const string examplesPath = "./data/examples/";
extern double dt;
extern vector<Object> objects;

int loadPremadeScene(string sceneFilename) {
    try {
        std::ifstream sceneFile((examplesPath + sceneFilename).c_str());
        if (!sceneFile.good()) {
            sceneFile.close();
            sceneFile.open(("../" + examplesPath + sceneFilename).c_str());
        }
        if (!sceneFile.good()) throw 1;

        unsigned n_objects;
        sceneFile >> n_objects;

        for (unsigned i = 0; i < n_objects; ++i) {	
            double r, x, y, z, vx, vy, vz, c_r, c_g, c_b, density, x_last, y_last, z_last;
            int light;
            sceneFile >> r >> x >> y >> z >> vx >> vy >> vz >> c_r >> c_g >> c_b >> density >> light;
            x_last = x - dt * vx;
            y_last = y - dt * vy;
            z_last = z - dt * vz;

            Object temp(Object(6));
            temp.collision_radius = r;
            temp.translateX = x;
            temp.translateY = y;
            temp.translateZ = z;
            temp.translateX_last = x_last;
            temp.translateY_last = y_last;
            temp.translateZ_last = z_last;
            temp.colorR = c_r;
            temp.colorG = c_g;
            temp.colorB = c_b;
            temp.density = density;
            temp.updateMass();

            objects.insert(objects.end() - 1, temp);
        }

        sceneFile.close();
        return 0;
    } catch (...) {
        std::cerr << "Error opening file." << std::endl;
        return -1;
    }
}