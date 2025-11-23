#include "Time.h"

Time::Time(Data *data) : AModule(data) {}
std::string Time::getModuleName() { return "Time"; };

void Time::Initialization()
{
  data->COMPUTE = true;
  data->CONTACT_SEARCH = true;
  data->WRITE_RESULTS = true;
  data->PRINT_TIMES = true;

  this->END = data->yaml.ReadDouble("simulation", "total");
  this->WRITE_RESULTS_SKIP = data->yaml.ReadInt("simulation", "write_skip");
  this->PRINT_TIMES_SKIP = data->yaml.ReadInt("simulation", "print_skip");
  this->CONTACT_SEARCH_SKIP = data->yaml.ReadInt("simulation", "search_skip");


}

void Time::Processing()
{
  data->cstep++;
  data->PRINT_TIMES = (data->cstep % this->PRINT_TIMES_SKIP == 0);
  data->CONTACT_SEARCH = (data->cstep % this->CONTACT_SEARCH_SKIP == 0);
  data->WRITE_RESULTS = (data->cstep % this->WRITE_RESULTS_SKIP == 0);
  data->COMPUTE = (data->cstep <= this->END);
  //   if(data->simConstants.maxOverlap<data->simConstants.overlap_limit)
  // {
  //   data->WRITE_RESULTS=true;
  // }
  // else
  // {
  //   data->WRITE_RESULTS=false;
  // }
}
