#include "Simulation.h"
#include <iostream>

// B-Spline weights
static inline void calcWeights(float x, float& w0, float& w1, float& w2, float& dw0, float& dw1, float& dw2, float inv_dx) {
    float d = x;
    w0 = 0.5f * (1.5f - d) * (1.5f - d);
    w1 = 0.75f - (d - 1.0f) * (d - 1.0f);
    w2 = 0.5f * (d - 0.5f) * (d - 0.5f);
    
    dw0 = (d - 1.5f) * inv_dx;
    dw1 = -2.0f * (d - 1.0f) * inv_dx;
    dw2 = (d - 0.5f) * inv_dx;
}

Simulation::Simulation(float grid_cell_size, int w, int h, int d) 
    : grid(grid_cell_size, w, h, d), time_scale(1.0f), fixed_dt(0.0001f), time_accumulator(0.0f) 
{
    particles.reserve(1000000); // Pre-allocate
    gas_material = new Material(MaterialType::GAS, 1.225f, 100000.0f, 0.0f, 0.0f, 0.0f);
}

Simulation::~Simulation() {
    delete gas_material;
}

void Simulation::update(float deltaTime) {
    time_accumulator += deltaTime * time_scale;
    
    int num_steps = 0;
    while (time_accumulator >= fixed_dt) {
        step();
        time_accumulator -= fixed_dt;
        num_steps++;
    }
}

void Simulation::step() {
    grid.clear();
    
    emitGas(fixed_dt);
    
    particleToGrid();
    
    updateDeformationAndStress();
    
    updateGridForcesAndVelocities();
    
    gridToParticle();
}

void Simulation::emitGas(float dt) {
    // 1. Bernoulli equation for velocity
    // v = sqrt( 2 * (P - P0) / rho )
    float p_diff = tank.current_pressure_pa - 101325.0f;
    if (p_diff <= 0) return; // No outward pressure

    float exit_velocity = std::sqrt(2.0f * p_diff / tank.gas_density);
    
    // Calculate mass flow rate: m_dot = rho * A * v
    float area = 3.14159f * (tank.nozzle_diameter_m * 0.5f) * (tank.nozzle_diameter_m * 0.5f);
    float mass_flow = tank.gas_density * area * exit_velocity;
    float emitted_mass = mass_flow * dt;
    
    if (emitted_mass > 0) {
        // Spawn particle
        Particle p;
        p.position = tank.nozzle_position;
        p.velocity = tank.nozzle_direction.normalized() * exit_velocity;
        p.mass = emitted_mass;
        p.initial_volume = emitted_mass / tank.gas_density;
        p.material = gas_material;
        particles.push_back(p);
    }
}

void Simulation::particleToGrid() {
    float inv_dx = 1.0f / grid.cell_size;
    
    for (auto& p : particles) {
        Vec3 base_coord = (p.position * inv_dx) - Vec3(0.5f, 0.5f, 0.5f);
        int base_x = (int)base_coord.x;
        int base_y = (int)base_coord.y;
        int base_z = (int)base_coord.z;
        
        Vec3 fx = base_coord - Vec3((float)base_x, (float)base_y, (float)base_z);
        
        float w[3][3], dw[3][3];
        float wx[3], wy[3], wz[3];
        float dwx[3], dwy[3], dwz[3];
        
        calcWeights(fx.x, wx[0], wx[1], wx[2], dwx[0], dwx[1], dwx[2], inv_dx);
        calcWeights(fx.y, wy[0], wy[1], wy[2], dwy[0], dwy[1], dwy[2], inv_dx);
        calcWeights(fx.z, wz[0], wz[1], wz[2], dwz[0], dwz[1], dwz[2], inv_dx);
        
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    int gx = base_x + i;
                    int gy = base_y + j;
                    int gz = base_z + k;
                    
                    if (!grid.contains(gx, gy, gz)) continue;
                    
                    float weight = wx[i] * wy[j] * wz[k];
                    
                    Vec3 dpos(
                        ((float)gx - base_coord.x - fx.x + 0.5f) * grid.cell_size,
                        ((float)gy - base_coord.y - fx.y + 0.5f) * grid.cell_size,
                        ((float)gz - base_coord.z - fx.z + 0.5f) * grid.cell_size
                    );
                    
                    GridNode& node = grid.getNode(gx, gy, gz);
                    node.mass += p.mass * weight;
                    // APIC transfer
                    node.velocity += (p.velocity + p.C * dpos) * (p.mass * weight);
                    node.is_active = true;
                }
            }
        }
    }
    
    for (auto& node : grid.nodes) {
        if (node.mass > 0) {
            node.velocity = node.velocity / node.mass;
        }
    }
}
