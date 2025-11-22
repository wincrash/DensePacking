#pragma once
#include "AModule.h"

class Time : public AModule
{
public:
  Time(Data *data);
  virtual void Initialization();
  virtual std::string getModuleName();

protected:
  virtual void Processing();
  double WALL_SPEED[6];
  int speed_startup;
  int END = 1;
  int CONTACT_SEARCH_SKIP = 1000;
  int WRITE_RESULTS_SKIP = 1000;
  int PRINT_TIMES_SKIP = 1000;
};