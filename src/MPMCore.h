#pragma once
#include "Math.h"
#include "Material.h"
#include <vector>

struct Particle {
    Vec3 position;
    Vec3 velocity;
    float mass;
    float initial_volume;
    Material* material;
    
    // MPM Specifics
    Mat3 F;          // Deformation gradient (Total)
    Mat3 F_elastic;  // Elastic deformation gradient
    Mat3 F_plastic;  // Plastic deformation gradient
    Mat3 stress;     // Cauchy stress tensor
    Mat3 C;          // Affine momentum matrix (for APIC)
    
    // State flag (for fracture)
    bool broken;

    Particle() : mass(0), initial_volume(0), material(nullptr), broken(false) {
        F = Mat3::identity();
        F_elastic = Mat3::identity();
        F_plastic = Mat3::identity();
        stress = Mat3();
        C = Mat3();
    }
};

struct GridNode {
    float mass;
    Vec3 velocity;
    Vec3 velocity_new;
    Vec3 force;
    bool is_active;

    GridNode() : mass(0), is_active(false) {}
    
    void clear() {
        mass = 0.0f;
        velocity = Vec3(0,0,0);
        velocity_new = Vec3(0,0,0);
        force = Vec3(0,0,0);
        is_active = false;
    }
};

struct MPMGrid {
    float cell_size;
    int width, height, depth;
    std::vector<GridNode> nodes;

    MPMGrid(float cell_size, int w, int h, int d) 
        : cell_size(cell_size), width(w), height(h), depth(d) 
    {
        nodes.resize(width * height * depth);
    }

    int index(int x, int y, int z) const {
        return x + y * width + z * width * height;
    }
    
    bool contains(int x, int y, int z) const {
        return x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth;
    }
    
    GridNode& getNode(int x, int y, int z) {
        return nodes[index(x, y, z)];
    }
    
    void clear() {
        for (auto& node : nodes) {
            node.clear();
        }
    }
};
