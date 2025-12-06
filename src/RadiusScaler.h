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
  double radius_min=0;
  int kiekis=0;
};