#include "ContactSearch.h"
#include <Kokkos_Sort.hpp>

KOKKOS_INLINE_FUNCTION int GetHash(const int x, const int y, const int z, const int HASH_TABLE_SIZE)
{
  unsigned long code = 0;

  for (unsigned long i = 0; i < (sizeof(unsigned long) * 8) / 3; ++i)
  {
    code |= ((x & ((unsigned long)1 << i)) << 2 * i) |
            ((y & ((unsigned long)1 << i)) << (2 * i + 1)) |
            ((z & ((unsigned long)1 << i)) << (2 * i + 2));
  }

  return code % HASH_TABLE_SIZE;
}

ContactSearch::ContactSearch(Data *data) : AModule(data) {}

std::string ContactSearch::getModuleName() { return "ContactSearch"; };

void ContactSearch::Initialization()
{
  // Find min and max RADIUS
  double min_radius = std::numeric_limits<double>::max();
  double max_radius = std::numeric_limits<double>::lowest();
  int N = data->PARTICLE_COUNT;
  auto RADIUS = Kokkos::create_mirror_view(data->RADIUS);
  Kokkos::deep_copy(RADIUS, data->RADIUS);

  for (int i = 0; i < N; ++i)
  {
    double r = RADIUS(i);
    if (r < min_radius)
      min_radius = r;
    if (r > max_radius)
      max_radius = r;
  }

  CELL_SIZE1 = 2.0 * max_radius * SKIN1;
  INV_CELL_SIZE1 = 1.0 / CELL_SIZE1;
  HASH_TABLE1 = data->PARTICLE_COUNT * 2;

  this->CELL_ID1 = Kokkos::View<int *>("CELL_ID", data->PARTICLE_COUNT);
  this->PARTICLE_ID1 = Kokkos::View<int *>("CELL_ID", data->PARTICLE_COUNT);
  this->STARTAS1 = Kokkos::View<int *>("CELL_ID", HASH_TABLE1);
  this->ENDAS1 = Kokkos::View<int *>("CELL_ID", HASH_TABLE1);

  Kokkos::deep_copy(this->CELL_ID1, 0);
  Kokkos::deep_copy(this->PARTICLE_ID1, 0);
  Kokkos::deep_copy(this->STARTAS1, 0);
  Kokkos::deep_copy(this->ENDAS1, -1);
}

void ContactSearch::Processing()
{
  RunKernels();
}
void ContactSearch::RunKernels()
{
  if (!data->CONTACT_SEARCH)
    return;
  Kokkos::DefaultExecutionSpace space;
  Kokkos::deep_copy(data->NN_COUNT, 0);
  Kokkos::deep_copy(this->STARTAS1, 0);
  Kokkos::deep_copy(this->ENDAS1, -1);
  auto &POSITION = data->POSITION;
  auto &RADIUS = data->RADIUS;
  auto &NN_COUNT = data->NN_COUNT;
  auto &NN_IDS = data->NN_IDS;
  int N = data->PARTICLE_COUNT;
  int NN_MAX = data->simConstants.NN_MAX;
  auto WALL_MAX = data->WALL_MAX;
  auto WALL_MIN = data->WALL_MIN;
  auto INV_CELL_SIZE = this->INV_CELL_SIZE1;
  auto HASH_TABLE = this->HASH_TABLE1;
  auto SKIN = this->SKIN1;
  auto &STARTAS = this->STARTAS1;
  auto &ENDAS = this->ENDAS1;
  auto &PARTICLE_ID = this->PARTICLE_ID1;
  auto &CELL_ID = this->CELL_ID1;

  Kokkos::parallel_for("CALCULATE_HASH", N, KOKKOS_LAMBDA(const int idx) {
         Vec3 pos = POSITION(idx);
         pos.x=floor(pos.x*INV_CELL_SIZE);
         pos.y=floor(pos.y*INV_CELL_SIZE);
         pos.z=floor(pos.z*INV_CELL_SIZE);
        CELL_ID(idx) = GetHash((int)pos.x, (int)pos.y, (int)pos.z,HASH_TABLE);
        PARTICLE_ID(idx) = idx; });

  Kokkos::Experimental::sort_by_key(space, CELL_ID, PARTICLE_ID);

  Kokkos::parallel_for("START_END", N, KOKKOS_LAMBDA(const int idx) {
        if(idx!=0)
        {
      int bb = CELL_ID(idx - 1);
      int cc = CELL_ID(idx);
      if (bb != cc) {
        ENDAS(bb) = idx;
        STARTAS(cc) = idx;
      }
      if (idx == (N - 1)) {
        ENDAS(cc) = idx + 1;
      }
        } });

  Kokkos::parallel_for("FIND_NEIGHBOURS", N, KOKKOS_LAMBDA(const int idx) {
    Vec3 POINT = POSITION(idx);
    double radius = RADIUS(idx);
    int CX = (int)floor(POINT.x * INV_CELL_SIZE);
    int CY = (int)floor(POINT.y * INV_CELL_SIZE);
    int CZ = (int)floor(POINT.z * INV_CELL_SIZE);

    int cell_IDS[27];
    int c_id = 0;
    for (int i = CX - 1; i <= CX + 1; i++)
      for (int j = CY - 1; j <= CY + 1; j++)
        for (int k = CZ - 1; k <= CZ + 1; k++)
        {
          int hash = GetHash(i, j, k, HASH_TABLE);
          int yra = 0;
          for (int h = 0; h < c_id; h++)
          {
            if (cell_IDS[h] == hash)
              yra = 1;
          }
          if (yra == 0)
          {
            cell_IDS[c_id] = hash;
            c_id++;
          }
        }
    int count = 0;
    for (int i = 0; i < c_id; i++)
    {
      int hash = cell_IDS[i];
      int startas = STARTAS(hash);
      int endas = ENDAS(hash);
      for (int h = startas; h < endas; h++)
      {
        int pid = PARTICLE_ID(h);
        if(pid==idx)continue;
        // if (pid <= idx)
        //   continue;

        Vec3 P2 = POSITION(pid);
        double R2 = RADIUS(pid);

        Vec3 diff = P2 - POINT;
        double ilgis = radius * SKIN + R2 - diff.length();
        if (ilgis > -1.0E-8)
        {

          NN_IDS(idx * NN_MAX + count) = pid;
          count++;
          if (count >= NN_MAX)
            printf("INCREASE NN_MAX VALUE !!! NN_MAX %d COUNT %d\n", NN_MAX,
                   count);
        }
      }
    }
    NN_COUNT(idx) = count; });
}
