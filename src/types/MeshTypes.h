#pragma once
#include <vector>
#include <string>
#include <cstdint>

enum MeshType {
    MESH_TYPE_PLANAR,
    MESH_TYPE_BOX,
    MESH_TYPE_CYLINDER,
    MESH_TYPE_CONE,
    MESH_TYPE_CURVE,
    MESH_TYPE_NONE
};

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct Triangle {
    uint32_t i0, i1, i2;
};

struct Mesh {
    std::string name = "";
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    MeshType type = MESH_TYPE_NONE;
    int meshIndex = 0;
    
    void addVertex(float x, float y, float z, float nx, float ny, float nz);
    void addTriangle(uint32_t i0, uint32_t i1, uint32_t i2);
    void clear();
};

