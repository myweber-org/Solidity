#include <iostream>
#include <cmath>
#include <array>

struct Quaternion {
    double w, x, y, z;
    
    Quaternion(double w, double x, double y, double z) : w(w), x(x), y(y), z(z) {}
    
    Quaternion normalize() const {
        double norm = std::sqrt(w*w + x*x + y*y + z*z);
        return Quaternion(w/norm, x/norm, y/norm, z/norm);
    }
    
    Quaternion conjugate() const {
        return Quaternion(w, -x, -y, -z);
    }
    
    Quaternion operator*(const Quaternion& q) const {
        return Quaternion(
            w*q.w - x*q.x - y*q.y - z*q.z,
            w*q.x + x*q.w + y*q.z - z*q.y,
            w*q.y - x*q.z + y*q.w + z*q.x,
            w*q.z + x*q.y - y*q.x + z*q.w
        );
    }
};

struct Vector3 {
    double x, y, z;
    
    Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
};

Vector3 rotateVector(const Vector3& v, const Quaternion& q) {
    Quaternion p(0, v.x, v.y, v.z);
    Quaternion q_conj = q.conjugate();
    Quaternion rotated = q * p * q_conj;
    
    return Vector3(rotated.x, rotated.y, rotated.z);
}

Quaternion fromAxisAngle(double angle, const Vector3& axis) {
    double halfAngle = angle * 0.5;
    double s = std::sin(halfAngle);
    return Quaternion(std::cos(halfAngle), axis.x*s, axis.y*s, axis.z*s).normalize();
}

int main() {
    Vector3 original(1.0, 0.0, 0.0);
    Vector3 axis(0.0, 1.0, 0.0);
    double angle = M_PI / 2.0;
    
    Quaternion rotation = fromAxisAngle(angle, axis);
    Vector3 rotated = rotateVector(original, rotation);
    
    std::cout << "Original: (" << original.x << ", " << original.y << ", " << original.z << ")\n";
    std::cout << "Rotated: (" << rotated.x << ", " << rotated.y << ", " << rotated.z << ")\n";
    
    return 0;
}