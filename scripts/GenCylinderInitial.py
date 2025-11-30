import sys
import math
import argparse
import random

#!/usr/bin/env python3
"""
Generate two filled disks of particle centers (spheres) at z=minz and z=maxz
and write to packInput.vtk (legacy VTK POLYDATA with verts and a point-data
array 'radius' containing particle radius).

This version fills each circular cross-section (disk) using a simple
rectangular (Cartesian) grid so the disk interior is populated with
particle centers arranged regularly.

Usage:
    python GenCylinderInitial.py --minz 0 --maxz 10 --cylradius 5 --pradius 0.2
"""

from vtk import (
    vtkPoints,
    vtkPolyData,
    vtkCellArray,
    vtkFloatArray,
    vtkPolyDataWriter,
)

def make_disk_points(z, cyl_radius, particle_radius,dir):
    pts=[]
    ilgis=2*cyl_radius
    kiekis=int(ilgis/(2*particle_radius))
    print(kiekis)
    newR=ilgis/kiekis
    newR=particle_radius

    for i in range(kiekis):
        for j in range(kiekis):
            x=-cyl_radius+i*2*newR+newR
            y=-cyl_radius+j*2*newR+newR
            a=math.sqrt(x*x+y*y)
            if a<cyl_radius:
                pts.append([x,y,z+newR*dir])
            


    
    return pts

def build_polydata(all_points, particle_radius):
    points = vtkPoints()
    verts = vtkCellArray()
    rad_array = vtkFloatArray()
    rad_array.SetName("RADIUS")
    for i, (x, y, z) in enumerate(all_points):
        pid = points.InsertNextPoint(x, y, z)
        # create a vertex cell referencing this point
        verts.InsertNextCell(1)
        verts.InsertCellPoint(pid)
        rad_array.InsertNextValue(float(particle_radius))

    poly = vtkPolyData()
    poly.SetPoints(points)
    poly.SetVerts(verts)
    poly.GetPointData().AddArray(rad_array)
    # also set active scalars to radius (optional)
    poly.GetPointData().SetScalars(rad_array)
    return poly

def write_vtk(polydata, filename="packInput.vtk"):
    writer = vtkPolyDataWriter()
    writer.SetFileName(filename)
    writer.SetInputData(polydata)
    writer.SetFileTypeToASCII()
    if not writer.Write():
        raise RuntimeError("Failed to write VTK file: " + filename)

def parse_args(argv):
    p = argparse.ArgumentParser(description="Generate two filled disks of particle centers and write packInput.vtk")
    p.add_argument("--minz", type=float, required=True, help="minimum z value")
    p.add_argument("--maxz", type=float, required=True, help="maximum z value")
    p.add_argument("--cylradius", type=float, required=True, help="cylinder radius")
    p.add_argument("--pradius", type=float, required=True, help="particle radius (sphere radius)")
    return p.parse_args(argv)

def main(argv):
    args = parse_args(argv)
    if args.maxz < args.minz:
        raise SystemExit("Error: maxz must be >= minz")
    if args.cylradius < 0 or args.pradius <= 0:
        raise SystemExit("Error: radii must be positive (cylradius >= 0, pradius > 0)")

    # Place disks exactly at minz and maxz (no offset by particle radius)
    pts_min = make_disk_points(args.minz, args.cylradius, args.pradius,1)
    pts_max = make_disk_points(args.maxz, args.cylradius, args.pradius,-1)

    all_pts = pts_min + pts_max
    # optionally randomize output order
    if getattr(args, "shuffle", False):
        if args.seed is not None:
            random.seed(args.seed)
        random.shuffle(all_pts)

    poly = build_polydata(all_pts, args.pradius)
    write_vtk(poly)
    print("Wrote", "with", len(all_pts), "particle centers.")

if __name__ == "__main__":
    main(sys.argv[1:])
    