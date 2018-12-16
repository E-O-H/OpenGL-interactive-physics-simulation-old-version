#include "common_header.h"
#include "object_class.h"

double G_para = 5.0; // Gravity parameter
double dt = 0.01;    // Time step

template <typename T>
inline T square(T a) {
    return a * a;
}

extern vector<Object> objects;

void physics() {
    int n_objects = objects.size();
    vector<Vector3d> acceleration = vector<Vector3d>(n_objects, Vector3d::Zero());
    vector<Vector3d> pos = vector<Vector3d>(n_objects);
    vector<Vector3d> pos_last = vector<Vector3d>(n_objects);
    vector<Vector3d> temp_pos = vector<Vector3d>(n_objects);      // For temporarily storing the result of collision calculation,
    vector<Vector3d> temp_pos_last = vector<Vector3d>(n_objects); // a.k.a. new states to be updated after collision
    // preparing data
    for (int i = 0; i < n_objects; ++i) {
        pos[i] = Vector3d(objects[i].translateX, 
                          objects[i].translateY, 
                          objects[i].translateZ);
        pos_last[i] = Vector3d(objects[i].translateX_last, 
                               objects[i].translateY_last, 
                               objects[i].translateZ_last);
        temp_pos[i] = pos[i];            // Used as the default update value if there is no collision.
        temp_pos_last[i] = pos_last[i];  // Both pos and pos_last are updated after collision in this implementation
                                         // in order to preserve the total energy (kinetic + potential energy)
    }

    // physics calculation
    for (int i = 0; i < n_objects; ++i) {
        if(objects[i].COLLISION == true) {
            // If the object is already in a collision, check if it has completed the collision
            objects[i].COLLISION = false;
            for (int j = 0; j < n_objects; ++j) {
                if (j == i) continue;
                Vector3d distance = pos[j] - pos[i];
                if (distance.squaredNorm() < square(objects[i].collision_radius + objects[j].collision_radius)) {
                    objects[i].COLLISION = true;
                    break;
                }
            }
        } else {
            // If the object is not in a collision, check for new collisions
            for (int j = 0; j < n_objects; ++j) {
                if (j == i) continue;
                Vector3d distance = pos[j] - pos[i];
                if (distance.squaredNorm() < square(objects[i].collision_radius + objects[j].collision_radius)) {
                    objects[i].COLLISION = true;
                    distance.normalize();
                    double vi = (pos[i] - pos_last[i]).dot(distance);
                    double vj = (pos[j] - pos_last[j]).dot(distance);
                    double vi_n = (vi * (objects[i].mass - objects[j].mass) + vj * (2 * objects[j].mass)) 
                                  / (objects[i].mass + objects[j].mass);
                    // calculate collision change of state
                    temp_pos[i] = pos_last[i];
                    temp_pos_last[i] = pos_last[i] + distance * (vi - vi_n)
                                       + (pos_last[i] - pos[i]); 
                    break;
                }
            }
        }

        // calculate the acceleration of object i
        for (int j = 0; j < n_objects; ++j) {
            if (j == i) continue;
            Vector3d distance = pos[j] - pos[i];
            acceleration[i] += distance.normalized() * (G_para * objects[j].mass / distance.squaredNorm());
        }
    }
    
    // calculate the next positions for each object
    for(int i=0;i<n_objects;i++) {
        pos[i] = temp_pos[i];
        pos_last[i] = temp_pos_last[i];

        Vector3d next_pos;
        next_pos = pos[i] * 2 - pos_last[i] + acceleration[i] * (dt * dt); // Verlet Algorithm

        // write result
        objects[i].translateX_last = pos[i].x();
        objects[i].translateY_last = pos[i].y();
        objects[i].translateZ_last = pos[i].z();
        objects[i].translateX = next_pos.x();
        objects[i].translateY = next_pos.y();
        objects[i].translateZ = next_pos.z();
    }
}