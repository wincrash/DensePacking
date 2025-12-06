#include "Writer.h"
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <vector>

// VTK Includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>



Writer::Writer(Data *data) : AModule(data) {}
std::string Writer::getModuleName() { return "Writer"; };

void Writer::Initialization()
{
    namespace fs = std::filesystem;
    const std::string dir = "data";
    // Using fs::create_directories handles creation even if parent directories don't exist.
    // However, the original logic was to clean the directory, so we keep that.
    
    if (fs::exists(dir))
    {
        fs::remove_all(dir);
    }
    fs::create_directory(dir);
}

void Writer::Processing()
{
if(data->simConstants.maxOverlap>data->simConstants.overlap_limit)return;
    //if (data->WRITE_RESULTS  )
        {

    const int N = data->PARTICLE_COUNT;
    const int MIN_COORD_NUM = 0; // Filter threshold for stable particles

    // --- 1. Copy Data from Device to Host Mirrors (Consolidated) ---
    // Using auto& for mirrors for clarity.
    auto POSITION = Kokkos::create_mirror_view(data->POSITION);
    auto RADIUS = Kokkos::create_mirror_view(data->RADIUS);
    auto NN_COUNT = Kokkos::create_mirror_view(data->NN_COUNT);
    auto NN_IDS = Kokkos::create_mirror_view(data->NN_IDS);
    auto FIX = Kokkos::create_mirror_view(data->FIX);
    auto MAX_OVERLAP = Kokkos::create_mirror_view(data->MAX_OVERLAP);

    Kokkos::deep_copy(POSITION, data->POSITION);
    Kokkos::deep_copy(RADIUS, data->RADIUS);
    // NN_COUNT mirror must be populated from device as well — missing copy caused empty sets on GPU
    Kokkos::deep_copy(NN_COUNT, data->NN_COUNT);
    Kokkos::deep_copy(NN_IDS, data->NN_IDS);
    Kokkos::deep_copy(FIX, data->FIX);
    Kokkos::deep_copy(MAX_OVERLAP, data->MAX_OVERLAP);
    
    // --- 2. Calculate Coordination Number (Z) ---
    std::vector<int> cnumber(N, 0);
    for (int i = 0; i < N; ++i)
    {
        int kiekis = NN_COUNT(i); // Use operator() for Kokkos view mirrors
        Vec3 P1 = POSITION(i);
        double R1 = RADIUS(i);

        for (int z = 0; z < kiekis; ++z)
        {
            int pid = NN_IDS(i * data->simConstants.NN_MAX + z);
            
            // Only calculate for i < pid to count each bond once
            if (i >= pid)
                continue;

            Vec3 P2 = POSITION(pid);
            double R2 = RADIUS(pid);
            double distance = (P1 - P2).length();
            double overlapas = R1 + R2 - distance;
            
            // Bond condition check
            if (overlapas > -data->simConstants.maxOverlap)
            {
                cnumber[i]++;
                cnumber[pid]++;
            }
        }
    }

    // --- 3. Filter Particles and Create Index Map ---
    std::vector<int> old_to_new_index(N, -1);
    std::vector<int> particles_to_keep;
    int N_filtered = 0;

    for (int i = 0; i < N; ++i)
    {
        if (cnumber[i] >= MIN_COORD_NUM)
        {
            particles_to_keep.push_back(i);
            old_to_new_index[i] = N_filtered;
            N_filtered++;
        }
    }

    // --- 4. Initialize and Populate VTK Objects with Filtered Data ---
    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(N_filtered); // Set size immediately

    // Lambda to create and configure a VTK array (DRY Principle)
    auto create_array = [&](const char* name, int components) {
        auto arr = vtkSmartPointer<vtkDoubleArray>::New();
        arr->SetName(name);
        arr->SetNumberOfComponents(components);
        arr->SetNumberOfTuples(N_filtered);
        return arr;
    };
    // Lambda for Int Array
    auto create_int_array = [&](const char* name, int components) {
        auto arr = vtkSmartPointer<vtkIntArray>::New();
        arr->SetName(name);
        arr->SetNumberOfComponents(components);
        arr->SetNumberOfTuples(N_filtered);
        return arr;
    };

    auto radiusArray = create_array("RADIUS", 1);
    auto MAX_OVERLAPArray = create_array("MAX_OVERLAP", 1);
    auto fixArray = create_int_array("FIX", 1);
    auto coordNRArray = create_int_array("COORDINATION_NUMBER", 1);
    

    // Populate filtered arrays
    for (int j = 0; j < N_filtered; ++j)
    {
        int i = particles_to_keep[j]; // Original index

        // Use SetPoint and SetTupleX methods for efficiency when size is known
        Vec3 pos = POSITION(i);
        points->SetPoint(j, pos.x, pos.y, pos.z);
        radiusArray->SetValue(j, RADIUS(i));
        fixArray->SetValue(j, FIX(i));
        MAX_OVERLAPArray->SetValue(j, MAX_OVERLAP(i));
        coordNRArray->SetValue(j, cnumber[i]); // Use SetValue/SetTuple1
        
    }

    // --- 5. Recalculate and Insert Bonds (Lines/Cells) with New Indices ---
    for (int i = 0; i < N; ++i)
    {
        if (old_to_new_index[i] != -1) // If particle i was kept
        {
            int new_i = old_to_new_index[i];
            
            // Loop through neighbors (NN_IDS is the original neighbor list)
            for (int z = 0; z < NN_COUNT(i); ++z)
            {
                int pid = NN_IDS(i * data->simConstants.NN_MAX + z);
                
                // Check if neighbor (pid) was kept AND avoid double counting (i < pid)
                if (old_to_new_index[pid] != -1 && i < pid)
                {
                    int new_pid = old_to_new_index[pid];
                    
                    // Recalculate overlap for safety (or reuse the logic from step 2 if possible)
                    double distance = (POSITION(i) - POSITION(pid)).length();
                    double overlapas = RADIUS(i) + RADIUS(pid) - distance;

                    if (overlapas > -data->simConstants.maxOverlap)
                    {
                        cells->InsertNextCell(2);
                        cells->InsertCellPoint(new_i); // Filtered index
                        cells->InsertCellPoint(new_pid); // Filtered index
                    }
                }
            }
        }
    }

    // --- 6. Final VTK Setup and Write ---
    polyData->SetLines(cells);
    polyData->SetPoints(points);

    // Add Point Data Arrays
    polyData->GetPointData()->SetScalars(radiusArray);
    polyData->GetPointData()->AddArray(fixArray);
    polyData->GetPointData()->AddArray(MAX_OVERLAPArray);
    polyData->GetPointData()->AddArray(coordNRArray);
    

    // Write to file
    std::stringstream stepParticles;
    stepParticles << "data/PARTICLES_" 
                  << std::setfill('0') << std::setw(10) << this->data->cstep << ".vtp";
    
    auto writerParticles = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    writerParticles->SetFileName(stepParticles.str().c_str());
    writerParticles->SetInputData(polyData);
    // Use try/catch or status check for robust file writing in production code
    writerParticles->Write(); 
    }


}