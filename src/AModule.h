#pragma once
#include "Data.h"
#include <sstream>
#include <string>
#include "Timer.h"
#include <iostream>
class AModule
{
public:
  AModule(Data *data);
  virtual void Initialization() = 0;
  void RunProcessing();
  virtual std::string getModuleName() = 0;
  double getModuleWorkTime();

protected:
  virtual void Processing() = 0;

  Data *data = nullptr;

public:
  Timer moduleTimer;
};