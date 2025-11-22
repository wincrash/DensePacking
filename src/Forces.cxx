#include "Forces.h"

Forces::Forces(Data *data) : AModule(data) {}

std::string Forces::getModuleName() { return "Forces"; };

void Forces::Initialization()
{
}

void Forces::Processing()
{

  RunKernels();
}
void Forces::RunKernels()
{
  const auto simConstants = data->simConstants;
  int N = data->PARTICLE_COUNT;
  const auto CYLINDER_RADIUS = data->cylinder_radius;
  auto &POSITION = data->POSITION;
  auto &RADIUS = data->RADIUS;
  auto &VELOCITY = data->VELOCITY;
  auto &NN_COUNT = data->NN_COUNT;
  auto &NN_IDS = data->NN_IDS;
  auto &FIX = data->FIX;
  auto &MAX_OVERLAP = data->MAX_OVERLAP;
  auto WALL_MAX = data->WALL_MAX;
  auto WALL_MIN = data->WALL_MIN;
  Kokkos::parallel_for("FORCES", N, KOKKOS_LAMBDA(const int idx) {
    if (FIX(idx) != 0)
      return;
    int kiekis = NN_COUNT(idx);
    Vec3 DISP(0, 0, 0);
    Vec3 P1 = POSITION(idx);
    double RADIUS1 = RADIUS(idx);
    double maxas = 0;

    for (int i = 0; i < kiekis; i++)
    {
      int pid = NN_IDS(idx * simConstants.NN_MAX + i);
      Vec3 P2 = POSITION(pid);
      double RADIUS2 = RADIUS(pid);
      Vec3 n_ij = P1 - P2;
      double h_ij = RADIUS1 + RADIUS2 - n_ij.length();
      n_ij = n_ij.normalize();
      if (h_ij < 0)
        continue;
      Vec3 d = n_ij * h_ij * simConstants.relaxation_coefficient;
      DISP = DISP + d;
      if (maxas < h_ij)
        maxas = h_ij;
    }
    for (int i = 0; i < 6; i++)
    {
      double h_ij = 0;
      Vec3 n_ij = Vec3(0, 0, 0);
      switch (i)
      {
      case 0:
        n_ij.x = 1;
        h_ij = RADIUS1 - fabs(WALL_MIN.x - P1.x);
        break;
      case 1:
        n_ij.x = -1;
        h_ij = RADIUS1 - fabs(WALL_MAX.x - P1.x);
        break;
      case 2:
        n_ij.y = 1;
        h_ij = RADIUS1 - fabs(WALL_MIN.y - P1.y);
        break;
      case 3:
        n_ij.y = -1;
        h_ij = RADIUS1 - fabs(WALL_MAX.y - P1.y);
        break;
      case 4:
        n_ij.z = 1;
        h_ij = RADIUS1 - fabs(WALL_MIN.z - P1.z);
        break;
      case 5:
        n_ij.z = -1;
        h_ij = RADIUS1 - fabs(WALL_MAX.z - P1.z);
        break;
      default:
        break;
      }

      if (h_ij < 0)
        continue;
      Vec3 d = n_ij * h_ij * simConstants.relaxation_coefficient;
      DISP = DISP + d;
      if (maxas < h_ij)
        maxas = h_ij;
    }
    // double h_ij = (Kokkos::sqrt(P1.x * P1.x + P1.y * P1.y) + RADIUS1) - CYLINDER_RADIUS;
    // if (h_ij > 0)
    // {
    //   Vec3 n_ij = Vec3(-P1.x, -P1.y, 0);
    //   Vec3 d = n_ij * h_ij * simConstants.relaxation_coefficient;
    //   DISP = DISP + d;
    //   if (maxas < h_ij)
    //     maxas = h_ij;
    // }


    VELOCITY(idx) = DISP;
    MAX_OVERLAP(idx) = maxas;
    /////
  });
}
