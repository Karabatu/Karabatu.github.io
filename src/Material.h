#pragma once
#include "Math.h"

enum class MaterialType {
    WOOD,
    CONCRETE,
    WATER,
    GAS
};

struct Material {
    MaterialType type;
    
    // Physical Parameters
    float density;
    float youngs_modulus;
    float poissons_ratio;
    
    // Fracture / Plasticity Parameters
    float yield_strength;
    float tensile_strength;
    
    // Lame Parameters (computed automatically)
    float lambda;
    float mu;
    
    // Custom parameter for wood (Anisotropy)
    Vec3 fiber_dir;

    Material(MaterialType t, float d, float E, float nu, float yield, float tensile) 
        : type(t), density(d), youngs_modulus(E), poissons_ratio(nu), 
          yield_strength(yield), tensile_strength(tensile), fiber_dir(Vec3(0,1,0)) 
    {
        // Compute Lame parameters
        // mu = E / (2 * (1 + nu))  -> Shear modulus
        // lambda = (E * nu) / ((1 + nu) * (1 - 2*nu))
        
        // Prevent division by zero or invalid parameters for fluids
        if (type == MaterialType::WATER || type == MaterialType::GAS) {
            mu = 0;
            lambda = E; // Use lambda as bulk modulus K for fluids
        } else {
            mu = E / (2.0f * (1.0f + nu));
            lambda = (E * nu) / ((1.0f + nu) * (1.0f - 2.0f * nu));
        }
    }
};
