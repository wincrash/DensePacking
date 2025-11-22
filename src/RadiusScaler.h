#pragma once
#include "AModule.h"

class RadiusScaler : public AModule
{
public:
  RadiusScaler(Data *data);
  virtual void Initialization();
  virtual std::string getModuleName();
  void RunKernels();

protected:
  void Processing();
};