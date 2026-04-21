#pragma once
#include "types/MeshTypes.h"
#include <string>
#include <vector>

class USDExporter {
public:
    USDExporter(const std::string& outputPath);
    
    // Export a single mesh
    void exportMesh(const Mesh& mesh);
    
    // Export multiple meshes
    void exportMeshes(const std::vector<Mesh>& meshes);
    
    // Finalize and write file
    void finalize();
    
private:
    std::string outputPath_;
    std::vector<Mesh> meshes_;
    
    std::string generateUsdHeader() const;
    std::string generateMeshPrim(const Mesh& mesh, int index) const;
};
