"""
Normalizes every .obj in the same directory (or paths given on the command line):
  - Centers each mesh at the origin (bounding-box centroid)
  - Scales uniformly so the largest bounding-box dimension fits inside the viewport

TARGET_SIZE is derived from the actual camera / projection settings used in main.cpp:
  Camera position : (0, 0, 3)  -> distance from origin = 3
  FOV Y           : 45 degrees
  Fill factor     : 0.75  (mesh occupies 75 % of the shorter viewport dimension)

  half_h    = tan(fovY / 2) * camera_dist  =  tan(22.5°) * 3  ≈ 1.243
  viewport_h = 2 * half_h                                       ≈ 2.485
  TARGET_SIZE = viewport_h * FILL_FACTOR                        ≈ 1.864
"""
import os, math, sys

# ── camera / projection constants (must match main.cpp) ──────────────────────
CAMERA_DIST  = 3.0   # camera Z position (camera looks at origin)
FOV_Y_DEG    = 45.0  # camera.zoom default (degrees)
FILL_FACTOR  = 0.75  # fraction of viewport height the mesh should occupy

half_h      = math.tan(math.radians(FOV_Y_DEG / 2.0)) * CAMERA_DIST
TARGET_SIZE = 2.0 * half_h * FILL_FACTOR
# ─────────────────────────────────────────────────────────────────────────────

DIR = os.path.dirname(os.path.abspath(__file__))

def process(path):
    with open(path, 'r') as f:
        lines = f.readlines()

    verts = []  # list of [x, y, z] for 'v' lines; (line_index, slot)
    vert_line_indices = []
    for i, line in enumerate(lines):
        if line.startswith('v '):
            parts = line.split()
            verts.append([float(parts[1]), float(parts[2]), float(parts[3])])
            vert_line_indices.append(i)

    if not verts:
        return

    xs = [v[0] for v in verts]
    ys = [v[1] for v in verts]
    zs = [v[2] for v in verts]
    cx = (min(xs) + max(xs)) / 2
    cy = (min(ys) + max(ys)) / 2
    cz = (min(zs) + max(zs)) / 2
    max_dim = max(max(xs)-min(xs), max(ys)-min(ys), max(zs)-min(zs))
    scale = TARGET_SIZE / max_dim if max_dim > 0 else 1.0

    for v in verts:
        v[0] = (v[0] - cx) * scale
        v[1] = (v[1] - cy) * scale
        v[2] = (v[2] - cz) * scale

    for idx, li in enumerate(vert_line_indices):
        x, y, z = verts[idx]
        # preserve any trailing comment
        rest = lines[li].split()[4:]
        suffix = (' ' + ' '.join(rest)) if rest else ''
        lines[li] = f'v {x:.6f} {y:.6f} {z:.6f}{suffix}\n'

    with open(path, 'w') as f:
        f.writelines(lines)

    print(f'{os.path.basename(path)}: maxDim={max_dim:.4f} -> {TARGET_SIZE:.4f}  centroid=({cx:.3f},{cy:.3f},{cz:.3f})')

# Accept explicit file paths from the command line, otherwise process all .obj
# files in the same directory as this script.
#   Usage: python normalize_objs.py [file1.obj file2.obj ...]
targets = sys.argv[1:] if len(sys.argv) > 1 else [
    os.path.join(DIR, f) for f in os.listdir(DIR) if f.endswith('.obj')
]

print(f'TARGET_SIZE = {TARGET_SIZE:.4f}  '
      f'(camera_dist={CAMERA_DIST}, fovY={FOV_Y_DEG}°, fill={FILL_FACTOR})\n')

for path in targets:
    process(path)

print("Done.")
