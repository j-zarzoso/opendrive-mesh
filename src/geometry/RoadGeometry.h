#pragma once
#include "Math.h"
#include "parser/types.h"
#include <vector>

struct RoadSample {
    Vec3 position;
    FrenetFrame frame;
    double s;
    double elevation;
    double superelevation;
    double laneOffset;
    std::vector<Lane> leftLanes;
    std::vector<Lane> rightLanes;
};

class RoadGeometry {
public:
    RoadGeometry(const Road& road);
    RoadSample sample(double s) const;
    std::vector<RoadSample> sampleRange(double stepSize) const;
    Vec3 getPosition(double s) const;
    Vec3 getTangent(double s) const;
    double getElevation(double s) const;
    double getSuperelevation(double s) const;
    double getLaneOffset(double s) const;
    double getLaneWidth(int laneId, double s) const;
    double getLaneBoundary(int laneId, double s, bool leftSide);
    const Road& road() const { return road_; }

private:
    const Road& road_;
    const Geometry* findGeometry(double s) const;
    FrenetFrame computeFrame(double s) const;
};
