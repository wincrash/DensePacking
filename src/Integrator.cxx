#include "Integrator.h"
#include <Kokkos_StdAlgorithms.hpp>

Integrator::Integrator(Data *data) : AModule(data) {}

std::string Integrator::getModuleName() { return "Integrator"; };

void Integrator::Initialization()
{
}

void Integrator::Processing()
{
  RunKernels();
}
void Integrator::RunKernels()
{
  const auto simConstants = data->simConstants;
  const int N = data->PARTICLE_COUNT;
  auto &POSITION = data->POSITION;
  auto &RADIUS = data->RADIUS;
  auto &VELOCITY = data->VELOCITY;
  auto &FORCE = data->FORCE;
  auto &FIX = data->FIX;

  Kokkos::parallel_for("INTEGRATION", N, KOKKOS_LAMBDA(const int idx) {
    if (FIX(idx) > 0)
      return;
    Vec3 pos = POSITION(idx);
    Vec3 vel = VELOCITY(idx);
    pos+=vel;
    POSITION(idx) = pos;
  });
    Kokkos::fence();

     auto MAX_OVERLAP_host = Kokkos::create_mirror_view(data->MAX_OVERLAP);
    Kokkos::deep_copy(MAX_OVERLAP_host, data->MAX_OVERLAP);
    Kokkos::fence();
    double max_val = 0.0;
    for (int i = 0; i < N; ++i) {
      if (MAX_OVERLAP_host(i) > max_val) max_val = MAX_OVERLAP_host(i);
    }
    data->simConstants.maxOverlap = max_val;

}
