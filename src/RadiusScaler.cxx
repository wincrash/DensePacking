#include "RadiusScaler.h"
#include <Kokkos_StdAlgorithms.hpp>

RadiusScaler::RadiusScaler(Data *data) : AModule(data) {}

std::string RadiusScaler::getModuleName() { return "RadiusScaler"; };

void RadiusScaler::Initialization()
{
}

void RadiusScaler::Processing()
{
  RunKernels();
}
void RadiusScaler::RunKernels()
{
  const int N = data->PARTICLE_COUNT;
  const auto simConstants = data->simConstants;
  auto &RADIUS = data->RADIUS;
  auto &OLD_RADIUS = data->OLD_RADIUS;
  auto &FIX = data->FIX;

  if(data->simConstants.maxOverlap<data->simConstants.overlap_limit)
  {
    data->simConstants.radius_scale_delta_current+=data->simConstants.radius_scale_delta;
    data->simConstants.relaxation_coefficient=data->simConstants.relaxation_coefficient*data->simConstants.relaxation_coefficient_scale;

     Kokkos::parallel_for("RadiusScaler", N, KOKKOS_LAMBDA(const int idx) {
    if (FIX(idx) > 0)
      return;
      RADIUS(idx)=OLD_RADIUS(idx)*(1.0+simConstants.radius_scale_delta_current);
    //
  });
  }
}
