#pragma once
#include "AModule.h"

class Writer : public AModule
{
public:
  Writer(Data *data);
  virtual void Initialization();
  virtual std::string getModuleName();

protected:
  virtual void Processing();
};