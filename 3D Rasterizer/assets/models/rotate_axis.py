"""
Rotates all vertices in an .obj file by a chosen angle about a chosen axis.
Use this to correct a mesh's starting orientation without re-exporting.

Supported angles: -90, 90, 180 degrees.
All transforms are exact (integer trig) — no floating-point drift.

Rotation formulas (right-hand rule, CCW when looking from positive axis):
  X,  90°: (x,  y,  z) -> (x, -z,  y)
  X, -90°: (x,  y,  z) -> (x,  z, -y)
  X, 180°: (x,  y,  z) -> (x, -y, -z)

  Y,  90°: (x,  y,  z) -> ( z,  y, -x)
  Y, -90°: (x,  y,  z) -> (-z,  y,  x)
  Y, 180°: (x,  y,  z) -> (-x,  y, -z)

  Z,  90°: (x,  y,  z) -> (-y,  x,  z)
  Z, -90°: (x,  y,  z) -> ( y, -x,  z)
  Z, 180°: (x,  y,  z) -> (-x, -y,  z)
"""
import os, sys

DIR = os.path.dirname(os.path.abspath(__file__))

VALID_DEGREES = (-90, 90, 180)
VALID_AXES    = ('x', 'y', 'z')

# Each entry is a lambda (x, y, z) -> (x', y', z')
TRANSFORMS = {
    ('x',  90):  lambda x, y, z: ( x, -z,  y),
    ('x', -90):  lambda x, y, z: ( x,  z, -y),
    ('x', 180):  lambda x, y, z: ( x, -y, -z),
    ('y',  90):  lambda x, y, z: ( z,  y, -x),
    ('y', -90):  lambda x, y, z: (-z,  y,  x),
    ('y', 180):  lambda x, y, z: (-x,  y, -z),
    ('z',  90):  lambda x, y, z: (-y,  x,  z),
    ('z', -90):  lambda x, y, z: ( y, -x,  z),
    ('z', 180):  lambda x, y, z: (-x, -y,  z),
}


def rotate(path, degrees, axis):
    transform = TRANSFORMS[(axis, degrees)]

    with open(path, 'r') as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if not line.startswith('v '):
            continue
        parts = line.split()
        x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
        x, y, z = transform(x, y, z)
        rest   = parts[4:]
        suffix = (' ' + ' '.join(rest)) if rest else ''
        lines[i] = f'v {x:.6f} {y:.6f} {z:.6f}{suffix}\n'

    with open(path, 'w') as f:
        f.writelines(lines)

    print(f"Rotated '{os.path.basename(path)}' {degrees:+d}° about the {axis.upper()}-axis.")


def prompt_degrees():
    options = ', '.join(str(d) for d in VALID_DEGREES)
    while True:
        raw = input(f"Degrees to rotate ({options}): ").strip()
        try:
            val = int(raw)
            if val in VALID_DEGREES:
                return val
        except ValueError:
            pass
        print(f"  Please enter one of: {options}.")


def prompt_axis():
    while True:
        raw = input("Axis to rotate about (X / Y / Z): ").strip().lower()
        if raw in VALID_AXES:
            return raw
        print("  Please enter X, Y, or Z.")


def prompt_file():
    while True:
        name = input("OBJ filename (name, name.obj, or full path): ").strip()
        for candidate in (name, os.path.join(DIR, name)):
            if os.path.isfile(candidate):
                return os.path.abspath(candidate)
        if not name.lower().endswith('.obj'):
            with_ext = name + '.obj'
            for candidate in (with_ext, os.path.join(DIR, with_ext)):
                if os.path.isfile(candidate):
                    return os.path.abspath(candidate)
        print(f"  File not found: '{name}'. Try again.")


def main():
    # Non-interactive: python rotate_axis.py <degrees> <axis> <file>
    if len(sys.argv) == 4:
        try:
            degrees = int(sys.argv[1])
        except ValueError:
            print(f"Error: degrees must be an integer (got '{sys.argv[1]}').")
            sys.exit(1)
        if degrees not in VALID_DEGREES:
            print(f"Error: degrees must be one of {VALID_DEGREES} (got {degrees}).")
            sys.exit(1)
        axis = sys.argv[2].strip().lower()
        if axis not in VALID_AXES:
            print(f"Error: axis must be X, Y, or Z (got '{sys.argv[2]}').")
            sys.exit(1)
        path = sys.argv[3].strip()
        for candidate in (path, os.path.join(DIR, path)):
            if os.path.isfile(candidate):
                rotate(os.path.abspath(candidate), degrees, axis)
                return
        print(f"Error: file not found: '{path}'.")
        sys.exit(1)

    print("=== OBJ axis rotation ===")
    degrees = prompt_degrees()
    axis    = prompt_axis()
    path    = prompt_file()
    rotate(path, degrees, axis)


if __name__ == '__main__':
    main()
