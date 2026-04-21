#pragma once
#include <vector>
#include <string>
#include <cstdint>

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct Triangle {
    uint32_t i0, i1, i2;
};

struct Mesh {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    
    void addVertex(float x, float y, float z, float nx, float ny, float nz);
    void addTriangle(uint32_t i0, uint32_t i1, uint32_t i2);
    void clear();
};

