#include <iostream>
#include "Simulation.h"

int main() {
    std::cout << "Initializing MPM Gas Engine Simulation..." << std::endl;
    
    // Create a simulation: 0.1m cell size, 100x100x100 grid (10m x 10m x 10m)
    Simulation sim(0.1f, 100, 100, 100);
    
    // Set up a concrete wall (for example)
    Material* concrete = new Material(MaterialType::CONCRETE, 2400.0f, 3e10f, 0.2f, 3e6f, 3e6f);
    
    // Spawn a block of concrete particles
    for (int x = 40; x < 60; ++x) {
        for (int y = 0; y < 50; ++y) {
            for (int z = 45; z < 55; ++z) {
                Particle p;
                p.position = Vec3(x * 0.1f, y * 0.1f, z * 0.1f);
                p.velocity = Vec3(0, 0, 0);
                p.mass = concrete->density * (0.1f * 0.1f * 0.1f);
                p.initial_volume = 0.1f * 0.1f * 0.1f;
                p.material = concrete;
                sim.particles.push_back(p);
            }
        }
    }
    
    // Aim gas tank at the wall
    sim.tank.nozzle_position = Vec3(2.0f, 2.5f, 5.0f);
    sim.tank.nozzle_direction = Vec3(1.0f, 0.0f, 0.0f);
    
    std::cout << "Starting Simulation Loop..." << std::endl;
    std::cout << "Initial particles: " << sim.particles.size() << std::endl;
    
    // Simulate 10 frames of 1/60th of a second
    for (int frame = 0; frame < 10; ++frame) {
        sim.update(1.0f / 60.0f);
        std::cout << "Frame " << frame + 1 << " completed. Total particles: " << sim.particles.size() << std::endl;
    }
    
    std::cout << "Simulation completed successfully." << std::endl;
    delete concrete;
    return 0;
}
