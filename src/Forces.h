#pragma once
#include "AModule.h"

class Forces : public AModule
{
public:
  Forces(Data *data);
  virtual void Initialization();
  virtual std::string getModuleName();
  void RunKernels();

protected:
  virtual void Processing();
};