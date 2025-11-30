#!/bin/bash
#SBATCH --time=999:99:99
#SBATCH --nodelist=gpu-0-0.cluster
#####SBATCH --gres=gpu:tesla:0
#SBATCH --job=COMPILE_DensePacking
#SBATCH --nodes=1
#SBATCH --output=output_gpu.log

hostname;date;pwd
module load  devel/kokkos-4.7.01-GPU devel/vtk-9.5.0  devel/yaml-cpp
MY_DIR="/export/home/ruslan/SOFTWARE/DensePacking/build-gpu"
rm -rf $MY_DIR
mkdir -p $MY_DIR
cd $MY_DIR
cmake ..
make -j20
hostname;date;pwd
