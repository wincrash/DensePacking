#pragma once
#include "AModule.h"

class ContactSearch : public AModule
{
public:
  ContactSearch(Data *data);
  virtual void Initialization();
  virtual std::string getModuleName();
  void RunKernels();

protected:
  virtual void Processing();

private:
  Kokkos::View<int *> CELL_ID1;
  Kokkos::View<int *> PARTICLE_ID1;
  Kokkos::View<int *> STARTAS1;
  Kokkos::View<int *> ENDAS1;

  double CELL_SIZE1;
  double INV_CELL_SIZE1;
  int HASH_TABLE1;
  double SKIN1 = 1.1;
};