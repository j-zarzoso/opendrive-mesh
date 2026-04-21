# OpenDRIVE Mesh Generator

A C++ tool that converts OpenDRIVE (.xodr) files to 3D mesh formats (USD and OBJ).

## Features

- Parses OpenDRIVE files (roads, lanes, objects, signals)
- Exports to USD (.usda) format
- Exports to OBJ (.obj) format
- Supports lane geometry with widths, elevation, and superelevation

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
├── parser/       # OpenDRIVE XML parser
├── geometry/     # Road geometry and lane mesh generation
├── mesh/         # Mesh generation (objects, signals, junctions)
├── export/       # USD and OBJ exporters
├── types/        # Shared data types
└── main.cpp      # CLI entry point

extern/pugixml/   # XML parsing library
tests/            # Unit tests
```

## License

[Add license information]
