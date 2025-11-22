#include "AModule.h"

AModule::AModule(Data *data)
{
  this->data = data;
}

void AModule::RunProcessing()
{
  // std::cout<<"RunProcessing Start "<<this->getModuleName()<<"\n";
  moduleTimer.Start();
  this->Processing();
  Kokkos::fence();
  moduleTimer.Stop();
  // std::cout<<"RunProcessing Stop "<<this->getModuleName()<<"\n";
}

double AModule::getModuleWorkTime()
{
  moduleTimer.CalculateAVG();
  return moduleTimer.sumTime;
}
