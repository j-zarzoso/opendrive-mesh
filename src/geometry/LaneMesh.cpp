#include "LaneMesh.h"
#include "Math.h"
#include <algorithm>
#include <cmath>

LaneMesh::LaneMesh(const Road& road, const RoadGeometry& geometry, double stepSize)
    : road_(road), geometry_(geometry), stepSize_(stepSize) {}

Mesh LaneMesh::generate() const {
    Mesh mesh;
    mesh.name = "road_" + std::to_string(road_.id);
    
    if (road_.laneSections.empty()) return mesh;
    
    // Generate mesh for each lane section
    for (size_t i = 0; i < road_.laneSections.size(); i++) {
        const auto& section = road_.laneSections[i];
        double sStart = section.s;
        double sEnd = (i + 1 < road_.laneSections.size()) ? 
                      road_.laneSections[i + 1].s : road_.length;
        
        auto sectionMesh = generateLaneSection(section, sStart, sEnd);
        mesh.vertices.insert(mesh.vertices.end(), 
                           sectionMesh.vertices.begin(), 
                           sectionMesh.vertices.end());
        
        // Offset triangle indices
        uint32_t baseIndex = static_cast<uint32_t>(mesh.triangles.size() * 3);
        for (const auto& tri : sectionMesh.triangles) {
            mesh.triangles.push_back({tri.i0 + baseIndex, tri.i1 + baseIndex, tri.i2 + baseIndex});
        }
    }
    
    return mesh;
}

Mesh LaneMesh::generateLaneSection(const LaneSection& section, double sStart, double sEnd) const {
    Mesh mesh;
    
    // Generate for center lane (id = 0) - this is the reference line, has no width
    // But we still need its position for lane boundaries
    
    // Generate for left lanes (positive IDs)
    for (const auto& lane : section.leftLanes) {
        auto laneMesh = generateLane(lane, sStart, sEnd, 1);
        uint32_t baseIndex = static_cast<uint32_t>(mesh.vertices.size());
        for (const auto& v : laneMesh.vertices) {
            mesh.vertices.push_back(v);
        }
        for (const auto& tri : laneMesh.triangles) {
            mesh.triangles.push_back({tri.i0 + baseIndex, tri.i1 + baseIndex, tri.i2 + baseIndex});
        }
    }
    
    // Generate for right lanes (negative IDs)
    for (const auto& lane : section.rightLanes) {
        auto laneMesh = generateLane(lane, sStart, sEnd, -1);
        uint32_t baseIndex = static_cast<uint32_t>(mesh.vertices.size());
        for (const auto& v : laneMesh.vertices) {
            mesh.vertices.push_back(v);
        }
        for (const auto& tri : laneMesh.triangles) {
            mesh.triangles.push_back({tri.i0 + baseIndex, tri.i1 + baseIndex, tri.i2 + baseIndex});
        }
    }
    
    return mesh;
}

Mesh LaneMesh::generateLane(const Lane& lane, double sStart, double sEnd, int side) const {
    Mesh mesh;
    
    // For each sample point along s
    for (double s = sStart; s <= sEnd; s += stepSize_) {
        double clampedS = std::min(s, sEnd - stepSize_ * 0.5);
        
        // Get road sample
        auto sample = geometry_.sample(clampedS);
        
        // Get lane width at this s
        double width = geometry_.getLaneWidth(lane.id, clampedS);
        
  
        
        // Compute inner and outer lane edges based on lane type
        double tInner, tOuter;
        
        // For left lanes (side = 1): lane 1 is innermost (closest to center)
        // For right lanes (side = -1): lane -1 is innermost (closest to center)
        if (side == 1) {
            // Left lane: inner edge is towards center (less negative t)
            // Outer edge is away from center (more negative t)
            tInner = getLaneTPosition(lane.id, clampedS);
            tOuter = tInner - width;
        } else {
            // Right lane: inner edge is towards center (less positive t)
            // Outer edge is away from center (more positive t)
            tInner = getLaneTPosition(lane.id, clampedS);
            tOuter = tInner + width;
        }
        
        // Get elevation and superelevation
        double elevation = geometry_.getElevation(clampedS);
        double superelev = geometry_.getSuperelevation(clampedS);
        
        // Compute vertices at inner and outer edges
        Vec3 innerPos = sample.frame.getPoint(0, tInner, elevation);
        Vec3 outerPos = sample.frame.getPoint(0, tOuter, elevation);
        
        // Apply superelevation
        innerPos = rotateSuperelevation(innerPos, superelev);
        outerPos = rotateSuperelevation(outerPos, superelev);
        
        // Convert to USD coords
        Vec3 usdInner = toUsdCoords(innerPos);
        Vec3 usdOuter = toUsdCoords(outerPos);
        
        // Compute normal (pointing up)
        Vec3 normal = sample.frame.binormal.normalized();
        
        // Add vertices
        mesh.addVertex(static_cast<float>(usdInner.x), static_cast<float>(usdInner.y), static_cast<float>(usdInner.z),
                      static_cast<float>(normal.x), static_cast<float>(normal.y), static_cast<float>(normal.z));
        mesh.addVertex(static_cast<float>(usdOuter.x), static_cast<float>(usdOuter.y), static_cast<float>(usdOuter.z),
                      static_cast<float>(normal.x), static_cast<float>(normal.y), static_cast<float>(normal.z));
    }
    
    // Generate triangles (strip)
    size_t numVerts = mesh.vertices.size() / 2;
    for (size_t i = 0; i + 1 < numVerts; i++) {
        uint32_t i0 = static_cast<uint32_t>(i * 2);
        uint32_t i1 = static_cast<uint32_t>(i * 2 + 1);
        uint32_t i2 = static_cast<uint32_t>(i * 2 + 2);
        uint32_t i3 = static_cast<uint32_t>(i * 2 + 3);
        
        mesh.addTriangle(i0, i1, i2);
        mesh.addTriangle(i1, i3, i2);
    }
    
    return mesh;
}

std::vector<Lane> LaneMesh::getActiveLanes(double s) const {
    std::vector<Lane> result;
    
    for (const auto& section : road_.laneSections) {
        // Find which section contains s
    }
    
    return result;
}

double LaneMesh::getLaneTPosition(int laneId, double s) const {
    for (const auto& section : road_.laneSections) {
        if (section.s <= s) {
            // For right lanes (negative IDs): inner edge is at cumulative sum from centerline
            if (laneId < 0) {
                double rightCum = 0;
                for (const auto& lane : section.rightLanes) {
                    if (lane.id == laneId) {
                        return rightCum;
                    }
                    rightCum += geometry_.getLaneWidth(lane.id, s);
                }
            }
            // For left lanes (positive IDs): process from innermost to outermost
            else if (laneId > 0) {
                double leftCum = 0;
                for (int i = static_cast<int>(section.leftLanes.size()) - 1; i >= 0; i--) {
                    const auto& lane = section.leftLanes[i];
                    if (lane.id == laneId) {
                        return leftCum;
                    }
                    leftCum -= geometry_.getLaneWidth(lane.id, s);
                }
            }
            // Center lane (id = 0)
            return 0;
        }
    }
    return 0;
}
