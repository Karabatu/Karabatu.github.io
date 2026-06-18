#include "Simulation.h"
#include <algorithm>
#include <cmath>

static inline void calcWeightsLocal(float x, float& w0, float& w1, float& w2, float& dw0, float& dw1, float& dw2, float inv_dx) {
    float d = x;
    w0 = 0.5f * (1.5f - d) * (1.5f - d);
    w1 = 0.75f - (d - 1.0f) * (d - 1.0f);
    w2 = 0.5f * (d - 0.5f) * (d - 0.5f);
    
    dw0 = (d - 1.5f) * inv_dx;
    dw1 = -2.0f * (d - 1.0f) * inv_dx;
    dw2 = (d - 0.5f) * inv_dx;
}

void Simulation::updateDeformationAndStress() {
    float inv_dx = 1.0f / grid.cell_size;
    
    for (auto& p : particles) {
        if (p.broken) {
            p.stress = Mat3(); // Zero stress if broken
            continue;
        }

        Vec3 base_coord = (p.position * inv_dx) - Vec3(0.5f, 0.5f, 0.5f);
        int base_x = (int)std::floor(base_coord.x);
        int base_y = (int)std::floor(base_coord.y);
        int base_z = (int)std::floor(base_coord.z);
        Vec3 fx = base_coord - Vec3((float)base_x, (float)base_y, (float)base_z);
        
        float wx[3], wy[3], wz[3];
        float dwx[3], dwy[3], dwz[3];
        
        calcWeightsLocal(fx.x, wx[0], wx[1], wx[2], dwx[0], dwx[1], dwx[2], inv_dx);
        calcWeightsLocal(fx.y, wy[0], wy[1], wy[2], dwy[0], dwy[1], dwy[2], inv_dx);
        calcWeightsLocal(fx.z, wz[0], wz[1], wz[2], dwz[0], dwz[1], dwz[2], inv_dx);
        
        Mat3 gradV; // initialized to 0
        
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    int gx = base_x + i;
                    int gy = base_y + j;
                    int gz = base_z + k;
                    if (!grid.contains(gx, gy, gz)) continue;
                    
                    GridNode& node = grid.getNode(gx, gy, gz);
                    if (!node.is_active) continue;
                    
                    Vec3 v = node.velocity;
                    Vec3 weight_grad(
                        dwx[i] * wy[j] * wz[k],
                        wx[i] * dwy[j] * wz[k],
                        wx[i] * wy[j] * dwz[k]
                    );
                    
                    // gradV += v ⊗ weight_grad
                    gradV = gradV + Mat3::outer_product(v, weight_grad);
                }
            }
        }
        
        Mat3 F_new = (Mat3::identity() + gradV * fixed_dt) * p.F;
        p.F = F_new;
        
        float J = p.F.determinant();
        if (J <= 0.0f) J = 0.001f;

        if (p.material->type == MaterialType::GAS || p.material->type == MaterialType::WATER) {
            float gamma = (p.material->type == MaterialType::GAS) ? 1.4f : 7.0f;
            float density_ratio = 1.0f / J;
            float pressure = p.material->lambda * (std::pow(density_ratio, gamma) - 1.0f);
            p.stress = Mat3::identity() * (-pressure);
        } else {
            Mat3 F_e_new = (Mat3::identity() + gradV * fixed_dt) * p.F_elastic;
            p.F_elastic = F_e_new;
            
            float J_e = p.F_elastic.determinant();
            if (J_e <= 0.0f) J_e = 0.001f;
            
            Mat3 F_e_T = p.F_elastic.transpose();
            Mat3 B_e = p.F_elastic * F_e_T;
            
            float mu = p.material->mu;
            float lambda = p.material->lambda;
            
            p.stress = (B_e - Mat3::identity()) * (mu / J_e) + Mat3::identity() * ((lambda / J_e) * std::log(J_e));
            
            float trace = p.stress.trace();
            float mean_stress = trace / 3.0f;
            
            if (p.material->type == MaterialType::CONCRETE) {
                if (mean_stress > p.material->tensile_strength) {
                    p.broken = true;
                    p.stress = Mat3();
                    p.F_elastic = Mat3::identity();
                }
            } else if (p.material->type == MaterialType::WOOD) {
                Vec3 traction = p.stress * p.material->fiber_dir;
                float normal_stress = traction.dot(p.material->fiber_dir);
                if (normal_stress > p.material->yield_strength) {
                    p.broken = true;
                    p.stress = Mat3();
                    p.F_elastic = Mat3::identity();
                }
            }
        }
    }
}

void Simulation::updateGridForcesAndVelocities() {
    float inv_dx = 1.0f / grid.cell_size;
    Vec3 gravity(0.0f, -9.81f, 0.0f);
    
    // Distribute Forces
    for (auto& p : particles) {
        float volume = p.initial_volume * p.F.determinant();
        Mat3 stress_force_term = p.stress * volume;
        
        Vec3 base_coord = (p.position * inv_dx) - Vec3(0.5f, 0.5f, 0.5f);
        int base_x = (int)std::floor(base_coord.x);
        int base_y = (int)std::floor(base_coord.y);
        int base_z = (int)std::floor(base_coord.z);
        Vec3 fx = base_coord - Vec3((float)base_x, (float)base_y, (float)base_z);
        
        float wx[3], wy[3], wz[3];
        float dwx[3], dwy[3], dwz[3];
        
        calcWeightsLocal(fx.x, wx[0], wx[1], wx[2], dwx[0], dwx[1], dwx[2], inv_dx);
        calcWeightsLocal(fx.y, wy[0], wy[1], wy[2], dwy[0], dwy[1], dwy[2], inv_dx);
        calcWeightsLocal(fx.z, wz[0], wz[1], wz[2], dwz[0], dwz[1], dwz[2], inv_dx);
        
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    int gx = base_x + i;
                    int gy = base_y + j;
                    int gz = base_z + k;
                    if (!grid.contains(gx, gy, gz)) continue;
                    
                    Vec3 weight_grad(
                        dwx[i] * wy[j] * wz[k],
                        wx[i] * dwy[j] * wz[k],
                        wx[i] * wy[j] * dwz[k]
                    );
                    
                    Vec3 force = stress_force_term * weight_grad * -1.0f;
                    grid.getNode(gx, gy, gz).force += force;
                }
            }
        }
    }
    
    // Update Grid Velocities
    for (auto& node : grid.nodes) {
        if (node.is_active && node.mass > 0.0f) {
            node.force += gravity * node.mass;
            node.velocity_new = node.velocity + (node.force / node.mass) * fixed_dt;
            
            // Simple boundary collision
            if (node.velocity_new.y < 0.0f) {
                node.velocity_new.y = 0.0f;
            }
        }
    }
}

void Simulation::gridToParticle() {
    float inv_dx = 1.0f / grid.cell_size;
    float B_inv = 3.0f / (grid.cell_size * grid.cell_size); // For quadratic B-Spline
    
    for (auto& p : particles) {
        Vec3 base_coord = (p.position * inv_dx) - Vec3(0.5f, 0.5f, 0.5f);
        int base_x = (int)std::floor(base_coord.x);
        int base_y = (int)std::floor(base_coord.y);
        int base_z = (int)std::floor(base_coord.z);
        Vec3 fx = base_coord - Vec3((float)base_x, (float)base_y, (float)base_z);
        
        float wx[3], wy[3], wz[3];
        float dwx[3], dwy[3], dwz[3]; // Not used here, just needed for func
        
        calcWeightsLocal(fx.x, wx[0], wx[1], wx[2], dwx[0], dwx[1], dwx[2], inv_dx);
        calcWeightsLocal(fx.y, wy[0], wy[1], wy[2], dwy[0], dwy[1], dwy[2], inv_dx);
        calcWeightsLocal(fx.z, wz[0], wz[1], wz[2], dwz[0], dwz[1], dwz[2], inv_dx);
        
        Vec3 new_v(0, 0, 0);
        Mat3 new_C;
        
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    int gx = base_x + i;
                    int gy = base_y + j;
                    int gz = base_z + k;
                    if (!grid.contains(gx, gy, gz)) continue;
                    
                    float weight = wx[i] * wy[j] * wz[k];
                    Vec3 node_vel = grid.getNode(gx, gy, gz).velocity_new;
                    
                    new_v += node_vel * weight;
                    
                    Vec3 dpos(
                        ((float)gx - base_coord.x - fx.x + 0.5f) * grid.cell_size,
                        ((float)gy - base_coord.y - fx.y + 0.5f) * grid.cell_size,
                        ((float)gz - base_coord.z - fx.z + 0.5f) * grid.cell_size
                    );
                    
                    new_C = new_C + Mat3::outer_product(node_vel, dpos) * (weight * B_inv);
                }
            }
        }
        
        p.velocity = new_v;
        p.position += new_v * fixed_dt;
        p.C = new_C;
    }
}
