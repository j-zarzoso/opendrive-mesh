#pragma once
#include "types/MeshTypes.h"
#include "parser/types.h"
#include <vector>

class JunctionMesh {
public:
    JunctionMesh(const OpenDRIVEFile& file);
    std::vector<Mesh> generate();
};
