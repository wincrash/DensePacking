#pragma once
#include <chrono>
#include <string>
#include <vector>
class Timer {
public:  
  Timer();
  void Start();
  void Stop();
  void CalculateAVG();

private:
  typedef std::chrono::high_resolution_clock Clock;
  Clock::time_point start;
  std::vector<double> times;

  public:
    double sumTime=0;
    double avgTime=0;
};