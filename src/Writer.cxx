#include "Writer.h"
#include <filesystem>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkInformation.h>
#include <vtkCellData.h>
Writer::Writer(Data *data) : AModule(data) {}
std::string Writer::getModuleName() { return "Writer"; };

void Writer::Initialization()
{
  namespace fs = std::filesystem;
  const std::string dir = "data";
  // Remove directory if it exists
  if (fs::exists(dir))
  {
    fs::remove_all(dir);
  }
  // Create new directory
  fs::create_directory(dir);
  
}

void Writer::Processing()
{
  if (!data->WRITE_RESULTS)
    return;

  int N = data->PARTICLE_COUNT;
  auto POSITION = Kokkos::create_mirror_view(data->POSITION);
  auto VELOCITY = Kokkos::create_mirror_view(data->VELOCITY);  
  auto RADIUS = Kokkos::create_mirror_view(data->RADIUS);
  auto NN_COUNT = Kokkos::create_mirror_view(data->NN_COUNT);
  auto NN_IDS = Kokkos::create_mirror_view(data->NN_IDS);
  auto FIX = Kokkos::create_mirror_view(data->FIX);
  auto MAX_OVERLAP=Kokkos::create_mirror_view(data->MAX_OVERLAP);
  Kokkos::deep_copy(POSITION, data->POSITION);
  Kokkos::deep_copy(VELOCITY, data->VELOCITY);

  Kokkos::deep_copy(RADIUS, data->RADIUS);

  Kokkos::deep_copy(NN_COUNT, data->NN_COUNT);
  Kokkos::deep_copy(NN_IDS, data->NN_IDS);
  Kokkos::deep_copy(FIX, data->FIX);
  Kokkos::deep_copy(MAX_OVERLAP, data->MAX_OVERLAP);

  auto polyData = vtkSmartPointer<vtkPolyData>::New();

  // Create VTK objects
  auto cells = vtkSmartPointer<vtkCellArray>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();


  auto velArray = vtkSmartPointer<vtkDoubleArray>::New();
  velArray->SetName("VELOCITY");
  velArray->SetNumberOfComponents(3);
  velArray->SetNumberOfTuples(N);

  auto radiusArray = vtkSmartPointer<vtkDoubleArray>::New();
  radiusArray->SetName("RADIUS");
  radiusArray->SetNumberOfComponents(1);
  radiusArray->SetNumberOfTuples(N);
  auto MAX_OVERLAPArray = vtkSmartPointer<vtkDoubleArray>::New();
  MAX_OVERLAPArray->SetName("MAX_OVERLAP");
  MAX_OVERLAPArray->SetNumberOfComponents(1);
  MAX_OVERLAPArray->SetNumberOfTuples(N);

  auto nnCountArray = vtkSmartPointer<vtkIntArray>::New();
  nnCountArray->SetName("NN_COUNT");
  nnCountArray->SetNumberOfComponents(1);
  nnCountArray->SetNumberOfTuples(N);

  auto fixArray = vtkSmartPointer<vtkIntArray>::New();
  fixArray->SetName("FIX");
  fixArray->SetNumberOfComponents(1);
  fixArray->SetNumberOfTuples(N);

  auto coordNRArray = vtkSmartPointer<vtkIntArray>::New();
  coordNRArray->SetName("COORDINATION_NUMBER");
  coordNRArray->SetNumberOfComponents(1);
  coordNRArray->SetNumberOfTuples(N);


  for (int i = 0; i < N; ++i)
  {
    
    Vec3 pos = POSITION(i);
    points->InsertNextPoint(pos.x, pos.y, pos.z);
    Vec3 vel = VELOCITY(i);
    velArray->SetTuple3(i, vel.x, vel.y, vel.z);
   
    radiusArray->SetValue(i, RADIUS(i));
   
    nnCountArray->SetValue(i, NN_COUNT(i));
    fixArray->SetValue(i, FIX(i));
   MAX_OVERLAPArray->SetValue(i,MAX_OVERLAP(i));
    // cells->InsertNextCell(1);
    // cells->InsertCellPoint(i);
  }
    // polyData->SetVerts(cells);
std::vector<int> cnumber(N,0);
  for(int i=0;i<N;i++)
  {
    int kiekis=NN_COUNT[i];
    Vec3 P1=POSITION(i);
    double R1=RADIUS(i);
    for(int z=0;z<kiekis;z++)
    {
      int pid=NN_IDS[i*data->simConstants.NN_MAX+z];
      if(i>=pid)continue;
      Vec3 P2=POSITION(pid);
      double R2=RADIUS(pid);
      double overlapas=R1+R2-(P1-P2).length();
      if(overlapas>-data->simConstants.maxOverlap)
      {
        cells->InsertNextCell(2);
        cells->InsertCellPoint(i);
        cells->InsertCellPoint(pid);
        cnumber[i]++;
        cnumber[pid]++;

      }
    }
  }

    for(int i=0;i<N;i++)
  {
    coordNRArray->SetTuple1(i,cnumber[i]);
  }

  polyData->SetLines(cells);
  polyData->SetPoints(points);

  polyData->GetPointData()->SetScalars(radiusArray);
  polyData->GetPointData()->SetVectors(velArray);
  polyData->GetPointData()->AddArray(nnCountArray);
  polyData->GetPointData()->AddArray(fixArray);
  polyData->GetPointData()->AddArray(MAX_OVERLAPArray);
  polyData->GetPointData()->AddArray(coordNRArray);

  Vec3 min = data->WALL_MIN;
  Vec3 max = data->WALL_MAX;
  auto boxPoints = vtkSmartPointer<vtkPoints>::New();
  boxPoints->InsertNextPoint(min.x, min.y, min.z); // 0
  boxPoints->InsertNextPoint(max.x, min.y, min.z); // 1
  boxPoints->InsertNextPoint(max.x, max.y, min.z); // 2
  boxPoints->InsertNextPoint(min.x, max.y, min.z); // 3
  boxPoints->InsertNextPoint(min.x, min.y, max.z); // 4
  boxPoints->InsertNextPoint(max.x, min.y, max.z); // 5
  boxPoints->InsertNextPoint(max.x, max.y, max.z); // 6
  boxPoints->InsertNextPoint(min.x, max.y, max.z); // 7
  // 6 faces of the box as quads
  int faces[6][4] = {
      {0, 1, 2, 3}, // bottom
      {4, 5, 6, 7}, // top
      {0, 1, 5, 4}, // front
      {1, 2, 6, 5}, // right
      {2, 3, 7, 6}, // back
      {3, 0, 4, 7}  // left
  };
  auto quads = vtkSmartPointer<vtkCellArray>::New();
  for (int i = 0; i < 6; ++i)
  {
    vtkIdType pts[4] = {faces[i][0], faces[i][1], faces[i][2], faces[i][3]};
    quads->InsertNextCell(4, pts);
  }
  auto boxPoly = vtkSmartPointer<vtkPolyData>::New();
  boxPoly->SetPoints(boxPoints);
  boxPoly->SetPolys(quads);

  // // Save as VTU
  std::stringstream stepParticles;
  stepParticles << "data/PARTICLES_" << std::setfill('0') << std::setw(10) << this->data->cstep << ".vtp";
  std::stringstream stepBox;
  stepBox << "data/BOX_" << std::setfill('0') << std::setw(10) << this->data->cstep << ".vtp";

  vtkSmartPointer<vtkXMLPolyDataWriter> writerParticles = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writerParticles->SetFileName(stepParticles.str().c_str());
  writerParticles->SetInputData(polyData);
  writerParticles->Write();

  vtkSmartPointer<vtkXMLPolyDataWriter> writerBox = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writerBox->SetFileName(stepBox.str().c_str());
  writerBox->SetInputData(boxPoly);
  writerBox->Write();

  std::stringstream stepVTM;
  stepVTM << "data/OUTPUT_" << std::setfill('0') << std::setw(10) << this->data->cstep << ".vtm";

  std::fstream vtmFile(stepVTM.str(), std::ios::out);
  vtmFile << "<?xml version=\"1.0\"?>\n";
  vtmFile << "<VTKFile type=\"vtkMultiBlockDataSet\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt32\" compressor=\"vtkZLibDataCompressor\">\n";
  vtmFile << "  <vtkMultiBlockDataSet>\n";
  vtmFile << "    <DataSet index=\"0\" name=\"MAIN\" file=\"" << "PARTICLES_" << std::setfill('0') << std::setw(10) << this->data->cstep << ".vtp" << "\"/>\n";
  vtmFile << "    <DataSet index=\"1\" name=\"WALLS\" file=\"" << "BOX_" << std::setfill('0') << std::setw(10) << this->data->cstep << ".vtp" << "\"/>\n";
  vtmFile << "  </vtkMultiBlockDataSet>\n";
  vtmFile << "</VTKFile>\n";
  vtmFile.close();
}