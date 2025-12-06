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

  // Calculate max overlap on CPU to avoid GPU portability issues
  if ((data->cstep % 50 == 0) && N > 0 && data->MAX_OVERLAP.extent(0) > 0)
  {
    auto MAX_OVERLAP_host = Kokkos::create_mirror_view(data->MAX_OVERLAP);
    Kokkos::deep_copy(MAX_OVERLAP_host, data->MAX_OVERLAP);
    double max_val = 0.0;
    for (int i = 0; i < N; ++i) {
      if (MAX_OVERLAP_host(i) > max_val) max_val = MAX_OVERLAP_host(i);
    }
    data->simConstants.maxOverlap = max_val;
  }else
  {
    data->simConstants.maxOverlap=simConstants.overlap_limit*1.1;
  }



// // Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace()> policy(0, N);
// //   auto res = Kokkos::max_element(policy, data->MAX_OVERLAP);
//   auto res = Kokkos::Experimental::max_element(Kokkos::DefaultExecutionSpace(), Kokkos::Experimental::begin(data->MAX_OVERLAP), Kokkos::Experimental::end(data->MAX_OVERLAP));
//   //std::cout<<"res "<<*res.data()<<"\n";
//   data->simConstants.maxOverlap=*res.data();

}
