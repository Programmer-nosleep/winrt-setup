#include "ui/svg_icon_cache.h"

#include <algorithm>
#include <cctype>
#include <cstdint>

#include <lunasvg.h>

namespace workspace_ui {
namespace {

std::string NormalizeName(std::string_view name)
{
    std::string normalized(name);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c) {
                       return static_cast<char>(std::tolower(c));
                   });
    return normalized;
}

} // namespace

SvgIconCache::~SvgIconCache()
{
    Clear();
}

bool SvgIconCache::Initialize(std::filesystem::path icon_directory,
                              std::string& error)
{
    Clear();
    icon_directory_ = std::move(icon_directory);
    if (!std::filesystem::exists(icon_directory_) ||
        !std::filesystem::is_directory(icon_directory_)) {
        error = "SVG icon directory was not found.";
        return false;
    }

    error.clear();
    return true;
}

void SvgIconCache::Clear()
{
    for (auto const& [_, entry] : textures_) {
        if (entry.texture != 0) {
            glDeleteTextures(1, &entry.texture);
        }
    }
    textures_.clear();
}

IconTexture SvgIconCache::Get(std::string_view icon_name, int pixel_size)
{
    if (icon_name.empty() || pixel_size <= 0 || icon_directory_.empty()) {
        return {};
    }

    const std::string cache_key = MakeCacheKey(icon_name, pixel_size);
    if (auto found = textures_.find(cache_key); found != textures_.end()) {
        return IconTexture{
            static_cast<ImTextureID>(found->second.texture),
            ImVec2(static_cast<float>(found->second.width),
                   static_cast<float>(found->second.height)),
        };
    }

    const std::filesystem::path icon_path = ResolveIconPath(icon_name);
    if (icon_path.empty()) {
        return {};
    }

    TextureEntry const entry = LoadTexture(icon_path, pixel_size);
    if (entry.texture == 0) {
        return {};
    }

    textures_.emplace(cache_key, entry);
    return IconTexture{
        static_cast<ImTextureID>(entry.texture),
        ImVec2(static_cast<float>(entry.width), static_cast<float>(entry.height)),
    };
}

SvgIconCache::TextureEntry SvgIconCache::LoadTexture(
    std::filesystem::path const& icon_path,
    int pixel_size) const
{
    auto document = lunasvg::Document::loadFromFile(icon_path.string());
    if (document == nullptr) {
        return {};
    }

    const auto bitmap = document->renderToBitmap(pixel_size, pixel_size);
    if (bitmap.isNull() || bitmap.width() <= 0 || bitmap.height() <= 0 ||
        bitmap.data() == nullptr) {
        return {};
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    if (texture == 0) {
        return {};
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bitmap.width(), bitmap.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    return TextureEntry{
        texture,
        bitmap.width(),
        bitmap.height(),
    };
}

std::filesystem::path SvgIconCache::ResolveIconPath(
    std::string_view icon_name) const
{
    if (icon_name.empty()) {
        return {};
    }

    const std::string normalized = NormalizeName(icon_name);
    for (auto const& entry : std::filesystem::directory_iterator(icon_directory_)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".svg") {
            continue;
        }
        if (NormalizeName(entry.path().stem().string()) == normalized) {
            return entry.path();
        }
    }

    return {};
}

std::string SvgIconCache::MakeCacheKey(std::string_view icon_name, int pixel_size)
{
    return NormalizeName(icon_name) + "#" + std::to_string(pixel_size);
}

} // namespace workspace_ui
