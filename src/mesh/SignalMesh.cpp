#include "SignalMesh.h"
#include "Math.h"
#include "types/MeshTypes.h"
#include <cmath>

SignalMesh::SignalMesh(const Road& road, const RoadGeometry& geometry)
    : road_(road), geometry_(geometry) {}

std::vector<Mesh> SignalMesh::generate() {
    std::vector<Mesh> allMeshes;
    
    for (auto& sig : road_.signals) {
        Mesh pole;
        pole.name = "signal_pole_" + sig.id;
        pole.type = MESH_TYPE_CYLINDER;
        
        auto sample = geometry_.sample(sig.s);
        Vec3 base = sample.position + sample.frame.normal * sig.hOffset + sample.frame.binormal * sig.z;
        
        float poleRadius = 0.05f;
        int segments = 8;
        float poleHeight = static_cast<float>(sig.height > 0 ? sig.height : 2.0f);
        
        std::vector<float> poleVerts;
        poleVerts.reserve(segments * 2);
        
        for (int i = 0; i < segments; i++) {
            float angle = (float)(2.0f * (float)atan(1.0) * 4.0f * i / segments);
            float x = poleRadius * cosf(angle);
            float z = poleRadius * sinf(angle);
            poleVerts.push_back(x);
            poleVerts.push_back(0);
            poleVerts.push_back(z);
        }
        for (int i = 0; i < segments; i++) {
            float angle = (float)(2.0f * (float)atan(1.0) * 4.0f * i / segments);
            float x = poleRadius * cosf(angle);
            float z = poleRadius * sinf(angle);
            poleVerts.push_back(x);
            poleVerts.push_back(poleHeight);
            poleVerts.push_back(z);
        }
        
        for (int i = 0; i < segments * 2; i++) {
            int idx = i / segments;
            float x = poleVerts[i * 3];
            float y = poleVerts[i * 3 + 1];
            float z = poleVerts[i * 3 + 2];
            float nx = x / poleRadius;
            float nz = z / poleRadius;
            pole.addVertex(base.x + x, base.y + y, base.z + z, nx, 0, nz);
        }
        
        for (int i = 0; i < segments; i++) {
            uint32_t i0 = i;
            uint32_t i1 = (i + 1) % segments;
            uint32_t i2 = i + segments;
            uint32_t i3 = (i + 1) % segments + segments;
            pole.addTriangle(i0, i1, i2);
            pole.addTriangle(i1, i3, i2);
        }
        
        allMeshes.push_back(pole);
        
        if (sig.height > 0) {
            Mesh sign;
            sign.name = "signal_sign_" + sig.id;
            
            float signWidth = 0.5f;
            float signHeight = 0.8f;
            float signThickness = 0.02f;
            
            Vec3 signCenter = base;
            signCenter.z += (float)(sig.height > 0 ? sig.height : 2.0f);
            
            std::vector<Vec3> signVerts;
            signVerts.reserve(8);
            signVerts.push_back(signCenter + Vec3(-signWidth, -signHeight, -signThickness));
            signVerts.push_back(signCenter + Vec3( signWidth, -signHeight, -signThickness));
            signVerts.push_back(signCenter + Vec3( signWidth,  signHeight, -signThickness));
            signVerts.push_back(signCenter + Vec3(-signWidth,  signHeight, -signThickness));
            signVerts.push_back(signCenter + Vec3(-signWidth, -signHeight,  signThickness));
            signVerts.push_back(signCenter + Vec3( signWidth, -signHeight,  signThickness));
            signVerts.push_back(signCenter + Vec3( signWidth,  signHeight,  signThickness));
            signVerts.push_back(signCenter + Vec3(-signWidth,  signHeight,  signThickness));
            
            for (auto& v : signVerts) {
                sign.addVertex(v.x, v.y, v.z, 0, 0, 0);
            }
            
            std::vector<uint32_t> triDef = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11 };
            for (size_t i = 0; i < triDef.size(); i += 3) {
                sign.addTriangle(triDef[i], triDef[i+1], triDef[i+2]);
            }
            
            allMeshes.push_back(sign);
        }
    }
    
    return allMeshes;
}
