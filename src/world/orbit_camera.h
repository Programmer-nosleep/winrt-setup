#pragma once

#include <SDL3/SDL.h>

#include "utils/math3d.h"

class OrbitCamera {
public:
    OrbitCamera();

    void Reset();
    void CancelInteractions();
    void HandleEvent(SDL_Event const& event);
    void SetHomeView();
    void SetIsometricView();
    void SetFrontView();
    void SetBackView();
    void SetLeftView();
    void SetRightView();
    void SetTopView();

    [[nodiscard]] Vec3 Position() const;
    [[nodiscard]] Vec3 Target() const;
    [[nodiscard]] Mat4 ViewMatrix() const;
    [[nodiscard]] Mat4 ProjectionMatrix(float aspect_ratio) const;
    [[nodiscard]] float Distance() const;
    [[nodiscard]] float YawDegrees() const;
    [[nodiscard]] float PitchDegrees() const;
    [[nodiscard]] bool IsInteracting() const;

private:
    Vec3 CameraRight() const;
    Vec3 CameraUp() const;

    Vec3 target_{0.0f, 0.0f, 2.0f};
    float distance_ = 34.0f;
    float yaw_ = Radians(42.0f);
    float pitch_ = Radians(36.0f);
    bool orbiting_ = false;
    bool panning_ = false;
};
