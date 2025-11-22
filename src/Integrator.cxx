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
  const int N = data->PARTICLE_COUNT;
  const auto simConstants = data->simConstants;
  auto &POSITION = data->POSITION;
  auto &RADIUS = data->RADIUS;
  auto &OLD_RADIUS = data->OLD_RADIUS;
  auto &VELOCITY = data->VELOCITY;
  auto &FIX = data->FIX;

  Kokkos::parallel_for("INTEGRATION", N, KOKKOS_LAMBDA(const int idx) {
    if (FIX(idx) > 0)
      return;
    Vec3 pos = POSITION(idx);
    Vec3 vel = VELOCITY(idx);
    POSITION(idx) = pos + vel;
    //
  });


// Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace()> policy(0, N);
//   auto res = Kokkos::max_element(policy, data->MAX_OVERLAP);
  auto res = Kokkos::Experimental::max_element(Kokkos::DefaultExecutionSpace(), Kokkos::Experimental::begin(data->MAX_OVERLAP), Kokkos::Experimental::end(data->MAX_OVERLAP));
  //std::cout<<"res "<<*res.data()<<"\n";
  data->simConstants.maxOverlap=*res.data();

}
