#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include <SDL3/SDL_opengl.h>
#include <imgui.h>

namespace workspace_ui {

struct IconTexture {
    ImTextureID texture_id = 0;
    ImVec2 size{};

    [[nodiscard]] bool IsValid() const { return texture_id != 0; }
};

class SvgIconCache {
public:
    SvgIconCache() = default;
    SvgIconCache(SvgIconCache const&) = delete;
    SvgIconCache& operator=(SvgIconCache const&) = delete;
    ~SvgIconCache();

    bool Initialize(std::filesystem::path icon_directory, std::string& error);
    void Clear();

    [[nodiscard]] IconTexture Get(std::string_view icon_name, int pixel_size);

private:
    struct TextureEntry {
        GLuint texture = 0;
        int width = 0;
        int height = 0;
    };

    [[nodiscard]] TextureEntry LoadTexture(std::filesystem::path const& icon_path,
                                           int pixel_size) const;
    [[nodiscard]] std::filesystem::path ResolveIconPath(
        std::string_view icon_name) const;
    [[nodiscard]] static std::string MakeCacheKey(std::string_view icon_name,
                                                  int pixel_size);

    std::filesystem::path icon_directory_{};
    std::unordered_map<std::string, TextureEntry> textures_{};
};

} // namespace workspace_ui
