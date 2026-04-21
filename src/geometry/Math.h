#pragma once
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>

struct Vec3 {
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
    
    double length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const {
        double len = length();
        if (len < 1e-10) return {0, 0, 0};
        return {x/len, y/len, z/len};
    }
    
    double dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x};
    }
};

struct FrenetFrame {
    Vec3 tangent;    // S direction (along road, in XY plane)
    Vec3 normal;     // T direction (lateral, perpendicular to tangent in XY plane)
    Vec3 binormal;   // N direction (up, perpendicular to XY plane)
    Vec3 position;   // Road centerline position
    double s;
    
    static FrenetFrame fromTangentBinormal(Vec3 tangent, Vec3 binormal) {
        FrenetFrame frame;
        frame.tangent = tangent.normalized();
        frame.binormal = binormal.normalized();
        frame.normal = frame.binormal.cross(frame.tangent).normalized();
        frame.position = {0, 0, 0};
        frame.s = 0;
        return frame;
    }
    
    Vec3 toWorld(const Vec3& local) const {
        return tangent * local.x + normal * local.y + binormal * local.z;
    }
    
    Vec3 getPoint(double sOffset, double tOffset, double zOffset) const {
        Vec3 offset = tangent * sOffset + normal * tOffset + binormal * zOffset;
        return position + offset;
    }
};

// Evaluate polynomial: a + b*s + c*s^2 + d*s^3
double evalPolynomial(double a, double b, double c, double d, double s);

// Get point from paramPoly3 geometry
Vec3 evalParamPoly3(double aU, double aV, double bU, double bV,
                   double cU, double cV, double dU, double dV,
                   double s, bool normalized, double length);

// Compute Frenet frame from consecutive points
FrenetFrame computeFrenetFrame(const Vec3& p0, const Vec3& p1, const Vec3& p2);

// Transform OpenDRIVE coords to USD coords
// OpenDRIVE: X-right, Y-forward, Z-up
// USD: X-right, Y-forward, Z-up (conventional Y-up)
// We use: X->X, Y->-Z, Z->Y (rotate -90 deg around X)
Vec3 toUsdCoords(const Vec3& od);

// Create rotation matrix for superelevation
Vec3 rotateSuperelevation(const Vec3& local, double bankAngle);
