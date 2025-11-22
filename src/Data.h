#pragma once
#include "DataTypes.h"
#include "SimulationConstants.h"
#include "YamlAPI.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#define MAX_MATERIALS 3

class Data
{
public:
  SimulationConstants simConstants;
  YamlAPI yaml;
  YAML::Node config = YAML::LoadFile("config.yaml");
  unsigned long total_steps=1000;
  unsigned long cstep = 0;
  bool COMPUTE = true;
  bool CONTACT_SEARCH = true;
  bool WRITE_RESULTS = true;
  bool PRINT_TIMES = true;
  Vec3 WALL_MIN;
  Vec3 WALL_MAX;
  double cylinder_radius=1E12;
  int PARTICLE_COUNT = 0;

  void initialize();
  Kokkos::View<Vec3 *> POSITION;
  Kokkos::View<double *> RADIUS;
  Kokkos::View<double *> MAX_OVERLAP;
  Kokkos::View<double *> OLD_RADIUS;
  Kokkos::View<int *> NN_COUNT;
  Kokkos::View<int *> NN_IDS;
  Kokkos::View<Vec3 *> VELOCITY;  
  Kokkos::View<int *> FIX;
  
};
