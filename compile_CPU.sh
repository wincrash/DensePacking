#!/bin/bash
#SBATCH --time=999:99:99
#SBATCH --constraint=i7-12700
#SBATCH --job=COMPILE_DensePacking
#SBATCH --nodes=1
#SBATCH --output=output_cpu.log

hostname;date;pwd
module load devel/kokkos-4.7.01-CPU devel/vtk-9.5.0 devel/yaml-cpp
MY_DIR="/export/home/ruslan/SOFTWARE/DensePacking/build-cpu"
rm -rf $MY_DIR
mkdir -p $MY_DIR
cd $MY_DIR
cmake ..
make -j20
hostname;date;pwd
