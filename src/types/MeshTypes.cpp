#include "MeshTypes.h"

void Mesh::addVertex(float x, float y, float z, float nx, float ny, float nz) {
    vertices.push_back({x, y, z, nx, ny, nz, 0, 0});
}

void Mesh::addTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
    triangles.push_back({i0, i1, i2});
}

void Mesh::clear() {
    vertices.clear();
    triangles.clear();
}
