#!/bin/bash
rsync -zarv --exclude="*.vtk" --exclude="build" --exclude="nial" --exclude="*.vtp" ./ wolf:/export/home/ruslan/SOFTWARE/DensePacking
