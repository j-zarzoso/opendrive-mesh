#pragma once
#include "parser/types.h"
#include "geometry/RoadGeometry.h"
#include "geometry/LaneMesh.h"
#include "types/MeshTypes.h"
#include "export/USDExporter.h"
#include "export/OBJExporter.h"
#include <string>
#include <vector>

class MeshGenerator {
public:
    MeshGenerator(const OpenDRIVEFile& file, double stepSize = 0.5);
    
    // Generate all meshes from the OpenDRIVE file
    std::vector<Mesh> generateAll();
    
    // Generate road meshes only
    std::vector<Mesh> generateRoads();
    
    // Generate object meshes
    std::vector<Mesh> generateObjects();
    
    // Generate signal meshes
    std::vector<Mesh> generateSignals();
    
    // Generate and export to USD
    void exportToUsd(const std::string& outputPath);
    
    // Generate and export to OBJ
    void exportToObj(const std::string& outputPath);
    
private:
    const OpenDRIVEFile& file_;
    double stepSize_;
    
    std::vector<Mesh> generateMeshesForRoad(const Road& road);
};
