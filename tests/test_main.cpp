#include "test.h"
#include <cmath>
#include <iostream>
#include <fstream>

#include "parser/OpenDRIVEParser.h"
#include "geometry/Math.h"
#include "geometry/RoadGeometry.h"
#include "geometry/LaneMesh.h"

namespace {

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

}

int main() {
    std::cout << "\n=== OpenDRIVE Mesh Generator Tests ===" << std::endl;
    
    // Math tests
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
    
    return runAllTests();
}
