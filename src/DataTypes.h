#pragma once
#include <Kokkos_Core.hpp>
#include <cmath> // for sqrt, etc.

struct Vec3
{
  double x, y, z;

  KOKKOS_INLINE_FUNCTION
  Vec3() : x(0.0), y(0.0), z(0.0) {}

  KOKKOS_INLINE_FUNCTION
  Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

  // Allow construction from array (e.g., for initialization)
  KOKKOS_INLINE_FUNCTION
  explicit Vec3(const double v[3]) : x(v[0]), y(v[1]), z(v[2]) {}

  // Access via index: v[0], v[1], v[2]
  KOKKOS_INLINE_FUNCTION
  double operator[](int i) const
  {
    return (&x)[i]; // Safe: x,y,z are contiguous in memory
  }

  KOKKOS_INLINE_FUNCTION
  double &operator[](int i)
  {
    return (&x)[i];
  }

  // Assignment
  KOKKOS_INLINE_FUNCTION
  Vec3 &operator=(const Vec3 &other) = default;

  // Unary minus
  KOKKOS_INLINE_FUNCTION
  Vec3 operator-() const
  {
    return Vec3(-x, -y, -z);
  }

  // Addition
  KOKKOS_INLINE_FUNCTION
  Vec3 operator+(const Vec3 &other) const
  {
    return Vec3(x + other.x, y + other.y, z + other.z);
  }

  KOKKOS_INLINE_FUNCTION
  Vec3 &operator+=(const Vec3 &other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  // Subtraction
  KOKKOS_INLINE_FUNCTION
  Vec3 operator-(const Vec3 &other) const
  {
    return Vec3(x - other.x, y - other.y, z - other.z);
  }

  KOKKOS_INLINE_FUNCTION
  Vec3 &operator-=(const Vec3 &other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  // Scalar multiplication
  KOKKOS_INLINE_FUNCTION
  Vec3 operator*(double s) const
  {
    return Vec3(x * s, y * s, z * s);
  }

  KOKKOS_INLINE_FUNCTION
  Vec3 &operator*=(double s)
  {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }

  // Scalar division
  KOKKOS_INLINE_FUNCTION
  Vec3 operator/(double s) const
  {
    double inv_s = 1.0 / s;
    return Vec3(x * inv_s, y * inv_s, z * inv_s);
  }

  KOKKOS_INLINE_FUNCTION
  Vec3 &operator/=(double s)
  {
    double inv_s = 1.0 / s;
    x *= inv_s;
    y *= inv_s;
    z *= inv_s;
    return *this;
  }

  // Length (magnitude)
  KOKKOS_INLINE_FUNCTION
  double length() const
  {
    return std::sqrt(x * x + y * y + z * z);
  }

  // Squared length (faster, avoids sqrt)
  KOKKOS_INLINE_FUNCTION
  double length2() const
  {
    return x * x + y * y + z * z;
  }

  // Normalize (return unit vector)
  KOKKOS_INLINE_FUNCTION
  Vec3 normalize() const
  {
    double l = length();
    return l > 1e-16 ? (*this) * (1.0 / l) : Vec3(0.0, 0.0, 0.0);
  }

  // Safe normalize with zero check
  KOKKOS_INLINE_FUNCTION
  Vec3 safe_normalize(const Vec3 &default_vec = Vec3(1.0, 0.0, 0.0)) const
  {
    double l2 = length2();
    if (l2 > 1e-16)
    {
      return (*this) * (1.0 / std::sqrt(l2));
    }
    return default_vec;
  }
};

// Non-member operator overloads (symmetric scalar multiplication)

KOKKOS_INLINE_FUNCTION
Vec3 operator*(double s, const Vec3 &v)
{
  return Vec3(s * v.x, s * v.y, s * v.z);
}

// Dot product
KOKKOS_INLINE_FUNCTION
double dot(const Vec3 &a, const Vec3 &b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Cross product
KOKKOS_INLINE_FUNCTION
Vec3 cross(const Vec3 &a, const Vec3 &b)
{
  return Vec3(
      a.y * b.z - a.z * b.y,
      a.z * b.x - a.x * b.z,
      a.x * b.y - a.y * b.x);
}

// Distance between two points
KOKKOS_INLINE_FUNCTION
double distance(const Vec3 &a, const Vec3 &b)
{
  return (b - a).length();
}

// Squared distance
KOKKOS_INLINE_FUNCTION
double distance2(const Vec3 &a, const Vec3 &b)
{
  Vec3 d = b - a;
  return d.x * d.x + d.y * d.y + d.z * d.z;
}

// Linear interpolation (lerp)
KOKKOS_INLINE_FUNCTION
Vec3 lerp(const Vec3 &a, const Vec3 &b, double t)
{
  return a + t * (b - a);
}

// Make Vec3 trivially copyable (important for Kokkos)
static_assert(std::is_trivially_copyable<Vec3>::value, "Vec3 must be trivially copyable");

KOKKOS_INLINE_FUNCTION
static void atomic_add(Vec3 *addr, const Vec3 &val)
{
  Kokkos::atomic_add(&addr->x, val.x);
  Kokkos::atomic_add(&addr->y, val.y);
  Kokkos::atomic_add(&addr->z, val.z);
}

KOKKOS_INLINE_FUNCTION
static void atomic_sub(Vec3 *addr, const Vec3 &val)
{
  Kokkos::atomic_add(&addr->x, -val.x);
  Kokkos::atomic_add(&addr->y, -val.y);
  Kokkos::atomic_add(&addr->z, -val.z);
}
