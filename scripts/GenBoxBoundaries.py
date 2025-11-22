#!/usr/bin/env python3
import sys
import math
import argparse
import random

"""
Generate two faces of particle centers (spheres) on the z=min and z=max
faces of an axis-aligned box defined by min and max 3D points. Spheres are
placed on a simple uniform rectangular grid over the face area.

Usage example:
    python GenBoxBoundaries.py --min 0 0 0 --max 1 1 1 --pradius 0.01

The script writes a legacy VTK `packInput.vtk` (modifiable via `--out`).
"""

from vtk import (
    vtkPoints,
    vtkPolyData,
    vtkCellArray,
    vtkFloatArray,
    vtkPolyDataWriter,
)


def make_grid_points_on_face(minx, miny, zcoord, maxx, maxy, particle_radius, spacing=None):
    """Generate points on a rectangle [minx,maxx] x [miny,maxy] at z=zcoord.

    - particle centers are constrained to be at least `particle_radius` from
      the box edges (so center range is [min+R, max-R]).
    - `spacing` controls grid spacing; if None it defaults to 2*particle_radius.
    - returns a list of (x,y,z) tuples. Ensures at least one point is returned
      (centered) if the available region is smaller than spacing.
    """
    R = float(particle_radius)
    if spacing is None:
        spacing = 2.0 * R
    else:
        spacing = float(spacing)
    if spacing <= 0.0:
        raise ValueError("spacing must be > 0")

    x_min = minx + R
    x_max = maxx - R
    y_min = miny + R
    y_max = maxy - R

    pts = []
    # If there is no space for centers, return a single center at the face center
    if x_max < x_min or y_max < y_min:
        cx = 0.5 * (minx + maxx)
        cy = 0.5 * (miny + maxy)
        pts.append((cx, cy, float(zcoord)))
        return pts

    # start at x_min, y_min and step by spacing, but also ensure coverage to x_max/y_max
    # compute number of steps that fit and the actual spacing to center grid neatly
    nx = max(1, int(math.floor((x_max - x_min) / spacing)) + 1)
    ny = max(1, int(math.floor((y_max - y_min) / spacing)) + 1)

    # recompute spacing so that last point <= x_max (we keep original spacing but
    # distribute remainder by keeping a uniform step <= requested spacing)
    if nx > 1:
        actual_dx = (x_max - x_min) / (nx - 1)
    else:
        actual_dx = 1.0  # arbitrary, single column
    if ny > 1:
        actual_dy = (y_max - y_min) / (ny - 1)
    else:
        actual_dy = 1.0

    for i in range(nx):
        x = x_min + i * actual_dx
        for j in range(ny):
            y = y_min + j * actual_dy
            pts.append((float(x), float(y), float(zcoord)))
    return pts


def build_polydata(all_points, particle_radius):
    points = vtkPoints()
    verts = vtkCellArray()
    rad_array = vtkFloatArray()
    rad_array.SetName("RADIUS")
    for i, (x, y, z) in enumerate(all_points):
        pid = points.InsertNextPoint(x, y, z)
        verts.InsertNextCell(1)
        verts.InsertCellPoint(pid)
        rad_array.InsertNextValue(float(particle_radius))

    poly = vtkPolyData()
    poly.SetPoints(points)
    poly.SetVerts(verts)
    poly.GetPointData().AddArray(rad_array)
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
    p = argparse.ArgumentParser(description="Generate grid of particle centers on box z-faces and write packInput.vtk")
    p.add_argument("--min", type=float, nargs=3, required=True, metavar=("MINX", "MINY", "MINZ"), help="minimum corner of the box (x y z)")
    p.add_argument("--max", type=float, nargs=3, required=True, metavar=("MAXX", "MAXY", "MAXZ"), help="maximum corner of the box (x y z)")
    p.add_argument("--pradius", type=float, required=True, help="particle radius (sphere radius)")
    p.add_argument("--spacing", type=float, default=None, help="optional grid spacing; defaults to 2*pradius")
    p.add_argument("--out", type=str, default="packInput.vtk", help="output VTK filename (default packInput.vtk)")
    p.add_argument("--shuffle", action="store_true", help="randomize the order of particles in the output file")
    p.add_argument("--seed", type=int, default=None, help="optional integer seed for reproducible shuffling")
    return p.parse_args(argv)


def main(argv):
    args = parse_args(argv)
    minx, miny, minz = args.min
    maxx, maxy, maxz = args.max
    R = args.pradius

    if maxx < minx or maxy < miny or maxz < minz:
        raise SystemExit("Error: each component of --max must be >= corresponding --min")
    if R <= 0.0:
        raise SystemExit("Error: pradius must be > 0")

    # z positions for center of spheres on faces
    z_low = minz + R
    z_high = maxz - R

    pts_low = make_grid_points_on_face(minx, miny, z_low, maxx, maxy, R, spacing=args.spacing)
    pts_high = make_grid_points_on_face(minx, miny, z_high, maxx, maxy, R, spacing=args.spacing)

    all_pts = pts_low + pts_high
    if getattr(args, "shuffle", False):
        if args.seed is not None:
            random.seed(args.seed)
        random.shuffle(all_pts)

    poly = build_polydata(all_pts, R)
    write_vtk(poly, args.out)
    print("Wrote", args.out, "with", len(all_pts), "particle centers.")


if __name__ == "__main__":
    main(sys.argv[1:])
