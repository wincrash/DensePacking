#include "Data.h"

void Data::initialize()
{
    auto wmin_node = yaml.ReadDoubleArray("constrains", "walls_min");
    this->WALL_MIN.x = wmin_node[0];
    this->WALL_MIN.y = wmin_node[1];
    this->WALL_MIN.z = wmin_node[2];

    auto wmax_node = yaml.ReadDoubleArray("constrains", "walls_max");
    this->WALL_MAX.x = wmax_node[0];
    this->WALL_MAX.y = wmax_node[1];
    this->WALL_MAX.z = wmax_node[2];

    this->cylinder_radius= yaml.ReadDouble("constrains", "cylinder_radius");

    this->simConstants.radius_scale_delta=yaml.ReadDouble("simulation","radius_scale_delta");
    this->simConstants.overlap_limit=yaml.ReadDouble("simulation","overlap_limit");
    this->simConstants.relaxation_coefficient=yaml.ReadDouble("simulation","relaxation_coefficient");
    this->simConstants.relaxation_coefficient_scale=yaml.ReadDouble("simulation","relaxation_coefficient_scale");
    this->simConstants.initial_scale=yaml.ReadDouble("simulation","initial_scale");


    

    
    

    

}