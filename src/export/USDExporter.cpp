#include "USDExporter.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

USDExporter::USDExporter(const std::string& outputPath) : outputPath_(outputPath) {}

void USDExporter::exportMesh(const Mesh& mesh) {
    meshes_.push_back(mesh);
}

void USDExporter::exportMeshes(const std::vector<Mesh>& meshes) {
    for (const auto& mesh : meshes) {
        meshes_.push_back(mesh);
    }
}

  void USDExporter::finalize() {
    std::ofstream out(outputPath_, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputPath_);
    }
    
    // Write USD header
    out << generateUsdHeader();
    
    // Write each mesh
  for (size_t i = 0; i < meshes_.size(); i++) {
        out << generateMeshPrim(meshes_[i], static_cast<int>(i));
    }
    
    out << "}" << std::endl;
    out.close();
}

std::string USDExporter::generateUsdHeader() const {
    std::stringstream ss;
    ss << "#usda 1.0" << std::endl;
    ss << "def Xform \"" << "World" << "\" {" << std::endl;
    return ss.str();
}

std::string USDExporter::generateMeshPrim(const Mesh& mesh, int index) const {
    std::stringstream ss;
    
    // Escape mesh name for USD
    std::string primName = mesh.name;
    for (char& c : primName) {
        if (c == ' ' || c == '/' || c == '.') c = '_';
    }
    
  ss << "    def mesh \"" << primName << "\" {" << std::endl;
    
    // Point positions - not uniform per USD mesh spec
    ss << "        point3f[] points = [" << std::endl;
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        const auto& v = mesh.vertices[i];
        ss << "            (" << std::fixed << std::setprecision(6) 
           << v.x << ", " << v.y << ", " << v.z << ")";
        if (i < mesh.vertices.size() - 1) ss << ",";
        ss << std::endl;
    }
    ss << "        ]" << std::endl;
    
    // Face vertex counts - uniform per USD mesh spec
    ss << "        uniform int[] faceVertexCounts = [";
    for (size_t i = 0; i < mesh.triangles.size(); i++) {
        ss << "3";
        if (i < mesh.triangles.size() - 1) ss << ", ";
    }
    ss << "]" << std::endl;
    
    // Face vertex indices - uniform per USD mesh spec
    ss << "        uniform int[] faceVertexIndices = [";
    for (size_t i = 0; i < mesh.triangles.size(); i++) {
        const auto& t = mesh.triangles[i];
        ss << t.i0 << ", " << t.i1 << ", " << t.i2;
        if (i < mesh.triangles.size() - 1) ss << ", ";
    }
    ss << "]" << std::endl;
    
    // Normals - not uniform per USD mesh spec
    ss << "        normal3f[] normals = [" << std::endl;
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        const auto& v = mesh.vertices[i];
        float len = std::sqrt(v.nx*v.nx + v.ny*v.ny + v.nz*v.nz);
        if (len > 1e-6) {
            ss << "            (" << v.nx/len << ", " << v.ny/len << ", " << v.nz/len << ")";
        } else {
            ss << "            (0, 1, 0)";
        }
        if (i < mesh.vertices.size() - 1) ss << ",";
        ss << std::endl;
    }
    ss << "        ]" << std::endl;
    
    // UV coordinates - uniform per USD mesh spec
    ss << "        uniform float2[] primvars:st = [" << std::endl;
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        const auto& v = mesh.vertices[i];
        ss << "            (" << v.u << ", " << v.v << ")";
        if (i < mesh.vertices.size() - 1) ss << ",";
        ss << std::endl;
    }
    ss << "        ]" << std::endl;
    
    // Prim interpolation
    ss << "        uniform token primvars:st:interpolation = \"interpolate\"" << std::endl;
    
    ss << "    }" << std::endl;
    
    return ss.str();
}
