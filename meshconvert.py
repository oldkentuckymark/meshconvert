import bpy
import csv
import os

# Set output path (adjust as needed)
output_path = "/home/mark/m.csv"

# Get the active object
obj = bpy.context.active_object

if obj is None or obj.type != 'MESH':
    raise Exception("No mesh object selected!")

# Ensure mesh data is up to date
mesh = obj.data
mesh.calc_loop_triangles()

# Try to get vertex colors
color_layer = None
if mesh.vertex_colors:
    color_layer = mesh.vertex_colors.active

# Open CSV file for writing
with open(output_path, mode='w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    # Header row
    #writer.writerow(["x", "y", "z", "r", "g", "b", "a"])

    # Iterate over triangles and their loops
    for tri in mesh.loop_triangles:
        for loop_index in tri.loops:
            vert_index = mesh.loops[loop_index].vertex_index
            vertex = mesh.vertices[vert_index]
            co = obj.matrix_world @ vertex.co  # world coordinates

            # Default color
            r = g = b = a = 1.0
            if color_layer:
                color = color_layer.data[loop_index].color
                r, g, b, a = color

            writer.writerow([co.x, co.z, co.y, r, g, b, a])

print(f"Exported {obj.name} vertices to {output_path}")
