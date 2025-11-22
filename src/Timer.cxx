#include "Timer.h"

#include <ctime>
#include <iostream>
#include <numeric>
#include <ratio>
#include <sstream>

Timer::Timer() {}
void Timer::Start() { start = Clock::now(); }

void Timer::Stop()
{
  std::chrono::duration<double> time_span =
      std::chrono::duration_cast<std::chrono::duration<double>>(Clock::now() -
                                                                start);
  times.push_back(time_span.count());
}
void Timer::CalculateAVG()
{
  this->sumTime = 0;
  this->avgTime = 0;
  if (times.size() > 0)
  {
    this->sumTime = std::accumulate(times.begin(), times.end(), 0.0);
    this->avgTime = this->sumTime / times.size();
  }
  times.resize(0, 0);
  times.reserve(1000);
}
