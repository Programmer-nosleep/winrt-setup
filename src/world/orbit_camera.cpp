#include "world/orbit_camera.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kMinDistance = 5.0f;
constexpr float kMaxDistance = 120.0f;
constexpr float kMinPitch = 0.15f;
constexpr float kMaxPitch = 1.55334306f;

} // namespace

OrbitCamera::OrbitCamera() = default;

void OrbitCamera::Reset()
{
    SetHomeView();
    CancelInteractions();
}

void OrbitCamera::CancelInteractions()
{
    orbiting_ = false;
    panning_ = false;
}

void OrbitCamera::SetHomeView()
{
    CancelInteractions();
    target_ = Vec3{0.0f, 0.0f, 2.0f};
    distance_ = 34.0f;
    yaw_ = Radians(42.0f);
    pitch_ = Radians(36.0f);
}

void OrbitCamera::SetIsometricView()
{
    CancelInteractions();
    target_ = Vec3{0.0f, 0.0f, 2.0f};
    distance_ = 34.0f;
    yaw_ = Radians(45.0f);
    pitch_ = Radians(35.264f);
}

void OrbitCamera::SetFrontView()
{
    CancelInteractions();
    target_ = Vec3{0.0f, 0.0f, 2.0f};
    distance_ = 34.0f;
    yaw_ = Radians(-90.0f);
    pitch_ = kMinPitch;
}

void OrbitCamera::SetBackView()
{
    CancelInteractions();
    target_ = Vec3{0.0f, 0.0f, 2.0f};
    distance_ = 34.0f;
    yaw_ = Radians(90.0f);
    pitch_ = kMinPitch;
}

void OrbitCamera::SetLeftView()
{
    CancelInteractions();
    target_ = Vec3{0.0f, 0.0f, 2.0f};
    distance_ = 34.0f;
    yaw_ = Radians(180.0f);
    pitch_ = kMinPitch;
}

void OrbitCamera::SetRightView()
{
    CancelInteractions();
    target_ = Vec3{0.0f, 0.0f, 2.0f};
    distance_ = 34.0f;
    yaw_ = 0.0f;
    pitch_ = kMinPitch;
}

void OrbitCamera::SetTopView()
{
    CancelInteractions();
    target_ = Vec3{0.0f, 0.0f, 2.0f};
    yaw_ = Radians(45.0f);
    pitch_ = Radians(89.0f);
}

void OrbitCamera::HandleEvent(SDL_Event const& event)
{
    switch (event.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event.button.button == SDL_BUTTON_RIGHT) {
            orbiting_ = true;
        }
        if (event.button.button == SDL_BUTTON_MIDDLE) {
            panning_ = true;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event.button.button == SDL_BUTTON_RIGHT) {
            orbiting_ = false;
        }
        if (event.button.button == SDL_BUTTON_MIDDLE) {
            panning_ = false;
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (orbiting_) {
            yaw_ -= event.motion.xrel * 0.01f;
            pitch_ = std::clamp(pitch_ + event.motion.yrel * 0.01f, kMinPitch, kMaxPitch);
        }
        if (panning_) {
            const float pan_scale = distance_ * 0.0025f;
            target_ = target_ - CameraRight() * (event.motion.xrel * pan_scale);
            target_ = target_ + CameraUp() * (event.motion.yrel * pan_scale);
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        distance_ = std::clamp(
            distance_ * std::exp(-event.wheel.y * 0.08f),
            kMinDistance,
            kMaxDistance
        );
        break;
    default:
        break;
    }
}

Vec3 OrbitCamera::Position() const
{
    const float cos_pitch = std::cos(pitch_);
    return Vec3{
        target_.x + distance_ * cos_pitch * std::cos(yaw_),
        target_.y + distance_ * cos_pitch * std::sin(yaw_),
        target_.z + distance_ * std::sin(pitch_),
    };
}

Vec3 OrbitCamera::Target() const
{
    return target_;
}

Mat4 OrbitCamera::ViewMatrix() const
{
    return LookAtMatrix(Position(), target_, Vec3{0.0f, 0.0f, 1.0f});
}

Mat4 OrbitCamera::ProjectionMatrix(float aspect_ratio) const
{
    return PerspectiveMatrix(Radians(48.0f), aspect_ratio, 0.1f, 2400.0f);
}

float OrbitCamera::Distance() const
{
    return distance_;
}

float OrbitCamera::YawDegrees() const
{
    return yaw_ * (180.0f / 3.1415926535f);
}

float OrbitCamera::PitchDegrees() const
{
    return pitch_ * (180.0f / 3.1415926535f);
}

bool OrbitCamera::IsInteracting() const
{
    return orbiting_ || panning_;
}

Vec3 OrbitCamera::CameraRight() const
{
    const Vec3 forward = Normalize(target_ - Position());
    const Vec3 right = Normalize(Cross(forward, Vec3{0.0f, 0.0f, 1.0f}));
    return right.x == 0.0f && right.y == 0.0f && right.z == 0.0f ? Vec3{1.0f, 0.0f, 0.0f} : right;
}

Vec3 OrbitCamera::CameraUp() const
{
    return Normalize(Cross(CameraRight(), Normalize(target_ - Position())));
}
