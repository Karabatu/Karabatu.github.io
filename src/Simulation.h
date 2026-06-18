#pragma once
#include "MPMCore.h"
#include "GasTank.h"
#include <vector>

class Simulation {
public:
    MPMGrid grid;
    std::vector<Particle> particles;
    GasTank tank;
    Material* gas_material;

    float fixed_dt;
    float time_scale;
    float time_accumulator;

    Simulation(float grid_cell_size, int w, int h, int d);
    ~Simulation();

    // Time-Stepping & Slow-Motion (Segment 2)
    void update(float deltaTime);

private:
    void step();
    
    // Gas Emitter (Segment 2)
    void emitGas(float dt);

    // MPM Loop methods (Segments 2 & 3)
    void particleToGrid();
    void updateGridForcesAndVelocities();
    void updateDeformationAndStress();
    void gridToParticle();

    // Helper functions for math
    void computeWeights(const Vec3& p_pos, float& out_w[3][3][3], Vec3& out_gx[3][3][3], int base_idx[3]);
};
