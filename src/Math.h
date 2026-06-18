#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return Vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
    }
    
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalized() const {
        float l = length();
        return l > 1e-8f ? (*this) / l : Vec3(0, 0, 0);
    }
};

inline Vec3 operator*(float s, const Vec3& v) { return v * s; }

struct Mat3 {
    float m[3][3]; // m[row][col]

    Mat3() {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = 0;
    }

    static Mat3 identity() {
        Mat3 res;
        res.m[0][0] = 1.0f; res.m[1][1] = 1.0f; res.m[2][2] = 1.0f;
        return res;
    }

    static Mat3 outer_product(const Vec3& a, const Vec3& b) {
        Mat3 res;
        res.m[0][0] = a.x * b.x; res.m[0][1] = a.x * b.y; res.m[0][2] = a.x * b.z;
        res.m[1][0] = a.y * b.x; res.m[1][1] = a.y * b.y; res.m[1][2] = a.y * b.z;
        res.m[2][0] = a.z * b.x; res.m[2][1] = a.z * b.y; res.m[2][2] = a.z * b.z;
        return res;
    }

    Mat3 operator+(const Mat3& o) const {
        Mat3 res;
        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                res.m[i][j] = m[i][j] + o.m[i][j];
        return res;
    }
    
    Mat3 operator-(const Mat3& o) const {
        Mat3 res;
        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                res.m[i][j] = m[i][j] - o.m[i][j];
        return res;
    }

    Mat3 operator*(float s) const {
        Mat3 res;
        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                res.m[i][j] = m[i][j] * s;
        return res;
    }

    Mat3 operator*(const Mat3& o) const {
        Mat3 res;
        for(int i=0; i<3; ++i) {
            for(int j=0; j<3; ++j) {
                res.m[i][j] = m[i][0] * o.m[0][j] + m[i][1] * o.m[1][j] + m[i][2] * o.m[2][j];
            }
        }
        return res;
    }

    Vec3 operator*(const Vec3& v) const {
        return Vec3(
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        );
    }

    Mat3 transpose() const {
        Mat3 res;
        for(int i=0; i<3; ++i)
            for(int j=0; j<3; ++j)
                res.m[i][j] = m[j][i];
        return res;
    }

    float determinant() const {
        return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
             - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
             + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    }
    
    float trace() const {
        return m[0][0] + m[1][1] + m[2][2];
    }
};

inline Mat3 operator*(float s, const Mat3& m) { return m * s; }
