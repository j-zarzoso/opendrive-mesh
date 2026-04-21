#include "ObjectMesh.h"
#include "Math.h"

ObjectMesh::ObjectMesh(const Road& road, const RoadGeometry& geometry)
    : road_(road), geometry_(geometry) {}

std::vector<Mesh> ObjectMesh::generate() {
    std::vector<Mesh> allMeshes;
    
    for (auto& obj : road_.objects) {
        Mesh mesh;
        mesh.name = "object_" + obj.id;
        
        float hw = static_cast<float>(obj.width / 2.0);
        float hh = static_cast<float>(obj.height / 2.0);
        float hl = static_cast<float>(obj.length / 2.0);
        
        if (obj.hasRepeat) {
            double dist = obj.repeat.distance;
            double sEnd = obj.s + obj.repeat.length;
            double currentS = obj.s;
            
            while (currentS <= sEnd) {
                auto sample = geometry_.sample(currentS);
                Vec3 c = sample.position + sample.frame.normal * obj.t;
                
                std::vector<Vec3> boxVerts = {
                    c + Vec3(-hw, -hh, -hl),
                    c + Vec3( hw, -hh, -hl),
                    c + Vec3( hw,  hh, -hl),
                    c + Vec3(-hw,  hh, -hl),
                    c + Vec3(-hw, -hh,  hl),
                    c + Vec3( hw, -hh,  hl),
                    c + Vec3( hw,  hh,  hl),
                    c + Vec3(-hw,  hh,  hl)
                };
                
                for (auto& v : boxVerts) {
                    mesh.addVertex(v.x, v.y, v.z, 0, 0, 0);
                }
                
                uint32_t triDef[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11 };
                for (size_t i = 0; i < 18; i += 3) {
                    mesh.addTriangle(triDef[i], triDef[i+1], triDef[i+2]);
                }
                
                currentS += dist;
            }
        } else {
            auto sample = geometry_.sample(obj.s);
            Vec3 c = sample.position + sample.frame.normal * obj.t;
            
            std::vector<Vec3> boxVerts = {
                c + Vec3(-hw, -hh, -hl),
                c + Vec3( hw, -hh, -hl),
                c + Vec3( hw,  hh, -hl),
                c + Vec3(-hw,  hh, -hl),
                c + Vec3(-hw, -hh,  hl),
                c + Vec3( hw, -hh,  hl),
                c + Vec3( hw,  hh,  hl),
                c + Vec3(-hw,  hh,  hl)
            };
            
            for (auto& v : boxVerts) {
                mesh.addVertex(v.x, v.y, v.z, 0, 0, 0);
            }
            
            uint32_t triDef[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11 };
            for (size_t i = 0; i < 18; i += 3) {
                mesh.addTriangle(triDef[i], triDef[i+1], triDef[i+2]);
            }
        }
        
        if (mesh.vertices.size() > 0) {
            allMeshes.push_back(mesh);
        }
    }
    
    return allMeshes;
}
