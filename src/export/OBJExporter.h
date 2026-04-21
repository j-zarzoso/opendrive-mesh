#pragma once
#include "types/MeshTypes.h"
#include <string>
#include <vector>

class OBJExporter {
public:
    OBJExporter(const std::string& outputPath);
    
    void exportMesh(const Mesh& mesh);
    void exportMeshes(const std::vector<Mesh>& meshes);
    void finalize();

private:
    std::string outputPath_;
    std::vector<Mesh> meshes_;
    
    void writeHeader();
    void writeMesh(const Mesh& mesh, int index);
};
