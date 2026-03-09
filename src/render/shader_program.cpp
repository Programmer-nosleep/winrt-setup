#include "render/shader_program.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <fmt/format.h>

namespace {

std::string ReadInfoLog(GlApi const& gl, GLuint object, bool is_program)
{
    GLint length = 0;
    if (is_program) {
        gl.GetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
    } else {
        gl.GetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
    }

    if (length <= 1) {
        return {};
    }

    std::vector<GLchar> buffer(static_cast<std::size_t>(length), 0);
    if (is_program) {
        gl.GetProgramInfoLog(object, length, nullptr, buffer.data());
    } else {
        gl.GetShaderInfoLog(object, length, nullptr, buffer.data());
    }

    return std::string(buffer.data());
}

} // namespace

bool ShaderProgram::LoadFromFiles(
    GlApi const& gl,
    std::filesystem::path const& vertex_shader_path,
    std::filesystem::path const& fragment_shader_path,
    std::string& error
)
{
    Destroy(gl);

    const std::string vertex_source = ReadTextFile(vertex_shader_path);
    const std::string fragment_source = ReadTextFile(fragment_shader_path);

    const GLuint vertex_shader = gl.CreateShader(GL_VERTEX_SHADER);
    const GLuint fragment_shader = gl.CreateShader(GL_FRAGMENT_SHADER);
    if (vertex_shader == 0 || fragment_shader == 0) {
        error = "OpenGL gagal membuat object shader.";
        return false;
    }

    if (!CompileShader(gl, vertex_shader, vertex_source, error) ||
        !CompileShader(gl, fragment_shader, fragment_source, error)) {
        gl.DeleteShader(vertex_shader);
        gl.DeleteShader(fragment_shader);
        return false;
    }

    program_ = gl.CreateProgram();
    gl.AttachShader(program_, vertex_shader);
    gl.AttachShader(program_, fragment_shader);
    gl.LinkProgram(program_);

    GLint linked = GL_FALSE;
    gl.GetProgramiv(program_, GL_LINK_STATUS, &linked);
    gl.DeleteShader(vertex_shader);
    gl.DeleteShader(fragment_shader);

    if (linked != GL_TRUE) {
        error = fmt::format("Link shader gagal:\n{}", ReadInfoLog(gl, program_, true));
        gl.DeleteProgram(program_);
        program_ = 0;
        return false;
    }

    view_projection_location_ = gl.GetUniformLocation(program_, "u_view_projection");
    if (view_projection_location_ < 0) {
        error = "Uniform u_view_projection tidak ditemukan di shader.";
        gl.DeleteProgram(program_);
        program_ = 0;
        return false;
    }

    return true;
}

void ShaderProgram::Destroy(GlApi const& gl)
{
    if (program_ != 0) {
        gl.DeleteProgram(program_);
        program_ = 0;
        view_projection_location_ = -1;
    }
}

GLuint ShaderProgram::Handle() const
{
    return program_;
}

GLint ShaderProgram::ViewProjectionLocation() const
{
    return view_projection_location_;
}

bool ShaderProgram::CompileShader(
    GlApi const& gl,
    GLuint shader,
    std::string const& source,
    std::string& error
)
{
    const GLchar* text = source.c_str();
    const GLint length = static_cast<GLint>(source.size());
    gl.ShaderSource(shader, 1, &text, &length);
    gl.CompileShader(shader);

    GLint compiled = GL_FALSE;
    gl.GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_TRUE) {
        return true;
    }

    error = fmt::format("Compile shader gagal:\n{}", ReadInfoLog(gl, shader, false));
    return false;
}

std::string ShaderProgram::ReadTextFile(std::filesystem::path const& path)
{
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    if (!stream.is_open()) {
        throw std::runtime_error(fmt::format("Gagal membuka file shader: {}", path.string()));
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}
