#pragma once
#include "types/MeshTypes.h"
#include "parser/types.h"
#include "geometry/Math.h"
#include <vector>
#include <string>

class JunctionMesh {
public:
    JunctionMesh(const OpenDRIVEFile& file, double stepSize = 0.5);
    std::vector<Mesh> generate();
    
private:
    const OpenDRIVEFile& file_;
    double stepSize_;
    
    void generateJunctionSurfaceMesh(const Junction& junction, std::vector<Mesh>& meshes);
    void generateJunctionLaneMesh(const Junction& junction, const Road& road, std::vector<Mesh>& meshes);
    void generateConnectionMesh(const Junction& junction, const OutgoingConnection& conn, std::vector<Mesh>& meshes);
    Vec3 getRoadCenterPosition(int roadId, double s);
};
