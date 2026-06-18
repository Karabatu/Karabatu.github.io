#pragma once
#include "Math.h"

struct GasTank {
    float volume_liters;
    float current_pressure_pa;
    float nozzle_diameter_m;
    float gas_density; // Base density at standard pressure (e.g. 1 atm)
    
    Vec3 nozzle_position;
    Vec3 nozzle_direction;

    GasTank() : volume_liters(100.0f), current_pressure_pa(101325.0f), 
                nozzle_diameter_m(0.05f), gas_density(1.225f),
                nozzle_position(0, 0, 0), nozzle_direction(1, 0, 0) {}

    // Computes the total mass of the gas using ideal gas law
    // Assuming base gas_density is given at 101325 Pa (1 atm)
    float getGasMass() const {
        float volume_m3 = volume_liters * 0.001f;
        // m = V * rho * (P / P0)
        return volume_m3 * gas_density * (current_pressure_pa / 101325.0f);
    }
};
