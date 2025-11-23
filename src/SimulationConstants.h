#pragma once

#include "DataTypes.h"
namespace Constants
{
    constexpr double PI = 3.141592653589793; // Pi constant
}

struct SimulationConstants
{
    double initial_scale=1.0;
    double radius_scale_delta_current=0;
    double radius_scale_delta;
    double overlap_limit;
    double maxOverlap;
    int NN_MAX=32;
    double relaxation_coefficient;
    double relaxation_coefficient_scale;

};