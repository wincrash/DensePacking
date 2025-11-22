import sys
import math
import argparse
import random

#!/usr/bin/env python3
"""
Generate two filled disks of particle centers (spheres) at z=minz and z=maxz
and write to packInput.vtk (legacy VTK POLYDATA with verts and a point-data
array 'radius' containing particle radius).

This version fills each circular cross-section (disk) using a hexagonal
(/triangular) lattice so the disk interior is populated with particle
centers rather than only the circumference.

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

def make_disk_points(z, cyl_radius, particle_radius):
    """
    Fill a disk (circle) in the XY plane at height z with particle centers.
    Uses a hexagonal (triangular) lattice for near-uniform dense packing:
      - horizontal spacing dx = 2*r
      - vertical spacing   dy = sqrt(3)*r
    Centers are trimmed so particle surfaces remain inside cyl_radius.
    """
    R = max(0.0, cyl_radius - particle_radius)  # allowed center radius
    pts = []
    if R <= 0.0:
        pts.append((0.0, 0.0, float(z)))
        return pts

    dx = 2.0 * particle_radius
    dy = math.sqrt(3.0) * particle_radius

    j = 0
    # iterate rows from -R to +R
    y = -R
    eps = 1e-12
    while y <= R + eps:
        # offset every other row for hexagonal packing
        row_offset = (j % 2) * (dx / 2.0)
        # maximum x for this y to stay inside disk of radius R
        max_x = math.sqrt(max(0.0, R * R - y * y))
        x = -max_x + row_offset
        while x <= max_x + eps:
            if x * x + y * y <= R * R + eps:
                pts.append((x, y, float(z)))
            x += dx
        j += 1
        y += dy
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
    p.add_argument("--out", type=str, default="packInput.vtk", help="output VTK filename (default packInput.vtk)")
    p.add_argument("--shuffle", action="store_true", help="randomize the order of particles in the output file")
    p.add_argument("--seed", type=int, default=None, help="optional integer seed for reproducible shuffling")
    return p.parse_args(argv)

def main(argv):
    args = parse_args(argv)
    if args.maxz < args.minz:
        raise SystemExit("Error: maxz must be >= minz")
    if args.cylradius < 0 or args.pradius <= 0:
        raise SystemExit("Error: radii must be positive (cylradius >= 0, pradius > 0)")

    pts_min = make_disk_points(args.minz+args.pradius, args.cylradius, args.pradius)
    pts_max = make_disk_points(args.maxz-args.pradius, args.cylradius, args.pradius)

    all_pts = pts_min + pts_max
    # optionally randomize output order
    if getattr(args, "shuffle", False):
        if args.seed is not None:
            random.seed(args.seed)
        random.shuffle(all_pts)

    poly = build_polydata(all_pts, args.pradius)
    write_vtk(poly, args.out)
    print("Wrote", args.out, "with", len(all_pts), "particle centers.")

if __name__ == "__main__":
    main(sys.argv[1:])
    