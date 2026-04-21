#include "Math.h"
#include <cmath>
#include <algorithm>

double evalPolynomial(double a, double b, double c, double d, double s) {
    double s2 = s * s;
    double s3 = s2 * s;
    return a + b * s + c * s2 + d * s3;
}

Vec3 evalParamPoly3(double aU, double aV, double bU, double bV,
                   double cU, double cV, double dU, double dV,
                   double s, bool normalized, double length) {
    double p = s / length;
    double p2 = p * p;
    double p3 = p2 * p;
    double x = aU + bU * p + cU * p2 + dU * p3;
    double y = aV + bV * p + cV * p2 + dV * p3;
    return {x, y, 0};
}

FrenetFrame computeFrenetFrame(const Vec3& p0, const Vec3& p1, const Vec3& p2) {
    Vec3 tangent = (p1 - p0).normalized();
    Vec3 binormal = {0, 0, 1};
    return FrenetFrame::fromTangentBinormal(tangent, binormal);
}

Vec3 toUsdCoords(const Vec3& od) {
    return {od.x, od.z, -od.y};
}

Vec3 rotateSuperelevation(const Vec3& local, double bankAngle) {
    double cosA = std::cos(bankAngle);
    double sinA = std::sin(bankAngle);
    Vec3 result;
    result.x = local.x;
    result.y = local.y * cosA + local.z * sinA;
    result.z = -local.y * sinA + local.z * cosA;
    return result;
}
