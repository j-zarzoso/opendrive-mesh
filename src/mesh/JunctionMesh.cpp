#include "JunctionMesh.h"
#include "geometry/Math.h"
#include "geometry/RoadGeometry.h"
#include "types/MeshTypes.h"

JunctionMesh::JunctionMesh(const OpenDRIVEFile& file, double stepSize)
    : file_(file), stepSize_(stepSize) {}

Vec3 JunctionMesh::getRoadCenterPosition(int roadId, double s) {
    for (const auto& road : file_.roads) {
        if (road.id == roadId) {
            RoadGeometry geometry(road);
            return geometry.getPosition(s);
        }
    }
    return Vec3(0, 0, 0);
}

void JunctionMesh::generateJunctionSurfaceMesh(const Junction& junction, std::vector<Mesh>& meshes) {
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    
    for (int roadId : junction.roadIds) {
        for (const auto& road : file_.roads) {
            if (road.id == roadId) {
                RoadGeometry geometry(road);
                double step = stepSize_;
                for (double s = 0; s < road.length; s += step) {
                    Vec3 pos = geometry.getPosition(s);
                    if (pos.x < minX) minX = pos.x;
                    if (pos.x > maxX) maxX = pos.x;
                    if (pos.y < minY) minY = pos.y;
                    if (pos.y > maxY) maxY = pos.y;
                }
            }
        }
    }
    
    double buffer = 5.0;
    minX -= buffer; maxX += buffer;
    minY -= buffer; maxY += buffer;
    
    double zMid = 0;
    
    std::vector<Vec3> verts = {
        {minX, minY, zMid}, {maxX, minY, zMid},
        {minX, maxY, zMid}, {maxX, maxY, zMid}
    };
    
    Mesh plane;
    plane.name = "junction_surface";
    plane.type = MESH_TYPE_PLANAR;
    plane.meshIndex = junction.id;
    
    for (const auto& v : verts) {
        Vertex vert;
        vert.x = static_cast<float>(v.x);
        vert.y = static_cast<float>(v.y);
        vert.z = static_cast<float>(v.z);
        vert.nx = 0; vert.ny = 0; vert.nz = 1;
        vert.u = 0; vert.v = 0;
        plane.vertices.push_back(vert);
    }
    
    plane.triangles.push_back({0, 1, 2});
    plane.triangles.push_back({2, 1, 3});
    
    meshes.push_back(plane);
}

void JunctionMesh::generateJunctionLaneMesh(const Junction& junction, const Road& road, std::vector<Mesh>& meshes) {
    RoadGeometry geometry(road);
    
    for (const auto& ls : road.laneSections) {
        for (const auto& lane : ls.centerLane) {
            if (lane.type != LANE_DRIVING && lane.type != LANE_BIDIRECTIONAL) continue;
            
            double laneWidth = 3.5;
            double halfWidth = laneWidth / 2.0;
            
            std::vector<Vec3> leftBoundary, rightBoundary;
            
            for (double s = 0; s < road.length; s += stepSize_) {
                auto sample = geometry.sample(s);
                Vec3 pos = sample.position;
                FrenetFrame frame = sample.frame;
                
                Vec3 left = pos + frame.normal * (-halfWidth) + frame.binormal * sample.elevation;
                Vec3 right = pos + frame.normal * (halfWidth) + frame.binormal * sample.elevation;
                
                leftBoundary.push_back(left);
                rightBoundary.push_back(right);
            }
            
            if (leftBoundary.size() < 2) continue;
            
            for (size_t i = 0; i < leftBoundary.size() - 1; i++) {
                Mesh strip;
                strip.name = "junction_lane_" + std::to_string(lane.id);
                strip.type = MESH_TYPE_PLANAR;
                strip.meshIndex = junction.id * 100 + static_cast<int>(lane.id);
                
                std::vector<Vec3> verts = {
                    leftBoundary[i], rightBoundary[i],
                    leftBoundary[i + 1], rightBoundary[i + 1]
                };
                
                for (const auto& v : verts) {
                    Vertex vert;
                    vert.x = static_cast<float>(v.x);
                    vert.y = static_cast<float>(v.y);
                    vert.z = static_cast<float>(v.z);
                    vert.nx = 0; vert.ny = 0; vert.nz = 1;
                    vert.u = 0; vert.v = 0;
                    strip.vertices.push_back(vert);
                }
                
                strip.triangles.push_back({0, 1, 2});
                strip.triangles.push_back({2, 1, 3});
                
                meshes.push_back(strip);
            }
        }
    }
}

void JunctionMesh::generateConnectionMesh(const Junction& junction, const OutgoingConnection& conn, std::vector<Mesh>& meshes) {
    for (const auto& road : file_.roads) {
        if (road.id == conn.roadId) {
            RoadGeometry geometry(road);
            Vec3 pos = geometry.getPosition(0);
            
            Mesh box;
            box.name = "junction_conn_" + std::to_string(conn.roadId);
            box.type = MESH_TYPE_BOX;
            box.meshIndex = junction.id * 1000 + conn.roadId;
            
            std::vector<Vec3> boxVerts = {
                {pos.x - 1.0, pos.y - 1.0, pos.z - 0.5}, {pos.x + 1.0, pos.y - 1.0, pos.z - 0.5},
                {pos.x - 1.0, pos.y + 1.0, pos.z - 0.5}, {pos.x + 1.0, pos.y + 1.0, pos.z - 0.5},
                {pos.x - 1.0, pos.y - 1.0, pos.z + 0.5}, {pos.x + 1.0, pos.y - 1.0, pos.z + 0.5},
                {pos.x - 1.0, pos.y + 1.0, pos.z + 0.5}, {pos.x + 1.0, pos.y + 1.0, pos.z + 0.5}
            };
            
            for (const auto& v : boxVerts) {
                Vertex vert;
                vert.x = static_cast<float>(v.x);
                vert.y = static_cast<float>(v.y);
                vert.z = static_cast<float>(v.z);
                vert.nx = 0; vert.ny = 0; vert.nz = 1;
                vert.u = 0; vert.v = 0;
                box.vertices.push_back(vert);
            }
            
            uint32_t indices[] = {
                0, 1, 2, 2, 1, 3,
                5, 4, 7, 7, 4, 6,
                2, 3, 6, 6, 3, 7,
                4, 5, 0, 0, 5, 1,
                1, 5, 3, 3, 5, 7,
                4, 0, 6, 6, 0, 4
            };
            
            for (int i = 0; i < 12; i++) {
                box.triangles.push_back({indices[i * 2], indices[i * 2 + 1], indices[i * 2 + 2]});
            }
            
            meshes.push_back(box);
            break;
        }
    }
}

std::vector<Mesh> JunctionMesh::generate() {
    std::vector<Mesh> meshes;
    
    for (const auto& junction : file_.junctions) {
        generateJunctionSurfaceMesh(junction, meshes);
        
        for (int roadId : junction.roadIds) {
            for (const auto& road : file_.roads) {
                if (road.id == roadId) {
                    generateJunctionLaneMesh(junction, road, meshes);
                }
            }
        }
        
        for (const auto& mg : junction.maneuverGroups) {
            for (const auto& oc : mg.outgoingConnections) {
                generateConnectionMesh(junction, oc, meshes);
            }
        }
    }
    
    return meshes;
}
