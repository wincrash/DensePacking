#include "RadiusScaler.h"
#include <iomanip>

RadiusScaler::RadiusScaler(Data *data) : AModule(data) {}

std::string RadiusScaler::getModuleName() { return "RadiusScaler"; };

void RadiusScaler::Initialization()
{
  radius_min=data->min_radius;
}

void RadiusScaler::Processing()
{
  RunKernels();
}
void RadiusScaler::RunKernels()
{
  //if (data->cstep % 100 != 0)return;
  
  const int N = data->PARTICLE_COUNT;
  auto &RADIUS = data->RADIUS;
  auto &OLD_RADIUS = data->OLD_RADIUS;
  auto &FIX = data->FIX;


  if (data->simConstants.maxOverlap > data->simConstants.overlap_limit)
    return;

  // Advance the count of scaling steps once per call.
  kiekis++;
 
 

      // Update cumulative scale and compute radius_min from the baseline.
      const double cumulative_scale = data->simConstants.radius_scale_delta_current;
      radius_min = data->min_radius * (1.0 + cumulative_scale);

      std::cout << data->cstep << " Max overlap: " << std::setprecision(5) << std::scientific << data->simConstants.maxOverlap
            << " radius_min " << radius_min << "\n";
  ///i//f(data->cstep%100==0)
  {
    data->simConstants.radius_scale_delta_current += data->simConstants.radius_scale_delta;
    data->simConstants.relaxation_coefficient = data->simConstants.relaxation_coefficient * data->simConstants.relaxation_coefficient_scale;

    // Use a simple cumulative additive scale computed from simConstants.
    // This makes the radius change linear with the cumulative scale instead
    // of multiplying by a factor that grows with the iteration count.
    const double cumulative_scale_after = data->simConstants.radius_scale_delta_current;

    Kokkos::parallel_for("RadiusScaler", N, KOKKOS_LAMBDA(const int idx) {
      if (FIX(idx) > 0)
        return;
      RADIUS(idx) = OLD_RADIUS(idx) * (1.0 + cumulative_scale_after);
    });
  }
  Kokkos::fence();
}
