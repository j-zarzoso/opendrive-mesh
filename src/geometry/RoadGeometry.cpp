#include "RoadGeometry.h"
#include <algorithm>
#include <cmath>

RoadGeometry::RoadGeometry(const Road& road) : road_(road) {}

const Geometry* RoadGeometry::findGeometry(double s) const {
    for (int i = static_cast<int>(road_.planViewGeometries.size()) - 1; i >= 0; i--) {
        const auto& geom = road_.planViewGeometries[i];
        if (geom.s <= s) return &geom;
    }
    return nullptr;
}
Vec3 RoadGeometry::getPosition(double s) const {
    if (road_.planViewGeometries.empty()) {
        return {0, 0, 0};
    }
    
    Vec3 position = {0, 0, 0};
    Vec3 tangent = {1, 0, 0};
    
    for (auto& geom : road_.planViewGeometries) {
        if (geom.s + geom.length > s) {
            double localS = s - geom.s;
            
            switch (geom.type) {
                case GEOM_LINE: {
                    return Vec3(position.x + tangent.x * localS,
                               position.y + tangent.y * localS, position.z);
                }
                case GEOM_ARC: {
                    double radius = (std::abs(geom.curvature) > 1e-10) ? 1.0 / geom.curvature : 1e10;
                    double theta = localS / radius;
                    double sign = (geom.curvature > 0) ? 1 : -1;
                    double cx = position.x + tangent.y * radius;
                    double cy = position.y - tangent.x * radius;
                    double nx = radius * std::sin(theta * sign);
                    double ny = radius * (1 - std::cos(theta * sign));
                    return Vec3(cx + nx * tangent.x - ny * tangent.y,
                               cy + nx * tangent.y + ny * tangent.x, 0);
                }
                case GEOM_SPIRAL: {
                    double curvMid = (geom.curvStart + geom.curvEnd) / 2;
                    double radius = std::abs(curvMid) > 1e-10 ? 1.0 / std::abs(curvMid) : 1e10;
                    double theta = localS / radius;
                    double sign = (curvMid > 0) ? 1 : -1;
                    double cx = position.x + tangent.y * radius;
                    double cy = position.y - tangent.x * radius;
                    double nx = radius * std::sin(theta * sign);
                    double ny = radius * (1 - std::cos(theta * sign));
                    return Vec3(cx + nx * tangent.x - ny * tangent.y,
                               cy + nx * tangent.y + ny * tangent.x, 0);
                }
                case GEOM_POLY3: {
                    double dx = localS + geom.b * localS * localS + geom.c * localS * localS * localS;
                    double dy = geom.a * localS;
                    return Vec3(position.x + tangent.x * dx + tangent.y * dy,
                               position.y - tangent.y * dx + tangent.x * dy, 0);
                }
                case GEOM_PARAMPOLY3: {
                    Vec3 result = evalParamPoly3(geom.aU, geom.aV, geom.bU, geom.bV,
                                        geom.cU, geom.cV, geom.dU, geom.dV,
                                        localS, geom.pRangeNormalized, geom.length);
                    return result + Vec3(geom.x, geom.y, 0);
                }
            }
        }
        
        // Process full segment to get to next segment
        Vec3 nextPos = position;
        double segLen = geom.length;
        
        switch (geom.type) {
            case GEOM_LINE: {
                nextPos = Vec3(position.x + tangent.x * segLen,
                              position.y + tangent.y * segLen, 0);
                break;
            }
            case GEOM_ARC: {
                double radius = (std::abs(geom.curvature) > 1e-10) ? 1.0 / geom.curvature : 1e10;
                double theta = segLen / radius;
                double sign = (geom.curvature > 0) ? 1 : -1;
                double cx = position.x + tangent.y * radius;
                double cy = position.y - tangent.x * radius;
                double nx = radius * std::sin(theta * sign);
                double ny = radius * (1 - std::cos(theta * sign));
                nextPos = Vec3(cx + nx * tangent.x - ny * tangent.y,
                              cy + nx * tangent.y + ny * tangent.x, 0);
                break;
            }
            case GEOM_SPIRAL: {
                double curvMid = (geom.curvStart + geom.curvEnd) / 2;
                double radius = std::abs(curvMid) > 1e-10 ? 1.0 / std::abs(curvMid) : 1e10;
                double theta = segLen / radius;
                double sign = (curvMid > 0) ? 1 : -1;
                double cx = position.x + tangent.y * radius;
                double cy = position.y - tangent.x * radius;
                double nx = radius * std::sin(theta * sign);
                double ny = radius * (1 - std::cos(theta * sign));
                nextPos = Vec3(cx + nx * tangent.x - ny * tangent.y,
                              cy + nx * tangent.y + ny * tangent.x, 0);
                break;
            }
            case GEOM_POLY3: {
                double dx = segLen + geom.b * segLen * segLen + geom.c * segLen * segLen * segLen;
                double dy = geom.a * segLen;
                nextPos = Vec3(position.x + tangent.x * dx + tangent.y * dy,
                              position.y - tangent.y * dx + tangent.x * dy, 0);
                break;
            }
            case GEOM_PARAMPOLY3: {
                nextPos = evalParamPoly3(geom.aU, geom.aV, geom.bU, geom.bV,
                                        geom.cU, geom.cV, geom.dU, geom.dV,
                                        segLen, geom.pRangeNormalized, geom.length);
                nextPos = nextPos + Vec3(geom.x, geom.y, 0);
                break;
            }
        }
        
        position = nextPos;
        // Update tangent for next segment (rotate 90 degrees in XY plane)
        tangent = Vec3(tangent.y, -tangent.x, 0);
    }
    
    return position;
}

Vec3 RoadGeometry::getTangent(double s) const {
    double eps = 0.01;
    Vec3 p0 = getPosition(std::max(0.0, s - eps));
    Vec3 p1 = getPosition(std::min(road_.length, s + eps));
    return (p1 - p0).normalized();
}

double RoadGeometry::getElevation(double s) const {
    if (road_.elevations.empty()) return 0;
    
    for (int i = static_cast<int>(road_.elevations.size()) - 1; i >= 0; i--) {
        const auto& ep = road_.elevations[i];
        if (ep.s <= s) {
            double ds = s - ep.s;
            return ep.a + ep.b * ds + ep.c * ds * ds + ep.d * ds * ds * ds;
        }
    }
    return 0;
}

double RoadGeometry::getSuperelevation(double s) const {
    if (road_.superelevations.empty()) return 0;
    
    for (int i = static_cast<int>(road_.superelevations.size()) - 1; i >= 0; i--) {
        const auto& se = road_.superelevations[i];
        if (se.s <= s) {
            double ds = s - se.s;
            return se.a + se.b * ds + se.c * ds * ds + se.d * ds * ds * ds;
        }
    }
    return 0;
}

double RoadGeometry::getLaneOffset(double s) const {
    if (road_.laneOffsets.empty()) return 0;
    
    for (int i = static_cast<int>(road_.laneOffsets.size()) - 1; i >= 0; i--) {
        const auto& lo = road_.laneOffsets[i];
        if (lo.s <= s) {
            double ds = s - lo.s;
            return lo.a + lo.b * ds + lo.c * ds * ds + lo.d * ds * ds * ds;
        }
    }
    return 0;
}

double RoadGeometry::getLaneWidth(int laneId, double s) const {
    for (const auto& section : road_.laneSections) {
        if (section.s <= s) {
            for (const auto& lane : section.leftLanes) {
                if (lane.id == laneId) {
                    for (int i = static_cast<int>(lane.widths.size()) - 1; i >= 0; i--) {
                        const auto& w = lane.widths[i];
                        if (w.s <= s - section.s) {
                            double ds = s - section.s - w.s;
                            return w.a + w.b * ds + w.c * ds * ds + w.d * ds * ds * ds;
                        }
                    }
                    return 0;
                }
            }
            for (const auto& lane : section.rightLanes) {
                if (lane.id == laneId) {
                    for (int i = static_cast<int>(lane.widths.size()) - 1; i >= 0; i--) {
                        const auto& w = lane.widths[i];
                        if (w.s <= s - section.s) {
                            double ds = s - section.s - w.s;
                            return w.a + w.b * ds + w.c * ds * ds + w.d * ds * ds * ds;
                        }
                    }
                    return 0;
                }
            }
        }
    }
    return 0;
}

double RoadGeometry::getLaneBoundary(int laneId, double s, bool leftSide) {
    double t = 0;
    
    for (const auto& section : road_.laneSections) {
        if (section.s <= s) {
            if (leftSide) {
                for (const auto& lane : section.leftLanes) {
                    if (lane.id <= laneId) {
                        t += getLaneWidth(lane.id, s);
                    }
                }
            } else {
                for (const auto& lane : section.rightLanes) {
                    if (lane.id >= laneId) {
                        t -= getLaneWidth(lane.id, s);
                    }
                }
            }
        }
    }
    
    return t;
}

RoadSample RoadGeometry::sample(double s) const {
    RoadSample sample;
    sample.s = s;
    sample.position = getPosition(s);
    sample.frame = computeFrame(s);
    sample.elevation = getElevation(s);
    sample.superelevation = getSuperelevation(s);
    sample.laneOffset = getLaneOffset(s);
    
    for (const auto& section : road_.laneSections) {
        if (section.s <= s) {
            sample.leftLanes = section.leftLanes;
            sample.rightLanes = section.rightLanes;
        }
    }
    
    return sample;
}

std::vector<RoadSample> RoadGeometry::sampleRange(double stepSize) const {
    std::vector<RoadSample> samples;
    for (double s = 0; s <= road_.length; s += stepSize) {
        samples.push_back(sample(std::min(s, road_.length)));
    }
    return samples;
}

FrenetFrame RoadGeometry::computeFrame(double s) const {
    Vec3 p0 = getPosition(std::max(0.0, s - 0.1));
    Vec3 p1 = getPosition(s);
    Vec3 p2 = getPosition(std::min(road_.length, s + 0.1));
    
    Vec3 tangent = (p2 - p0).normalized();
    Vec3 binormal = {0, 0, 1};
    
    FrenetFrame frame;
    frame.tangent = tangent.normalized();
    frame.binormal = binormal.normalized();
    frame.normal = frame.binormal.cross(frame.tangent).normalized();
    frame.position = p1;
    frame.s = s;
    return frame;
}
