#!/bin/bash
rsync -zarv --exclude="*.vtk" --exclude="build" --exclude="run" --exclude="*.vtp" ./ wolf:/export/home/ruslan/SOFTWARE/DensePacking
