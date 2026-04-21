# OpenDRIVE Mesh Generator

A C++ tool that converts OpenDRIVE (.xodr) files to 3D mesh formats (USD and OBJ).

## Features

- Parses OpenDRIVE files (roads, lanes, objects, signals, junctions)
- Exports to USD (.usda) format
- Exports to OBJ (.obj) format
- Supports lane geometry with widths, elevation, and superelevation
- Handles junction connections and lane meshing
- Supports 5 geometry types: LINE, ARC, SPIRAL, POLY3, PARAMPOLY3
- Supports 29+ lane types (driving, shoulder, border, stopping, restricted, parking, median, biking, sidewalk, curb, walking, tram, rail, bidirectional, shared, onRamp, offRamp, connectingRamp, entry, exit, bus, taxi, hov, slipLane, roadworks, special1/2/3)
- Supports 22+ object types (barrier, pole, tree, vegetation, building, obstacle, gantry, crosswalk, roadMark, roadSurface, parkingSpace, trafficIsland, and vehicle-dependent objects)
- Supports 60+ signal types (speedLimit, maxSpeed, minSpeed, noPassing, stop, yield, trafficLight, and many more)

## Building

### Prerequisites

- CMake 3.14 or later
- C++ compiler with C++17 support (MSVC, GCC, or Clang)

### Build Steps

```bash
mkdir -p build && cd build
cmake ..
cmake --build . --config Release
```

### Running Tests

```bash
ctest -C Release
```

## Usage

```bash
opendrive-mesh <input.xodr> <output.usd> [step_size]
opendrive-mesh <input.xodr> <output.obj> [step_size]
```

### Arguments

| Argument | Description |
|----------|-------------|
| `input.xodr` | Input OpenDRIVE file |
| `output.usd` or `output.obj` | Output file (format auto-detected from extension) |
| `step_size` | Optional mesh step size in meters (default: 0.5) |

### Examples

```bash
# Export to USD
opendrive-mesh road.xodr road.usd 1.0

# Export to OBJ
opendrive-mesh road.xodr road.obj 0.5
```

## Project Structure

```
src/
├── parser/       # OpenDRIVE XML parser (roads, lanes, objects, signals, junctions)
├── geometry/     # Road geometry and lane mesh generation
├── mesh/         # Mesh generation (objects, signals, junctions)
├── export/       # USD and OBJ exporters
├── types/        # Shared data types (Mesh, Vertex, Triangle, MeshType)
└── main.cpp      # CLI entry point

extern/pugixml/   # XML parsing library
tests/            # Unit tests (59 tests)
```

## Coordinate System

The tool converts OpenDRIVE coordinates to USD coordinates:
- OpenDRIVE X → USD X (forward)
- OpenDRIVE Y → USD Z (left)
- OpenDRIVE Z → USD -Y (up becomes down)

## Test Coverage

59 unit tests covering:
- Vec3 vector math (constructor, arithmetic, length, normalization, dot/cross products)
- FrenetFrame computation and coordinate transforms
- Plan view geometry (LINE, ARC, POLY3, PARAMPOLY3, SPIRAL)
- Elevation and superelevation profiles
- Junction parsing and mesh generation
- Signal, object, and lane type parsing

## License

[Add license information]
