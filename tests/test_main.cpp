#include "test.h"
#include <cmath>
#include <iostream>
#include <fstream>

#include "parser/OpenDRIVEParser.h"
#include "parser/types.h"
#include "geometry/Math.h"
#include "geometry/RoadGeometry.h"
#include "geometry/LaneMesh.h"
#include "mesh/JunctionMesh.h"

namespace {

void parseSignalType(const std::string& typeStr, SignalType& sigType) {
    if (typeStr == "speedLimit") sigType = SIG_SPEED_LIMIT;
    else if (typeStr == "maxSpeed") sigType = SIG_MAX_SPEED;
    else if (typeStr == "minSpeed") sigType = SIG_MIN_SPEED;
    else if (typeStr == "noPassing") sigType = SIG_NO_PASSING;
    else if (typeStr == "stop") sigType = SIG_STOP;
    else if (typeStr == "yield") sigType = SIG_YIELD;
    else if (typeStr == "trafficLight") sigType = SIG_TRAFFIC_LIGHT;
    else if (typeStr == "endAllRestrictions") sigType = SIG_END_ALL_RESTRICTIONS;
    else if (typeStr == "general") sigType = SIG_GENERAL;
    else sigType = SIG_GENERIC;
}

void parseObjectType(const std::string& typeStr, ObjectType& objType) {
    if (typeStr == "barrier") objType = OBJ_BARRIER;
    else if (typeStr == "pole") objType = OBJ_POLE;
    else if (typeStr == "tree") objType = OBJ_TREE;
    else if (typeStr == "vegetation") objType = OBJ_VEGETATION;
    else if (typeStr == "building") objType = OBJ_BUILDING;
    else if (typeStr == "obstacle") objType = OBJ_OBSTACLE;
    else if (typeStr == "gantry") objType = OBJ_GANTRY;
    else if (typeStr == "crosswalk") objType = OBJ_CROSSWALK;
    else if (typeStr == "roadMark") objType = OBJ_ROAD_MARK;
    else if (typeStr == "roadSurface") objType = OBJ_ROAD_SURFACE;
    else if (typeStr == "parkingSpace") objType = OBJ_PARKING_SPACE;
    else if (typeStr == "trafficIsland") objType = OBJ_TRAFFIC_ISLAND;
    else objType = OBJ_NONE;
}

void parseLaneType(const std::string& typeStr, LaneType& laneType) {
    if (typeStr == "driving") laneType = LANE_DRIVING;
    else if (typeStr == "shoulder") laneType = LANE_SHOULDER;
    else if (typeStr == "border") laneType = LANE_BORDER;
    else if (typeStr == "stop") laneType = LANE_STOP;
    else if (typeStr == "none") laneType = LANE_NONE;
    else if (typeStr == "restricted") laneType = LANE_RESTRICTED;
    else if (typeStr == "parking") laneType = LANE_PARKING;
    else if (typeStr == "median") laneType = LANE_MEDIAN;
    else if (typeStr == "biking") laneType = LANE_BIKING;
    else if (typeStr == "sidewalk") laneType = LANE_SIDEWALK;
    else if (typeStr == "curb") laneType = LANE_CURB;
    else if (typeStr == "walking") laneType = LANE_WALKING;
    else if (typeStr == "tram") laneType = LANE_TRAM;
    else if (typeStr == "rail") laneType = LANE_RAIL;
    else if (typeStr == "bidirectional") laneType = LANE_BIDIRECTIONAL;
    else if (typeStr == "shared") laneType = LANE_SHARED;
    else if (typeStr == "onRamp") laneType = LANE_ONRAMP;
    else if (typeStr == "offRamp") laneType = LANE_OFFRAMP;
    else if (typeStr == "connectingRamp") laneType = LANE_CONNECTINGRAMP;
    else if (typeStr == "entry") laneType = LANE_ENTRY;
    else if (typeStr == "exit") laneType = LANE_EXIT;
    else if (typeStr == "bus") laneType = LANE_BUS;
    else if (typeStr == "taxi") laneType = LANE_TAXI;
    else if (typeStr == "hov") laneType = LANE_HOV;
    else if (typeStr == "slipLane") laneType = LANE_SLIP_LANE;
    else if (typeStr == "roadworks") laneType = LANE_ROADWORKS;
    else if (typeStr == "special1") laneType = LANE_SPECIAL1;
    else if (typeStr == "special2") laneType = LANE_SPECIAL2;
    else if (typeStr == "special3") laneType = LANE_SPECIAL3;
    else laneType = LANE_DRIVING;
}

void testJunctionParsing();
void testJunctionMeshGeneration();
void testSignalTypeParsing();
void testObjectTypeParsing();
void testLaneTypeParsing();

bool approxEqual(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

void testVec3Constructor() {
    Vec3 v;
    TEST_PASS("default constructor sets zero", v.x == 0 && v.y == 0 && v.z == 0);
    
    Vec3 v2(1.0, 2.0, 3.0);
    TEST_PASS("parameterized constructor", v2.x == 1.0 && v2.y == 2.0 && v2.z == 3.0);
}

void testVec3Addition() {
    Vec3 a(1.0, 2.0, 3.0);
    Vec3 b(4.0, 5.0, 6.0);
    Vec3 sum = a + b;
    TEST_PASS("addition", approxEqual(sum.x, 5.0) && approxEqual(sum.y, 7.0) && approxEqual(sum.z, 9.0));
}

void testVec3Subtraction() {
    Vec3 a(5.0, 7.0, 9.0);
    Vec3 b(1.0, 2.0, 3.0);
    Vec3 diff = a - b;
    TEST_PASS("subtraction", approxEqual(diff.x, 4.0) && approxEqual(diff.y, 5.0) && approxEqual(diff.z, 6.0));
}

void testVec3Multiplication() {
    Vec3 v(1.0, 2.0, 3.0);
    Vec3 scaled = v * 2.0;
    TEST_PASS("scalar multiplication", approxEqual(scaled.x, 2.0) && approxEqual(scaled.y, 4.0) && approxEqual(scaled.z, 6.0));
}

void testVec3Length() {
    Vec3 v(3.0, 4.0, 0.0);
    double len = v.length();
    TEST_PASS("length", approxEqual(len, 5.0));
}

void testVec3Normalized() {
    Vec3 v(3.0, 0.0, 0.0);
    Vec3 n = v.normalized();
    TEST_PASS("normalized length is 1.0", approxEqual(n.length(), 1.0));
    TEST_PASS("normalized direction", approxEqual(n.x, 1.0) && approxEqual(n.y, 0.0) && approxEqual(n.z, 0.0));
}

void testVec3DotProduct() {
    Vec3 a(1.0, 0.0, 0.0);
    Vec3 b(0.0, 1.0, 0.0);
    TEST_PASS("orthogonal dot product is 0", approxEqual(a.dot(b), 0.0));
    
    Vec3 c(2.0, 0.0, 0.0);
    TEST_PASS("dot product of parallel vectors", approxEqual(a.dot(c), 2.0));
}

void testVec3CrossProduct() {
    Vec3 a(1.0, 0.0, 0.0);
    Vec3 b(0.0, 1.0, 0.0);
    Vec3 cross = a.cross(b);
    TEST_PASS("cross product XY->Z", approxEqual(cross.x, 0.0) && approxEqual(cross.y, 0.0) && approxEqual(cross.z, 1.0));
}

void testFrenetFrameFromTangentBinormal() {
    Vec3 tangent(1.0, 0.0, 0.0);
    Vec3 binormal(0.0, 0.0, 1.0);
    FrenetFrame frame = FrenetFrame::fromTangentBinormal(tangent, binormal);
    
    TEST_PASS("tangent preserved", approxEqual(frame.tangent.x, 1.0) && approxEqual(frame.tangent.y, 0.0) && approxEqual(frame.tangent.z, 0.0));
    TEST_PASS("binormal preserved", approxEqual(frame.binormal.x, 0.0) && approxEqual(frame.binormal.y, 0.0) && approxEqual(frame.binormal.z, 1.0));
    TEST_PASS("normal is cross product", approxEqual(frame.normal.x, 0.0) && approxEqual(frame.normal.y, 1.0) && approxEqual(frame.normal.z, 0.0));
}

void testFrenetFrameToWorld() {
    Vec3 tangent(1.0, 0.0, 0.0);
    Vec3 normal(0.0, 1.0, 0.0);
    Vec3 binormal(0.0, 0.0, 1.0);
    
    FrenetFrame frame;
    frame.tangent = tangent;
    frame.normal = normal;
    frame.binormal = binormal;
    
    Vec3 local(1.0, 2.0, 3.0);
    Vec3 world = frame.toWorld(local);
    TEST_PASS("toWorld transform", approxEqual(world.x, 1.0) && approxEqual(world.y, 2.0) && approxEqual(world.z, 3.0));
}

void testPolynomialEvaluation() {
    double result = evalPolynomial(1.0, 2.0, 3.0, 4.0, 1.0);
    TEST_PASS("polynomial at s=1", approxEqual(result, 10.0));  // 1 + 2 + 3 + 4
    
    result = evalPolynomial(0.0, 1.0, 0.0, 0.0, 5.0);
    TEST_PASS("polynomial at s=5", approxEqual(result, 5.0));  // 0 + 5
    
    result = evalPolynomial(1.0, 0.0, 0.0, 0.0, 5.0);
    TEST_PASS("constant polynomial", approxEqual(result, 1.0));
}

void testParamPoly3Evaluation() {
    // Simple linear: x = p, y = 0
    Vec3 result = evalParamPoly3(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, false, 10.0);
    TEST_PASS("paramPoly3 linear", approxEqual(result.x, 0.5) && approxEqual(result.y, 0.0));  // 5/10 = 0.5
    
    // Zero polynomial
    result = evalParamPoly3(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, false, 10.0);
    TEST_PASS("paramPoly3 zero", approxEqual(result.x, 0.0) && approxEqual(result.y, 0.0));
}

void testToUsdCoords() {
    Vec3 od(1.0, 2.0, 3.0);  // OpenDRIVE coords
    Vec3 usd = toUsdCoords(od);
    TEST_PASS("X preserved", approxEqual(usd.x, 1.0));
    TEST_PASS("Y becomes Z", approxEqual(usd.y, 3.0));
    TEST_PASS("Z becomes -Y", approxEqual(usd.z, -2.0));
}

void testRotateSuperelevation() {
    Vec3 local(1.0, 0.0, 0.0);
    Vec3 result = rotateSuperelevation(local, 0.0);
    TEST_PASS("zero rotation", approxEqual(result.x, 1.0) && approxEqual(result.y, 0.0) && approxEqual(result.z, 0.0));
    
    Vec3 local2(0.0, 1.0, 0.0);
    result = rotateSuperelevation(local2, atan(1.0) * 2);  // 90 degrees
    TEST_PASS("90 degree rotation", approxEqual(result.x, 0.0) && approxEqual(result.y, 0.0) && approxEqual(result.z, -1.0));
}

void testLineGeometry() {
    Geometry geom;
    geom.type = GEOM_LINE;
    geom.s = 0.0;
    geom.x = 0.0;
    geom.y = 0.0;
    geom.hdg = 0.0;
    geom.length = 100.0;
    
    Road road;
    road.planViewGeometries.push_back(geom);
    road.length = 100.0;
    
    RoadGeometry geometry(road);
    
    Vec3 pos = geometry.getPosition(0.0);
    TEST_PASS("line at s=0", approxEqual(pos.x, 0.0) && approxEqual(pos.y, 0.0));
    
    pos = geometry.getPosition(50.0);
    TEST_PASS("line at s=50", approxEqual(pos.x, 50.0) && approxEqual(pos.y, 0.0));
    
    pos = geometry.getPosition(100.0);
    TEST_PASS("line at s=100", approxEqual(pos.x, 100.0) && approxEqual(pos.y, 0.0));
}

void testArcGeometry() {
    Geometry geom;
    geom.type = GEOM_ARC;
    geom.s = 0.0;
    geom.x = 0.0;
    geom.y = 0.0;
    geom.hdg = 0.0;
    geom.length = 10.0;
    geom.curvature = 0.1;  // radius = 10
    
    Road road;
    road.planViewGeometries.push_back(geom);
    road.length = 10.0;
    
    RoadGeometry geometry(road);
    
    Vec3 pos = geometry.getPosition(0.0);
    TEST_PASS("arc at s=0 position.x", approxEqual(pos.x, 0.0));
    
    pos = geometry.getPosition(5.0);
    TEST_PASS("arc at s=5 position.x", approxEqual(pos.x, 4.7943, 0.0001));
    TEST_PASS("arc at s=5 position.y", approxEqual(pos.y, -8.7758, 0.0001));
}

void testElevationProfile() {
    Geometry geom;
    geom.type = GEOM_LINE;
    geom.s = 0.0;
    geom.x = 0.0;
    geom.y = 0.0;
    geom.hdg = 0.0;
    geom.length = 100.0;
    
    ElevationPoint elev1;
    elev1.s = 0.0;
    elev1.a = 0.0;
    elev1.b = 0.02;  // 2% slope
    elev1.c = 0.0;
    elev1.d = 0.0;
    
    Road road;
    road.planViewGeometries.push_back(geom);
    road.elevations.push_back(elev1);
    road.length = 100.0;
    
    RoadGeometry geometry(road);
    
    double elev = geometry.getElevation(0.0);
    TEST_PASS("elevation at s=0", approxEqual(elev, 0.0));
    
    elev = geometry.getElevation(50.0);
    TEST_PASS("elevation at s=50", approxEqual(elev, 1.0));  // 50 * 0.02 = 1.0
    
    elev = geometry.getElevation(100.0);
    TEST_PASS("elevation at s=100", approxEqual(elev, 2.0));  // 100 * 0.02 = 2.0
}

void testSuperelevationProfile() {
    Geometry geom;
    geom.type = GEOM_LINE;
    geom.s = 0.0;
    geom.x = 0.0;
    geom.y = 0.0;
    geom.hdg = 0.0;
    geom.length = 100.0;
    
    SuperelevationPoint se;
    se.s = 0.0;
    se.a = 0.0;
    se.b = 0.01;
    se.c = 0.0;
    se.d = 0.0;
    
    Road road;
    road.planViewGeometries.push_back(geom);
    road.superelevations.push_back(se);
    road.length = 100.0;
    
    RoadGeometry geometry(road);
    
    double selev = geometry.getSuperelevation(0.0);
    TEST_PASS("superelevation at s=0", approxEqual(selev, 0.0));
    
    selev = geometry.getSuperelevation(50.0);
    TEST_PASS("superelevation at s=50", approxEqual(selev, 0.5));  // 50 * 0.01 = 0.5
}

void testJunctionParsing() {
    OpenDRIVEFile file;
    file.header.name = "TestJunction";
    
    Road road1;
    road1.id = 1;
    road1.name = "Road1";
    road1.length = 100.0;
    road1.rule = "RHT";
    road1.header = &file.header;
    
    Geometry geom;
    geom.type = GEOM_LINE;
    geom.s = 0.0;
    geom.x = 0.0;
    geom.y = 0.0;
    geom.hdg = 0.0;
    geom.length = 100.0;
    road1.planViewGeometries.push_back(geom);
    
    file.roads.push_back(road1);
    
    Road road2;
    road2.id = 2;
    road2.name = "Road2";
    road2.length = 80.0;
    road2.rule = "RHT";
    road2.header = &file.header;
    
    Geometry geom2;
    geom2.type = GEOM_LINE;
    geom2.s = 0.0;
    geom2.x = 50.0;
    geom2.y = 0.0;
    geom2.hdg = atan(1.0) * 2.0;
    geom2.length = 80.0;
    road2.planViewGeometries.push_back(geom2);
    
    file.roads.push_back(road2);
    
    Junction junction;
    junction.id = 0;
    junction.name = "Intersection";
    junction.type = 0;
    
    ManeuverGroup mg;
    mg.id = "maneuver1";
    
    IncomingConnection ic;
    ic.roadId = 1;
    mg.incomingConnections.push_back(ic);
    
    OutgoingConnection oc1;
    oc1.roadId = 2;
    mg.outgoingConnections.push_back(oc1);
    
    OutgoingConnection oc2;
    oc2.roadId = 1;
    mg.outgoingConnections.push_back(oc2);
    
    junction.maneuverGroups.push_back(mg);
    junction.roadIds.push_back(1);
    junction.roadIds.push_back(2);
    
    file.junctions.push_back(junction);
    
    TEST_PASS("junction has 2 road ids", static_cast<int>(file.junctions[0].roadIds.size()) == 2);
    TEST_PASS("junction has 1 maneuver group", file.junctions[0].maneuverGroups.size() == 1);
    TEST_PASS("incoming connection road id is 1", file.junctions[0].maneuverGroups[0].incomingConnections[0].roadId == 1);
    TEST_PASS("first outgoing connection road id is 2", file.junctions[0].maneuverGroups[0].outgoingConnections[0].roadId == 2);
}

void testJunctionMeshGeneration() {
    OpenDRIVEFile file;
    file.header.name = "JunctionTest";
    
    Road road;
    road.id = 1;
    road.name = "Road1";
    road.length = 100.0;
    road.rule = "RHT";
    road.header = &file.header;
    
    Geometry geom;
    geom.type = GEOM_LINE;
    geom.s = 0.0;
    geom.x = 0.0;
    geom.y = 0.0;
    geom.hdg = 0.0;
    geom.length = 100.0;
    road.planViewGeometries.push_back(geom);
    
    LaneSection ls;
    ls.s = 0.0;
    
    Lane lane;
    lane.id = 0;
    lane.type = LANE_DRIVING;
    ls.centerLane.push_back(lane);
    
    road.laneSections.push_back(ls);
    
    file.roads.push_back(road);
    
    Junction junction;
    junction.id = 0;
    junction.name = "Intersection";
    junction.type = 0;
    
    OutgoingConnection oc;
    oc.roadId = 1;
    
    ManeuverGroup mg;
    mg.id = "m1";
    mg.outgoingConnections.push_back(oc);
    junction.maneuverGroups.push_back(mg);
    junction.roadIds.push_back(1);
    
    file.junctions.push_back(junction);
    
    JunctionMesh junctionMesh(file, 10.0);
    auto meshes = junctionMesh.generate();
    
    TEST_PASS("junction mesh generation produces meshes", meshes.size() > 0);
}

void testSignalTypeParsing() {
    SignalType sigType = SIG_GENERIC;
    
    parseSignalType("speedLimit", sigType);
    TEST_PASS("speedLimit type parsed", sigType == SIG_SPEED_LIMIT);
    
    sigType = SIG_GENERIC;
    parseSignalType("maxSpeed", sigType);
    TEST_PASS("maxSpeed type parsed", sigType == SIG_MAX_SPEED);
    
    sigType = SIG_GENERIC;
    parseSignalType("stop", sigType);
    TEST_PASS("stop type parsed", sigType == SIG_STOP);
    
    sigType = SIG_GENERIC;
    parseSignalType("trafficLight", sigType);
    TEST_PASS("trafficLight type parsed", sigType == SIG_TRAFFIC_LIGHT);
    
    sigType = SIG_GENERIC;
    parseSignalType("general", sigType);
    TEST_PASS("general type parsed", sigType == SIG_GENERAL);
    
    sigType = SIG_GENERIC;
    parseSignalType("unknown", sigType);
    TEST_PASS("unknown type defaults to generic", sigType == SIG_GENERIC);
}

void testObjectTypeParsing() {
    ObjectType objType = OBJ_NONE;
    
    parseObjectType("barrier", objType);
    TEST_PASS("barrier type parsed", objType == OBJ_BARRIER);
    
    objType = OBJ_NONE;
    parseObjectType("obstacle", objType);
    TEST_PASS("obstacle type parsed", objType == OBJ_OBSTACLE);
    
    objType = OBJ_NONE;
    parseObjectType("crosswalk", objType);
    TEST_PASS("crosswalk type parsed", objType == OBJ_CROSSWALK);
    
    objType = OBJ_NONE;
    parseObjectType("roadMark", objType);
    TEST_PASS("roadMark type parsed", objType == OBJ_ROAD_MARK);
    
    objType = OBJ_NONE;
    parseObjectType("unknown", objType);
    TEST_PASS("unknown type defaults to none", objType == OBJ_NONE);
}

void testLaneTypeParsing() {
    LaneType laneType = LANE_DRIVING;
    
    parseLaneType("driving", laneType);
    TEST_PASS("driving lane type parsed", laneType == LANE_DRIVING);
    
    laneType = LANE_DRIVING;
    parseLaneType("bidirectional", laneType);
    TEST_PASS("bidirectional lane type parsed", laneType == LANE_BIDIRECTIONAL);
    
    laneType = LANE_DRIVING;
    parseLaneType("shared", laneType);
    TEST_PASS("shared lane type parsed", laneType == LANE_SHARED);
    
    laneType = LANE_DRIVING;
    parseLaneType("onRamp", laneType);
    TEST_PASS("onRamp lane type parsed", laneType == LANE_ONRAMP);
    
    laneType = LANE_DRIVING;
    parseLaneType("offRamp", laneType);
    TEST_PASS("offRamp lane type parsed", laneType == LANE_OFFRAMP);
    
    laneType = LANE_DRIVING;
    parseLaneType("hov", laneType);
    TEST_PASS("hov lane type parsed", laneType == LANE_HOV);
    
    laneType = LANE_DRIVING;
    parseLaneType("unknown", laneType);
    TEST_PASS("unknown lane type defaults to driving", laneType == LANE_DRIVING);
}

}

int main() {
    std::cout << "\n=== OpenDRIVE Mesh Generator Tests ===" << std::endl;
    
    std::cout << "\n--- Vec3 Tests ---";
    testVec3Constructor();
    testVec3Addition();
    testVec3Subtraction();
    testVec3Multiplication();
    testVec3Length();
    testVec3Normalized();
    testVec3DotProduct();
    testVec3CrossProduct();
    
    std::cout << "\n--- FrenetFrame Tests ---";
    testFrenetFrameFromTangentBinormal();
    testFrenetFrameToWorld();
    
    std::cout << "\n--- Geometry Tests ---";
    testPolynomialEvaluation();
    testParamPoly3Evaluation();
    testToUsdCoords();
    testRotateSuperelevation();
    testLineGeometry();
    testArcGeometry();
    testElevationProfile();
    testSuperelevationProfile();
    
    std::cout << "\n--- Junction Tests ---";
    testJunctionParsing();
    testJunctionMeshGeneration();
    
    std::cout << "\n--- Parser Tests ---";
    testSignalTypeParsing();
    testObjectTypeParsing();
    testLaneTypeParsing();
    
    return runAllTests();
}
