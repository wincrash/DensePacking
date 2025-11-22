#pragma once
#include "AModule.h"

class Reader : public AModule {
public:
  Reader(Data *data);
  virtual void Initialization();
  virtual std::string getModuleName();

protected:
  virtual void Processing();
};