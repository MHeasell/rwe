#include "GraphicsContext.h"
#include <rwe/rwe_string.h>

#include <GL/glew.h>

namespace rwe
{
    void requireNoOpenGlError()
    {
        auto error = glGetError();
        if (error != GL_NO_ERROR)
        {
            throw OpenGlException(error);
        }
    }

    GlTexturedVertex::GlTexturedVertex(const Vector3f& pos, const Vector2f& texCoord)
        : x(pos.x), y(pos.y), z(pos.z), u(texCoord.x), v(texCoord.y)
    {
    }

    GlTextureArrayVertex::GlTextureArrayVertex(const Vector3f& pos, const Vector3f& texCoord)
        : x(pos.x), y(pos.y), z(pos.z), u(texCoord.x), v(texCoord.y), w(texCoord.z)
    {
    }

    GlTexturedNormalVertex::GlTexturedNormalVertex(const Vector3f& pos, const Vector2f& texCoord, const Vector3f& normal)
        : x(pos.x), y(pos.y), z(pos.z), u(texCoord.x), v(texCoord.y), nx(normal.x), ny(normal.y), nz(normal.z)
    {
    }

    GlColoredVertex::GlColoredVertex(const Vector3f& pos, const Vector3f& color)
        : x(pos.x), y(pos.y), z(pos.z), r(color.x), g(color.y), b(color.z)
    {
    }

    GlColoredNormalVertex::GlColoredNormalVertex(const Vector3f& pos, const Vector3f& color, const Vector3f& normal)
        : x(pos.x), y(pos.y), z(pos.z), r(color.x), g(color.y), b(color.z), nx(normal.x), ny(normal.y), nz(normal.z)
    {
    }

    AttribMapping::AttribMapping(const std::string& name, GLuint location) : name(name), location(location)
    {
    }

    GraphicsException::GraphicsException(const std::string& __arg) : runtime_error(__arg) {}

    GraphicsException::GraphicsException(const char* string) : runtime_error(string) {}

    OpenGlException::OpenGlException(GLenum error) : GraphicsException(std::string("OpenGL error: ") + std::to_string(error))
    {
    }

    void GraphicsContext::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    TextureHandle GraphicsContext::createTexture(const Grid<Color>& image)
    {
        return createTexture(image.getWidth(), image.getHeight(), image.getData());
    }

    TextureHandle
    GraphicsContext::createTexture(unsigned int width, unsigned int height, const std::vector<Color>& image)
    {
        assert(image.size() == width * height);
        return createTexture(width, height, image.data());
    }

    TextureHandle GraphicsContext::createTexture(unsigned int width, unsigned int height, const Color* image)
    {
        GLuint texture;
        glGenTextures(1, &texture);
        TextureIdentifier id(texture);
        TextureHandle handle(id);

        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return handle;
    }

    TextureHandle GraphicsContext::createColorTexture(Color c)
    {
        GLuint texture;
        glGenTextures(1, &texture);
        TextureIdentifier id(texture);
        TextureHandle handle(id);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            1,
            1,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            &c);
        requireNoOpenGlError();

        return handle;
    }

    TextureArrayHandle GraphicsContext::createTextureArray(unsigned int width, unsigned int height, unsigned int mipMapLevels, std::vector<Color>& images)
    {
        assert(images.size() % (width * height) == 0);
        GLsizei depth = static_cast<GLsizei>(images.size()) / (width * height);
        GLuint texture;
        glGenTextures(1, &texture);
        TextureArrayIdentifier id(texture);
        TextureArrayHandle handle(id);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipMapLevels, GL_RGBA8, width, height, depth);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, depth, GL_RGBA, GL_UNSIGNED_BYTE, images.data());

        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return handle;
    }

    void GraphicsContext::enableDepthBuffer()
    {
        glEnable(GL_DEPTH_TEST);
    }

    void GraphicsContext::disableDepthBuffer()
    {
        glDisable(GL_DEPTH_TEST);
    }

    void GraphicsContext::enableCulling()
    {
        glEnable(GL_CULL_FACE);
    }

    ShaderHandle GraphicsContext::compileVertexShader(const std::string& source)
    {
        return compileShader(GL_VERTEX_SHADER, source);
    }

    ShaderHandle GraphicsContext::compileFragmentShader(const std::string& source)
    {
        return compileShader(GL_FRAGMENT_SHADER, source);
    }

    ShaderProgramHandle GraphicsContext::linkShaderProgram(
        ShaderIdentifier vertexShader,
        ShaderIdentifier fragmentShader,
        const std::vector<AttribMapping>& attribs)
    {
        ShaderProgramHandle program{ShaderProgramIdentifier{glCreateProgram()}};

        glAttachShader(program.get().value, vertexShader.value);
        glAttachShader(program.get().value, fragmentShader.value);

        for (const auto& attrib : attribs)
        {
            glBindAttribLocation(program.get().value, attrib.location, attrib.name.c_str());
        }

        glBindFragDataLocation(program.get().value, 0, "outColor");
        glLinkProgram(program.get().value);

        glDetachShader(program.get().value, vertexShader.value);
        glDetachShader(program.get().value, fragmentShader.value);

        GLint linkStatus;
        glGetProgramiv(program.get().value, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE)
        {
            throw GraphicsException("shader linking error");
        }

        return program;
    }

    ShaderHandle GraphicsContext::compileShader(GLenum shaderType, const std::string& source)
    {
        auto data = source.data();
        GLint length = static_cast<int>(source.size());
        ShaderHandle shader(ShaderIdentifier(glCreateShader(shaderType)));
        glShaderSource(shader.get().value, 1, &data, &length);
        glCompileShader(shader.get().value);
        GLint compileStatus;
        glGetShaderiv(shader.get().value, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus == GL_FALSE)
        {
            std::vector<char> errorBuffer(512);
            GLsizei len;
            glGetShaderInfoLog(shader.get().value, 512, &len, errorBuffer.data());
            std::string error(errorBuffer.data(), len);
            throw GraphicsException("shader compilation error: " + error);
        }

        return shader;
    }

    void GraphicsContext::enableDepthWrites()
    {
        glDepthMask(GL_TRUE);
    }

    void GraphicsContext::disableDepthWrites()
    {
        glDepthMask(GL_FALSE);
    }

    void GraphicsContext::enableDepthTest()
    {
        glDepthFunc(GL_LESS);
    }

    void GraphicsContext::disableDepthTest()
    {
        glDepthFunc(GL_ALWAYS);
    }

    GlMesh GraphicsContext::createTexturedMesh(const std::vector<GlTexturedVertex>& vertices, GLenum usage)
    {
        auto vao = genVertexArray();
        bindVertexArray(vao.get());

        auto vbo = genBuffer();
        bindBuffer(GL_ARRAY_BUFFER, vbo.get());

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GlTexturedVertex), vertices.data(), usage);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

        unbindBuffer(GL_ARRAY_BUFFER);
        unbindVertexArray();

        return GlMesh(std::move(vao), std::move(vbo), vertices.size());
    }

    GlMesh GraphicsContext::createTextureArrayMesh(const std::vector<GlTextureArrayVertex>& vertices, GLenum usage)
    {
        auto vao = genVertexArray();
        bindVertexArray(vao.get());

        auto vbo = genBuffer();
        bindBuffer(GL_ARRAY_BUFFER, vbo.get());

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GlTextureArrayVertex), vertices.data(), usage);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

        unbindBuffer(GL_ARRAY_BUFFER);
        unbindVertexArray();

        return GlMesh(std::move(vao), std::move(vbo), vertices.size());
    }

    GlMesh GraphicsContext::createColoredMesh(const std::vector<GlColoredVertex>& vertices, GLenum usage)
    {
        auto vao = genVertexArray();
        bindVertexArray(vao.get());

        auto vbo = genBuffer();
        bindBuffer(GL_ARRAY_BUFFER, vbo.get());

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GlColoredVertex), vertices.data(), usage);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

        unbindBuffer(GL_ARRAY_BUFFER);
        unbindVertexArray();

        return GlMesh(std::move(vao), std::move(vbo), vertices.size());
    }

    GlMesh GraphicsContext::createTexturedNormalMesh(const std::vector<GlTexturedNormalVertex>& vertices, GLenum usage)
    {
        auto vao = genVertexArray();
        bindVertexArray(vao.get());

        auto vbo = genBuffer();
        bindBuffer(GL_ARRAY_BUFFER, vbo.get());

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GlTexturedNormalVertex), vertices.data(), usage);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(5 * sizeof(GLfloat)));

        unbindBuffer(GL_ARRAY_BUFFER);
        unbindVertexArray();

        return GlMesh(std::move(vao), std::move(vbo), vertices.size());
    }

    GlMesh GraphicsContext::createColoredNormalMesh(const std::vector<GlColoredNormalVertex>& vertices, GLenum usage)
    {
        auto vao = genVertexArray();
        bindVertexArray(vao.get());

        auto vbo = genBuffer();
        bindBuffer(GL_ARRAY_BUFFER, vbo.get());

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GlColoredNormalVertex), vertices.data(), usage);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<void*>(6 * sizeof(GLfloat)));

        unbindBuffer(GL_ARRAY_BUFFER);
        unbindVertexArray();

        return GlMesh(std::move(vao), std::move(vbo), vertices.size());
    }

    VboHandle GraphicsContext::genBuffer()
    {
        GLuint vbo;
        glGenBuffers(1, &vbo);
        return VboHandle(VboIdentifier(vbo));
    }

    VaoHandle GraphicsContext::genVertexArray()
    {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        return VaoHandle(VaoIdentifier(vao));
    }

    void GraphicsContext::bindBuffer(GLenum type, VboIdentifier id)
    {
        glBindBuffer(type, id.value);
    }

    void GraphicsContext::bindVertexArray(VaoIdentifier id)
    {
        glBindVertexArray(id.value);
    }

    void GraphicsContext::unbindBuffer(GLenum type)
    {
        glBindBuffer(type, 0);
    }

    void GraphicsContext::unbindVertexArray()
    {
        glBindVertexArray(0);
    }

    void GraphicsContext::drawMesh(GLenum mode, const GlMesh& mesh, const Matrix4f& mvpMatrix, ShaderProgramIdentifier shader)
    {
        glUseProgram(shader.value);
        glBindVertexArray(mesh.vao.get().value);

        {
            auto location = glGetUniformLocation(shader.value, "mvpMatrix");
            glUniformMatrix4fv(location, 1, GL_FALSE, mvpMatrix.data);
        }

        glDrawArrays(mode, 0, mesh.vertexCount);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void GraphicsContext::drawUnitMesh(
        const GlMesh& mesh,
        const Matrix4f& modelMatrix,
        const Matrix4f& mvpMatrix,
        float seaLevel,
        ShaderProgramIdentifier shader)
    {
        glUseProgram(shader.value);
        glBindVertexArray(mesh.vao.get().value);

        {
            auto textureModelMatrix = glGetUniformLocation(shader.value, "modelMatrix");
            glUniformMatrix4fv(textureModelMatrix, 1, GL_FALSE, modelMatrix.data);
        }
        {
            auto textureViewMatrix = glGetUniformLocation(shader.value, "mvpMatrix");
            glUniformMatrix4fv(textureViewMatrix, 1, GL_FALSE, mvpMatrix.data);
        }

        {
            auto seaLevelUniform = glGetUniformLocation(shader.value, "seaLevel");
            glUniform1f(seaLevelUniform, seaLevel);
        }

        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void GraphicsContext::bindShader(ShaderProgramIdentifier shader)
    {
        glUseProgram(shader.value);
    }

    void GraphicsContext::unbindShader()
    {
        glUseProgram(0);
    }

    void GraphicsContext::bindTexture(TextureIdentifier texture)
    {
        glBindTexture(GL_TEXTURE_2D, texture.value);
    }

    void GraphicsContext::bindTextureArray(TextureArrayIdentifier texture)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture.value);
    }

    void GraphicsContext::unbindTexture()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void GraphicsContext::unbindTextureArray()
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    void GraphicsContext::enableBlending()
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void GraphicsContext::disableBlending()
    {
        glDisable(GL_BLEND);
    }

    UniformLocation GraphicsContext::getUniformLocation(ShaderProgramIdentifier shader, const std::string& name)
    {
        auto loc = glGetUniformLocation(shader.value, name.data());
        return UniformLocation(loc);
    }

    void GraphicsContext::setUniformFloat(UniformLocation location, float value)
    {
        glUniform1f(location.value, value);
    }

    void GraphicsContext::setUniformVec4(UniformLocation location, float a, float b, float c, float d)
    {
        glUniform4f(location.value, a, b, c, d);
    }

    void GraphicsContext::setUniformMatrix(UniformLocation location, const Matrix4f& matrix)
    {
        glUniformMatrix4fv(location.value, 1, GL_FALSE, matrix.data);
    }

    void GraphicsContext::setUniformBool(UniformLocation location, bool value)
    {
        glUniform1i(location.value, value);
    }

    void GraphicsContext::drawTriangles(const GlMesh& mesh)
    {
        glBindVertexArray(mesh.vao.get().value);
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        glBindVertexArray(0);
    }

    void GraphicsContext::drawLines(const GlMesh& mesh)
    {
        glBindVertexArray(mesh.vao.get().value);
        glDrawArrays(GL_LINES, 0, mesh.vertexCount);
        glBindVertexArray(0);
    }

    void GraphicsContext::drawLineLoop(const GlMesh& mesh)
    {
        glBindVertexArray(mesh.vao.get().value);
        glDrawArrays(GL_LINE_LOOP, 0, mesh.vertexCount);
        glBindVertexArray(0);
    }

    Sprite GraphicsContext::createSprite(
        const Rectangle2f& bounds,
        const Rectangle2f& textureRegion,
        const SharedTextureHandle& texture)
    {
        return Sprite(bounds, texture, std::make_shared<GlMesh>(createUnitTexturedQuad(textureRegion)));
    }

    GlMesh GraphicsContext::createUnitTexturedQuad(const Rectangle2f& textureRegion)
    {
        std::vector<GlTexturedVertex> vertices{
            // clang-format off
            {{-1.0f, -1.0f, 0.0f}, {textureRegion.left(), textureRegion.top()}},
            {{-1.0f,  1.0f, 0.0f}, {textureRegion.left(), textureRegion.bottom()}},
            {{ 1.0f,  1.0f, 0.0f}, {textureRegion.right(), textureRegion.bottom()}},

            {{ 1.0f,  1.0, 0.0f}, {textureRegion.right(), textureRegion.bottom()}},
            {{ 1.0f, -1.0f, 0.0f}, {textureRegion.right(), textureRegion.top()}},
            {{-1.0f, -1.0f, 0.0f}, {textureRegion.left(), textureRegion.top()}},
            // clang-format on
        };

        return createTexturedMesh(vertices, GL_STATIC_DRAW);
    }

    void GraphicsContext::enableStencilBuffer()
    {
        glEnable(GL_STENCIL_TEST);
    }

    void GraphicsContext::enableColorBuffer()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    void GraphicsContext::disableColorBuffer()
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }

    void GraphicsContext::disableStencilBuffer()
    {
        glDisable(GL_STENCIL_TEST);
    }

    void GraphicsContext::useStencilBufferAsMask()
    {
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }

    void GraphicsContext::clearStencilBuffer()
    {
        glClear(GL_STENCIL_BUFFER_BIT);
    }

    void GraphicsContext::useStencilBufferForWrites()
    {
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }

    void GraphicsContext::setViewport(int x, int y, int width, int height)
    {
        glViewport(x, y, width, height);
    }
}
