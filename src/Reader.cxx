#include "Reader.h"
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSetReader.h>

Reader::Reader(Data *data) : AModule(data) {}
std::string Reader::getModuleName() { return "Reader"; };

void Reader::Initialization()
{
  auto filename=this->data->yaml.ReadString("simulation","input");
    vtkPolyData *polyData=vtkPolyData::New() ;
  if (filename.length() >= 3 && filename.rfind("vtp") == (filename.length() - 3))
  {auto reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName(filename.c_str());
  reader->Update();
  //polyData = reader->GetOutput();
  polyData->DeepCopy(reader->GetOutput());
  }else 
  {
    auto reader = vtkSmartPointer<vtkDataSetReader>::New();
  reader->SetFileName(filename.c_str());
  reader->Update();
  polyData->DeepCopy(reader->GetOutput());
  //polyData = reader->GetOutput();

  std::cout<<"aaaa\n";

  }
    polyData->Print(std::cout);


  vtkPoints *points = polyData->GetPoints();
  vtkDataArray *radiusArray = polyData->GetPointData()->GetArray("RADIUS");
  data->PARTICLE_COUNT = points->GetNumberOfPoints();
  data->POSITION = Kokkos::View<Vec3 *>("POSITION", data->PARTICLE_COUNT);
  data->RADIUS = Kokkos::View<double *>("RADIUS", data->PARTICLE_COUNT);
  data->OLD_RADIUS = Kokkos::View<double *>("OLD_RADIUS", data->PARTICLE_COUNT);
  data->VELOCITY = Kokkos::View<Vec3 *>("VELOCITY", data->PARTICLE_COUNT);
  data->NN_COUNT = Kokkos::View<int *>("NN_COUNT", data->PARTICLE_COUNT);
  data->FIX = Kokkos::View<int *>("FIX", data->PARTICLE_COUNT);
data->MAX_OVERLAP = Kokkos::View<double *>("MAX_OVERLAP", data->PARTICLE_COUNT);
  
  
  auto POSITION_host = Kokkos::create_mirror_view(data->POSITION);
  auto RADIUS_host = Kokkos::create_mirror_view(data->RADIUS);
  auto OLD_RADIUS_host = Kokkos::create_mirror_view(data->OLD_RADIUS);
  auto VELOCITY_host = Kokkos::create_mirror_view(data->VELOCITY);  
  auto FIX_host = Kokkos::create_mirror_view(data->FIX);

  Kokkos::deep_copy(data->POSITION, Vec3{0.0, 0.0, 0.0});
  Kokkos::deep_copy(data->VELOCITY, Vec3{0.0, 0.0, 0.0});
  Kokkos::deep_copy(data->NN_COUNT, 0);
  Kokkos::deep_copy(data->FIX, 0);
  Kokkos::deep_copy(data->RADIUS, 0);
  Kokkos::deep_copy(data->OLD_RADIUS, 0);
  Kokkos::deep_copy(data->MAX_OVERLAP, 0);
  

  double min_radius = std::numeric_limits<double>::max();
  double max_radius = std::numeric_limits<double>::lowest();
  for (int i = 0; i < data->PARTICLE_COUNT; ++i)
  {
    double p[3];
    points->GetPoint(i, p);
    double r = radiusArray->GetTuple1(i);

    POSITION_host(i) = Vec3(p[0], p[1], p[2]);

    RADIUS_host(i) = r;
    OLD_RADIUS_host(i) = r;
    if (r < min_radius)
      min_radius = r;
    if (r > max_radius)
      max_radius = r;

    if (polyData->GetPointData()->HasArray("FIX"))
    {
      FIX_host(i) = polyData->GetPointData()->GetArray("FIX")->GetTuple1(i);
    }
    else
    {
      FIX_host(i) = 0;
    }
  }

  std::cout << "Min radius: " << min_radius << ", Max radius: " << max_radius << std::endl;
  data->simConstants.NN_MAX = (int)(4.0 * 0.74 * (min_radius + max_radius * 1.1) * (min_radius + max_radius * 1.1) / (min_radius * min_radius)) +1;
  data->simConstants.NN_MAX=data->simConstants.NN_MAX*2;
  
  std::cout << "NN max " << data->simConstants.NN_MAX << "\n";

  data->NN_IDS = Kokkos::View<int *>("NN_IDS", data->PARTICLE_COUNT * data->simConstants.NN_MAX);  
  Kokkos::deep_copy(data->NN_IDS, 0);

  Kokkos::deep_copy(data->POSITION, POSITION_host);
  Kokkos::deep_copy(data->RADIUS, RADIUS_host);
  Kokkos::deep_copy(data->OLD_RADIUS, OLD_RADIUS_host);
    Kokkos::deep_copy(data->VELOCITY, VELOCITY_host);
  Kokkos::deep_copy(data->FIX, FIX_host);
}

void Reader::Processing() {}
