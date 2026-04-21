#pragma once
#include "types/MeshTypes.h"
#include "parser/types.h"
#include "geometry/RoadGeometry.h"
#include <vector>

class ObjectMesh {
public:
    ObjectMesh(const Road& road, const RoadGeometry& geometry);
    std::vector<Mesh> generate();

private:
    const Road& road_;
    const RoadGeometry& geometry_;
};
