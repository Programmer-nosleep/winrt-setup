#pragma once

#include <array>
#include <cmath>

struct Vec3 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

inline Vec3 operator+(Vec3 const &a, Vec3 const &b) {
  return Vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vec3 operator-(Vec3 const &a, Vec3 const &b) {
  return Vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vec3 operator*(Vec3 const &value, float scalar) {
  return Vec3{value.x * scalar, value.y * scalar, value.z * scalar};
}

inline Vec3 operator/(Vec3 const &value, float scalar) {
  return Vec3{value.x / scalar, value.y / scalar, value.z / scalar};
}

inline float Dot(Vec3 const &a, Vec3 const &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 Cross(Vec3 const &a, Vec3 const &b) {
  return Vec3{
      a.y * b.z - a.z * b.y,
      a.z * b.x - a.x * b.z,
      a.x * b.y - a.y * b.x,
  };
}

inline float Length(Vec3 const &value) { return std::sqrt(Dot(value, value)); }

inline Vec3 Normalize(Vec3 const &value) {
  const float length = Length(value);
  if (length <= 0.0001f) {
    return Vec3{};
  }
  return value / length;
}

inline float Radians(float degrees) {
  constexpr float kPi = 3.1415926535f;
  return degrees * (kPi / 180.0f);
}

struct Mat4 {
  std::array<float, 16> values{};

  [[nodiscard]] const float *Data() const { return values.data(); }
};

inline Mat4 IdentityMatrix() {
  Mat4 result{};
  result.values[0] = 1.0f;
  result.values[5] = 1.0f;
  result.values[10] = 1.0f;
  result.values[15] = 1.0f;
  return result;
}

inline Mat4 Multiply(Mat4 const &left, Mat4 const &right) {
  Mat4 result{};
  for (int column = 0; column < 4; ++column) {
    for (int row = 0; row < 4; ++row) {
      float value = 0.0f;
      for (int index = 0; index < 4; ++index) {
        value +=
            left.values[index * 4 + row] * right.values[column * 4 + index];
      }
      result.values[column * 4 + row] = value;
    }
  }
  return result;
}

inline Mat4 PerspectiveMatrix(float fov_y_radians, float aspect_ratio,
                              float near_plane, float far_plane) {
  Mat4 result{};
  const float focal_length = 1.0f / std::tan(fov_y_radians * 0.5f);
  result.values[0] = focal_length / aspect_ratio;
  result.values[5] = focal_length;
  result.values[10] = (far_plane + near_plane) / (near_plane - far_plane);
  result.values[11] = -1.0f;
  result.values[14] =
      (2.0f * far_plane * near_plane) / (near_plane - far_plane);
  return result;
}

inline Mat4 LookAtMatrix(Vec3 const &eye, Vec3 const &target,
                         Vec3 const &world_up) {
  const Vec3 forward = Normalize(target - eye);
  const Vec3 right = Normalize(Cross(forward, world_up));
  const Vec3 up = Cross(right, forward);

  Mat4 result = IdentityMatrix();
  result.values[0] = right.x;
  result.values[1] = up.x;
  result.values[2] = -forward.x;
  result.values[4] = right.y;
  result.values[5] = up.y;
  result.values[6] = -forward.y;
  result.values[8] = right.z;
  result.values[9] = up.z;
  result.values[10] = -forward.z;
  result.values[12] = -Dot(right, eye);
  result.values[13] = -Dot(up, eye);
  result.values[14] = Dot(forward, eye);
  return result;
}
