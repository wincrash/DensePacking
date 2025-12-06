#include <Kokkos_Core.hpp>
#include "Data.h"
#include <iomanip> // for std::setw, std::left, etc.
#include "ContactSearch.h"
#include "Forces.h"
#include "Reader.h"
#include "Time.h"
#include "Writer.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "Integrator.h"
#include "RadiusScaler.h"

int main(int argc, char *argv[])
{
  Kokkos::initialize();
  {

    // Print backend information
    std::cout << "\n[Kokkos] Running on: ";
#ifdef KOKKOS_ENABLE_CUDA
    std::cout << "NVIDIA GPU (CUDA)\n";
#elif defined(KOKKOS_ENABLE_HIP)
    std::cout << "AMD GPU (HIP)\n";
#elif defined(KOKKOS_ENABLE_OPENMP)
    std::cout << "CPU (OpenMP)\n";
#elif defined(KOKKOS_ENABLE_SERIAL)
    std::cout << "CPU (Serial)\n";
#else
    std::cout << "Unknown backend\n";
#endif

    Data data;
    data.initialize();
    Reader reader(&data);
    reader.Initialization();

    std::vector<AModule *> modules;
    modules.push_back(new RadiusScaler(&data));
    modules.push_back(new ContactSearch(&data));
    modules.push_back(new Forces(&data));
    modules.push_back(new Integrator(&data));
    modules.push_back(new Time(&data));
//    modules.push_back(new Logs(&data));


    modules.push_back(new Writer(&data));

    for (int i = 0; i < modules.size(); ++i)
    {
      modules[i]->Initialization();
    }

    // Determine column widths once (you can adjust these as needed)
    const int timeWidth = 16;
    const int stepWidth = 16;
    const int moduleWidth = 16; // per module column
    const int totalWidth = 16;

    // Print header to console with nice columns
    std::cout << std::left << std::setw(timeWidth) << "OVERLAP"
          << std::setw(timeWidth) << "R_SCALE_DELTA"
          << std::setw(timeWidth) << "RELAX_COEFF"
          << std::setw(stepWidth) << "STEP";
    for (int i = 0; i < modules.size(); ++i)
    {
      std::cout << std::setw(moduleWidth) << modules[i]->getModuleName();
    }
    std::cout << std::setw(totalWidth) << "Total" << "\n";

    // Write CSV header as before
    std::stringstream csvHeader;
    csvHeader << "STEP;OVERLAP;RADIUS_SCALE_DELTA;RELAXATION_COEFFICIENT";
    for (int i = 0; i < modules.size(); ++i)
    {
      csvHeader << ";" << modules[i]->getModuleName();
    }
    csvHeader << ";Total\n";

    std::ofstream file("timers.csv");
    file << csvHeader.str();
    file.close();


    while (data.COMPUTE)
    {
      for (int i = 0; i < modules.size(); ++i)
      {
        modules[i]->RunProcessing();
      }

      if (data.PRINT_TIMES)
      {
        // Prepare timing data
        double total = 0.0;
        std::vector<double> times(modules.size());
        for (int i = 0; i < modules.size(); ++i)
        {
          times[i] = modules[i]->getModuleWorkTime();
          total += times[i];
        }

        // Console output: nicely formatted columns
        std::cout << std::fixed << std::setprecision(6);
        {
          std::ostringstream ossOverlap, ossRadius, ossRelax;
          ossOverlap << std::scientific << std::setprecision(6) << data.simConstants.maxOverlap;
          ossRadius << std::scientific << std::setprecision(6) << data.simConstants.radius_scale_delta_current;
          ossRelax << std::scientific << std::setprecision(6) << data.simConstants.relaxation_coefficient;
          std::cout << std::left << std::setw(timeWidth) << ossOverlap.str()
                    << std::setw(timeWidth) << ossRadius.str()
                    << std::setw(timeWidth) << ossRelax.str()
                    << std::setw(stepWidth) << data.cstep;
        }
                  
        for (double t : times)
        {
          std::cout << std::setw(moduleWidth) << t;
        }
        std::cout << std::setw(totalWidth) << total << "\n";
        std::cout.flush();

        // CSV output: unchanged (semicolon-separated)
        std::stringstream csvLine;
        csvLine << data.cstep << ";" << data.simConstants.maxOverlap << ";" << data.simConstants.radius_scale_delta_current << ";" << data.simConstants.relaxation_coefficient;
        for (double t : times)
        {
          csvLine << ";" << t;
        }
        csvLine << ";" << total << "\n";

        std::ofstream file("timers.csv", std::ios_base::app);
        file << csvLine.str();
        file.close();
      }
    }
  }
  Kokkos::finalize();
  return 0;
}
