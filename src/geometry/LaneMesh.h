#pragma once
#include "types/MeshTypes.h"
#include "geometry/RoadGeometry.h"
#include "parser/types.h"
#include <vector>

class LaneMesh {
public:
    LaneMesh(const Road& road, const RoadGeometry& geometry, double stepSize);
    
    // Generate mesh for all lanes in the road
    Mesh generate() const;
    
    // Generate mesh for a specific lane section
    Mesh generateLaneSection(const LaneSection& section, double sStart, double sEnd) const;
    
    // Generate mesh for a single lane
    Mesh generateLane(const Lane& lane, double sStart, double sEnd, int side) const;
    
private:
    const Road& road_;
    const RoadGeometry& geometry_;
    double stepSize_;
    
    // Get lanes active in a given s range
    std::vector<Lane> getActiveLanes(double s) const;
    
    // Compute lane t-position from center
    double getLaneTPosition(int laneId, double s) const;
};
