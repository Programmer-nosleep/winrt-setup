#pragma once

#include <SDL3/SDL.h>

namespace InputConstants {

constexpr SDL_Keycode kOpenHelp = SDLK_F1;
constexpr SDL_Keycode kRefreshSystemInfo = SDLK_F5;
constexpr SDL_Keycode kToggleInspector = SDLK_TAB;
constexpr SDL_Keycode kResetCamera = SDLK_HOME;
constexpr SDL_Keycode kFrontView = SDLK_1;
constexpr SDL_Keycode kRightView = SDLK_3;
constexpr SDL_Keycode kIsometricView = SDLK_5;
constexpr SDL_Keycode kTopView = SDLK_7;
constexpr SDL_Keycode kToggleGrid = SDLK_G;
constexpr SDL_Keycode kToggleAxes = SDLK_A;
constexpr SDL_Keycode kToggleGuide = SDLK_B;

constexpr SDL_Keymod kSceneToggleModifier = SDL_KMOD_CTRL;

constexpr char kOpenHelpLabel[] = "F1";
constexpr char kRefreshSystemInfoLabel[] = "F5";
constexpr char kToggleInspectorLabel[] = "Tab";
constexpr char kResetCameraLabel[] = "Home";
constexpr char kFrontViewLabel[] = "1";
constexpr char kRightViewLabel[] = "3";
constexpr char kIsometricViewLabel[] = "5";
constexpr char kTopViewLabel[] = "7";
constexpr char kToggleGridLabel[] = "Ctrl+G";
constexpr char kToggleAxesLabel[] = "Ctrl+A";
constexpr char kToggleGuideLabel[] = "Ctrl+B";

inline bool HasModifier(SDL_Keymod current, SDL_Keymod expected)
{
    return (current & expected) == expected;
}

} // namespace InputConstants
