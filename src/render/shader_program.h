#pragma once

#include <filesystem>
#include <string>

#include <SDL3/SDL_opengl.h>

#include "render/gl_api.h"

class ShaderProgram {
public:
    bool LoadFromFiles(
        GlApi const& gl,
        std::filesystem::path const& vertex_shader_path,
        std::filesystem::path const& fragment_shader_path,
        std::string& error
    );

    void Destroy(GlApi const& gl);

    [[nodiscard]] GLuint Handle() const;
    [[nodiscard]] GLint ViewProjectionLocation() const;

private:
    static bool CompileShader(
        GlApi const& gl,
        GLuint shader,
        std::string const& source,
        std::string& error
    );

    static std::string ReadTextFile(std::filesystem::path const& path);

    GLuint program_ = 0;
    GLint view_projection_location_ = -1;
};
