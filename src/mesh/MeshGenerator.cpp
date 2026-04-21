#include "MeshGenerator.h"
#include "ObjectMesh.h"
#include "SignalMesh.h"

MeshGenerator::MeshGenerator(const OpenDRIVEFile& file, double stepSize)
    : file_(file), stepSize_(stepSize) {}

std::vector<Mesh> MeshGenerator::generateAll() {
    std::vector<Mesh> result;
    
    auto roads = generateRoads();
    result.insert(result.end(), roads.begin(), roads.end());
    
    auto objects = generateObjects();
    result.insert(result.end(), objects.begin(), objects.end());
    
    auto signals = generateSignals();
    result.insert(result.end(), signals.begin(), signals.end());
    
    return result;
}

std::vector<Mesh> MeshGenerator::generateRoads() {
    std::vector<Mesh> result;
    
    for (const auto& road : file_.roads) {
        RoadGeometry geometry(road);
        LaneMesh laneMesh(road, geometry, stepSize_);
        auto mesh = laneMesh.generate();
        result.push_back(mesh);
    }
    
    return result;
}

std::vector<Mesh> MeshGenerator::generateObjects() {
    std::vector<Mesh> result;
    
    for (const auto& road : file_.roads) {
        RoadGeometry geometry(road);
        ObjectMesh objMesh(road, geometry);
        auto meshes = objMesh.generate();
        result.insert(result.end(), meshes.begin(), meshes.end());
    }
    
    return result;
}

std::vector<Mesh> MeshGenerator::generateSignals() {
    std::vector<Mesh> result;
    
    for (const auto& road : file_.roads) {
        RoadGeometry geometry(road);
        SignalMesh sigMesh(road, geometry);
        auto meshes = sigMesh.generate();
        result.insert(result.end(), meshes.begin(), meshes.end());
    }
    
    return result;
}

void MeshGenerator::exportToUsd(const std::string& outputPath) {
    auto meshes = generateAll();
    
    USDExporter exporter(outputPath);
    exporter.exportMeshes(meshes);
    exporter.finalize();
}

void MeshGenerator::exportToObj(const std::string& outputPath) {
    auto meshes = generateAll();
    
    OBJExporter exporter(outputPath);
    exporter.exportMeshes(meshes);
    exporter.finalize();
}

std::vector<Mesh> MeshGenerator::generateMeshesForRoad(const Road& road) {
    std::vector<Mesh> result;
    RoadGeometry geometry(road);
    LaneMesh laneMesh(road, geometry, stepSize_);
    result.push_back(laneMesh.generate());
    return result;
}
