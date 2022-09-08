#pragma once

#ifndef CGL_PRINT_EXCEPTION
    #define CGL_PRINT_EXCEPTION(text) { std::cout << text << std::endl; throw std::runtime_error{ text }; }
#endif

#ifndef CGL_ASSERT
    #define CGL_ASSERT(expr) if(!expr) std::cout << "Failed assertion for '" << #expr << "' @ line " << __LINE__ << " file " << __FILE__ << std::endl;
#endif

#include <glad/glad.h>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <array>
#include <tuple>
#include <stb_image.h>

struct texture
{
    GLenum target;
    GLuint handle;

    texture(GLenum target)
        : target(target)
    {
        glCreateTextures(target, 1, &handle);
    }

    void params(std::unordered_map<GLenum, GLint> parameters)
    {
        for (const auto& [k, v] : parameters)
        {
            glTextureParameteri(handle, k, v);
        }
    }

    void storage(GLsizei levels, GLenum internal_format, GLsizei width, GLsizei height, GLsizei depth)
    {
        switch (target)
        {
        case GL_TEXTURE_1D:
            glTextureStorage1D(handle, levels, internal_format, width);
            break;
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
            glTextureStorage2D(handle, levels, internal_format, width, height);
            break;
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glTextureStorage3D(handle, levels, internal_format, width, height, depth);
            break;
        default:
            CGL_PRINT_EXCEPTION("unimplemented");
        }
    }

    void sub_image(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
    {
        switch (target)
        {
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
            glTextureSubImage2D(handle, level, xoffset, yoffset, width, height, format, type, pixels);
            break;
        default:
            CGL_PRINT_EXCEPTION("unimplemented");
        }
    }


    void bind_image_texture(GLuint unit, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
    {
        glBindImageTexture(unit, handle, level, layered, layer, access, format);
    }

    void bind()
    {
        glBindTexture(target, handle);
    }

    void unbind()
    {
        glBindTexture(target, 0);
    }
};

struct shader_program
{
    GLuint program;

    shader_program()
    {
        program = glCreateProgram();
    }

    void load_shader(GLenum shader_type, const std::string& path)
    {
        std::stringstream src_builder;

        std::ifstream ifs(path, std::ios::binary);
        if (ifs.is_open())
        {
            std::string line;
            while (std::getline(ifs, line))
            {
                src_builder << line << std::endl;
            }
            ifs.close();
        }
        else
        {
            CGL_PRINT_EXCEPTION("shader not found: " + path);
        }

        std::string src = src_builder.str();
        const char* c_src = src.c_str();

        GLuint shader = glCreateShader(shader_type);
        glShaderSource(shader, 1, &c_src, NULL);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint log_len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

            if (log_len > 0)
            {
                GLchar* log = new GLchar[log_len];
                glGetShaderInfoLog(shader, log_len, NULL, log);
                std::cout << "shader failure @ " << path << std::endl;
                std::cout << log << std::endl;
                delete[] log;
            }

            glDeleteShader(shader);
            throw std::runtime_error{ "bad shader: " + path };
        }

        glAttachShader(program, shader);
        glDeleteShader(shader);
    }

    void link()
    {
        glLinkProgram(program);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint log_len;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

            if (log_len > 0)
            {
                GLchar* log = new GLchar[log_len];
                glGetProgramInfoLog(program, log_len, NULL, log);
                std::cout << "shader program link failure" << std::endl;
                std::cout << log << std::endl;
                delete[] log;
            }

            throw std::runtime_error{ "bad program" };
        }
    }

    void bind()
    {
        glUseProgram(program);
    }

    void unbind()
    {
        glUseProgram(0);
    }

    void destroy()
    {
        glDeleteProgram(program);
    }
};

struct buffer_object
{
    GLuint handle;

    buffer_object()
    {
        glGenVertexArrays(1, &handle);
    }

    void bind()
    {
        glBindVertexArray(handle);
    }

    void unbind()
    {
        glBindVertexArray(0);
    }

    buffer_object& operator=(const buffer_object&) = delete;
    buffer_object(const buffer_object&) = delete;

    ~buffer_object()
    {
        glDeleteVertexArrays(1, &handle);
    }
};

struct buffer
{
    GLuint handle;

    template<size_t N>
    buffer(const std::array<GLfloat, N>& data, int index, int size)
    {
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    buffer(const std::vector<GLfloat>& data, int index, int size)
    {
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    template<size_t N>
    buffer(const std::array<GLuint, N>& indices)
    {
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
    }

    buffer(const std::vector<GLuint>& indices)
    {
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
    }

    buffer& operator=(const buffer&) = delete;
    buffer(const buffer&) = delete;

    ~buffer()
    {
        glDeleteBuffers(1, &handle);
    }
};

struct pixel_grid
{
    std::vector<float> data;

    pixel_grid(int width, int height, int color_size)
        : width(width), height(height), color_size(color_size)
    {
        data.resize(static_cast<size_t>(width * height * color_size));
    }

    pixel_grid(const std::string& path)
    {
        auto* img_data = stbi_load("input.png", &width, &height, &color_size, STBI_rgb_alpha);

        if (!img_data)
        {
            CGL_PRINT_EXCEPTION("no image data found");
        }

        data.resize(static_cast<size_t>(width * height * color_size));
        for (int i = 0; i < width * height * color_size; i++)
        {
            data[i] = static_cast<float>(img_data[i]) / 255.0f;
        }
        stbi_image_free(img_data);
    }

    pixel_grid(const pixel_grid&) = delete;
    pixel_grid& operator=(const pixel_grid&) = delete;

    void set(int x, int y, float r, float g, float b, float a)
    {
        size_t i = (x * color_size) + (y * color_size * width);
        data[i + 0u] = r;
        data[i + 1u] = g;
        data[i + 2u] = b;
        data[i + 3u] = a;
    }

    std::tuple<float, float, float, float> get(int x, int y)
    {
        size_t i = (x * color_size) + (y * color_size * width);
        return { data[i + 0u], data[i + 1u], data[i + 2u], data[i + 3u] };
    }

    int width;
    int height;
    int color_size;
};