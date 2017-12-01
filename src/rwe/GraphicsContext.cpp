#include "GraphicsContext.h"
#include "rwe_string.h"

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

    GlColoredVertex::GlColoredVertex(const Vector3f& pos, const Vector3f& color)
        : x(pos.x), y(pos.y), z(pos.z), r(color.x), g(color.y), b(color.z)
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
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

    void GraphicsContext::enableDepth()
    {
        glEnable(GL_DEPTH_TEST);
    }

    void GraphicsContext::disableDepth()
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
        GLint length = source.size();
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

    void GraphicsContext::beginUnitShadow()
    {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_ALWAYS);
        glClear(GL_STENCIL_BUFFER_BIT);
    }

    void GraphicsContext::endUnitShadow()
    {
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilMask(0x00);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        glBegin(GL_TRIANGLES);

        glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);

        glVertex3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);

        glEnd();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_STENCIL_TEST);
    }

    void GraphicsContext::enableDepthWrites()
    {
        glDepthMask(GL_TRUE);
    }

    void GraphicsContext::disableDepthWrites()
    {
        glDepthMask(GL_FALSE);
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

    void GraphicsContext::unbindTexture()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
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

    void GraphicsContext::setUniformMatrix(UniformLocation location, const Matrix4f& matrix)
    {
        glUniformMatrix4fv(location.value, 1, GL_FALSE, matrix.data);
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
        std::vector<GlTexturedVertex> vertices{
            {{bounds.left(), bounds.top(), 0.0f}, {textureRegion.left(), textureRegion.top()}},
            {{bounds.left(), bounds.bottom(), 0.0f}, {textureRegion.left(), textureRegion.bottom()}},
            {{bounds.right(), bounds.bottom(), 0.0f}, {textureRegion.right(), textureRegion.bottom()}},

            {{bounds.right(), bounds.bottom(), 0.0f}, {textureRegion.right(), textureRegion.bottom()}},
            {{bounds.right(), bounds.top(), 0.0f}, {textureRegion.right(), textureRegion.top()}},
            {{bounds.left(), bounds.top(), 0.0f}, {textureRegion.left(), textureRegion.top()}},
        };

        auto mesh = createTexturedMesh(vertices, GL_STATIC_DRAW);
        GlTexturedMesh texturedMesh(texture, std::move(mesh));

        return Sprite(bounds, std::move(texturedMesh));
    }
}
