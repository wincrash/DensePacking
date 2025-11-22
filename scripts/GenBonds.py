import numpy as np
from scipy.spatial import cKDTree
import vtk

def find_intersecting_spheres_kdtree(spheres_data,max_overlap):
    """
    Finds all unique intersecting pairs of spheres using a k-D tree for acceleration.

    :param spheres_data: A list of tuples/lists: [((x, y, z), radius), ...]
    :return: A list of unique intersecting sphere index pairs [(i, j), ...]
    """
    
    # --- Data Preparation ---
    # Separate centers (for k-D tree) and radii
    centers = np.array([s[0] for s in spheres_data])
    radii = np.array([s[1] for s in spheres_data])
    n_spheres = len(centers)
    
    # The maximum radius in the dataset is used to set the search bound
    max_radius = np.max(radii)*2.5
    
    # --- 1. Build the k-D Tree ---
    # The k-D tree is built on the centers (3D coordinates)
    tree = cKDTree(centers)
    
    intersecting_pairs = set()
    
    # --- 2. Query the k-D Tree and Refine ---
    for i in range(n_spheres):
        r_i = radii[i]
        
        # Calculate the search radius for the k-D tree.
        # This is the radius of Sphere i + the maximum possible radius of any other sphere.
        # This ensures we capture all potential intersections.
        query_radius = r_i + max_radius
        
        # Query the tree: find indices of all points (centers) within query_radius of center[i]
        # 'results' contains a list of neighbor indices (j)
        results = tree.query_ball_point(centers[i], query_radius)
        
        # --- 3. Refine (Precise Check) ---
        for j in results:
            # We only check unique pairs (i < j) and skip self-check (i == j is guaranteed to be in results)
            if i < j:
                r_j = radii[j]
                
                # Calculate the exact distance between centers C_i and C_j
                # Since we are using NumPy arrays, we can use optimized L2 norm
                d = np.linalg.norm(centers[i] - centers[j])

                # Check for intersection
                if d<= (r_i + r_j+max_overlap):
                    intersecting_pairs.add((i, j))
    
    # Convert the set of unique pairs to a list
    return list(intersecting_pairs)


reader=vtk.vtkXMLPolyDataReader()
reader.SetFileName("result.vtp")
reader.Update()
output=reader.GetOutput()
print(output)
numSpheres=output.GetNumberOfPoints()
spheres_data=[]
max_overlap=output.GetPointData().GetArray("MAX_OVERLAP").GetRange()[1]
print("Max overlap ",max_overlap)

material=vtk.vtkIntArray()
material.SetName("MATERIAL")
material.SetNumberOfComponents(1)
material.SetNumberOfTuples(numSpheres)

for i in range(numSpheres):
    cell=output.GetCell(i)
    center=output.GetPoint(i)
    radius=output.GetPointData().GetArray("RADIUS").GetTuple1(i)
    output.GetPointData().GetArray("FIX").SetTuple1(i,0)
    spheres_data.append((center, radius))
    material.SetTuple1(i,0)

output.GetPointData().AddArray(material)
pairs = find_intersecting_spheres_kdtree(spheres_data,max_overlap)

print("List of sphere indices that intersect (using cKDTree):")
print("Total pairs ",len(pairs))
lines=vtk.vtkCellArray()
for pair in sorted(pairs):
    line=vtk.vtkLine()
    line.GetPointIds().SetId(0,pair[0])
    line.GetPointIds().SetId(1,pair[1])
    lines.InsertNextCell(line)
output.SetLines(lines)
output.SetVerts(vtk.vtkCellArray())
writer=vtk.vtkXMLPolyDataWriter()
writer.SetFileName("input.vtp")
writer.SetInputData(output)
writer.Write()
# Expected Output: [(0, 1), (0, 3), (1, 2)] (Order may vary due to use of set)