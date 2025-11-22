#pragma once
#include "AModule.h"

class Integrator : public AModule
{
public:
  Integrator(Data *data);
  virtual void Initialization();
  virtual std::string getModuleName();
  void RunKernels();

protected:
  void Processing();
};