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

    void GraphicsContext::drawTextureRegion(float x, float y, float width, float height, const TextureRegion& texture)
    {
        drawTextureRegion(
            x,
            y,
            width,
            height,
            texture.texture,
            texture.region.left(),
            texture.region.top(),
            texture.region.width(),
            texture.region.height());
    }

    void GraphicsContext::drawTextureRegion(
        float x,
        float y,
        float width,
        float height,
        TextureIdentifier texture,
        float u,
        float v,
        float uw,
        float vh)
    {
        glBindTexture(GL_TEXTURE_2D, texture.value);
        glEnable(GL_TEXTURE_2D);

        // disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glTexCoord2f(u, v);
        glVertex2f(x, y);

        glTexCoord2f(u, v + vh);
        glVertex2f(x, y + height);

        glTexCoord2f(u + uw, v + vh);
        glVertex2f(x + width, y + height);

        glTexCoord2f(u + uw, v);
        glVertex2f(x + width, y);

        glEnd();
    }

    void GraphicsContext::drawTextureRegion(
        float x,
        float y,
        float width,
        float height,
        const SharedTextureHandle& texture,
        float u,
        float v,
        float uw,
        float vh)
    {
        drawTextureRegion(x, y, width, height, texture.get(), u, v, uw, vh);
    }

    void GraphicsContext::drawTexture(float x, float y, float width, float height, TextureIdentifier texture)
    {
        drawTextureRegion(x, y, width, height, texture, 0.0f, 0.0f, 1.0f, 1.0f);
    }

    void GraphicsContext::drawTexture(float x, float y, float width, float height, const SharedTextureHandle& texture)
    {
        drawTexture(x, y, width, height, texture.get());
    }

    void GraphicsContext::drawSprite(float x, float y, const Sprite& sprite)
    {
        drawTextureRegion(
            x + sprite.bounds.left(),
            y + sprite.bounds.top(),
            sprite.bounds.width(),
            sprite.bounds.height(),
            sprite.texture);
    }

    void GraphicsContext::applyCamera(const AbstractCamera& camera)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        multiplyMatrix(camera.getProjectionMatrix());

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        multiplyMatrix(camera.getViewMatrix());
    }

    void GraphicsContext::multiplyMatrix(const Matrix4f& m)
    {
        glMultMatrixf(m.data);
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

    void GraphicsContext::drawText(float x, float y, const std::string& text, const SpriteSeries& font)
    {
        auto it = utf8Begin(text);
        auto end = utf8End(text);
        for (; it != end; ++it)
        {
            auto ch = *it;
            if (ch > font.sprites.size())
            {
                ch = 0;
            }

            const auto& sprite = font.sprites[ch];

            if (ch != ' ')
            {
                drawSprite(x, y, sprite);
            }

            x += sprite.bounds.right();
        }
    }

    float GraphicsContext::getTextWidth(const std::string& text, const SpriteSeries& font)
    {
        float width = 0;
        auto it = utf8Begin(text);
        auto end = utf8End(text);
        for (; it != end; ++it)
        {
            auto ch = *it;
            if (ch > font.sprites.size())
            {
                ch = 0;
            }

            const auto& sprite = font.sprites[ch];

            width += sprite.bounds.right();
        }

        return width;
    }

    void GraphicsContext::drawTextCentered(float x, float y, const std::string& text, const SpriteSeries& font)
    {
        float width = getTextWidth(text, font);
        float halfWidth = width / 2.0f;
        float halfHeight = 5.0f;

        drawText(std::round(x - halfWidth), std::round(y + halfHeight), text, font);
    }

    void GraphicsContext::pushMatrix()
    {
        glPushMatrix();
    }

    void GraphicsContext::popMatrix()
    {
        glPopMatrix();
    }

    void GraphicsContext::fillColor(float x, float y, float width, float height, Color color)
    {
        glDisable(GL_TEXTURE_2D);

        // enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);

        glColor4ub(color.r, color.g, color.b, color.a);

        glVertex2f(x, y);
        glVertex2f(x, y + height);
        glVertex2f(x + width, y + height);
        glVertex2f(x + width, y);

        glEnd();
    }

    void GraphicsContext::drawTextWrapped(Rectangle2f area, const std::string& text, const SpriteSeries& font)
    {
        float x = 0.0f;
        float y = 0.0f;

        auto it = utf8Begin(text);
        auto end = utf8End(text);
        while (it != end)
        {

            auto endOfWord = findEndOfWord(it, end);
            auto width = getTextWidth(it, endOfWord, font);

            if (x + width > area.width())
            {
                y += 15.0f;
                x = 0;
            }

            for (; it != endOfWord; ++it)
            {
                auto ch = *it;
                if (ch > font.sprites.size())
                {
                    ch = 0;
                }

                const auto& sprite = font.sprites[ch];

                drawSprite(area.left() + x, area.top() + y, sprite);

                x += sprite.bounds.right();
            }

            if (endOfWord != end)
            {
                const auto& spaceSprite = font.sprites[' '];
                x += spaceSprite.bounds.right();
                ++it;
            }
        }
    }

    void GraphicsContext::drawMapTerrain(const MapTerrain& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    {
        glEnable(GL_TEXTURE_2D);

        // disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // disable blending
        glDisable(GL_BLEND);

        std::unordered_map<TextureIdentifier, std::vector<std::pair<unsigned int, unsigned int>>> batches;

        for (unsigned int dy = 0; dy < height; ++dy)
        {
            for (unsigned int dx = 0; dx < width; ++dx)
            {
                auto tileIndex = terrain.getTiles().get(x + dx, y + dy);
                const auto& tileTexture = terrain.getTileTexture(tileIndex);

                batches[tileTexture.texture.get()].emplace_back(dx, dy);
            }
        }

        for (const auto& batch : batches)
        {
            glBindTexture(GL_TEXTURE_2D, batch.first.value);

            glBegin(GL_QUADS);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            for (const auto& p : batch.second)
            {
                auto dx = p.first;
                auto dy = p.second;

                auto tileIndex = terrain.getTiles().get(x + dx, y + dy);
                auto tilePosition = terrain.tileCoordinateToWorldCorner(x + dx, y + dy);

                const auto& tileTexture = terrain.getTileTexture(tileIndex);

                glTexCoord2f(tileTexture.region.left(), tileTexture.region.top());
                glVertex3f(tilePosition.x, 0.0f, tilePosition.z);

                glTexCoord2f(tileTexture.region.left(), tileTexture.region.bottom());
                glVertex3f(tilePosition.x, 0.0f, tilePosition.z + MapTerrain::TileHeightInWorldUnits);

                glTexCoord2f(tileTexture.region.right(), tileTexture.region.bottom());
                glVertex3f(tilePosition.x + MapTerrain::TileWidthInWorldUnits, 0.0f, tilePosition.z + MapTerrain::TileHeightInWorldUnits);

                glTexCoord2f(tileTexture.region.right(), tileTexture.region.top());
                glVertex3f(tilePosition.x + MapTerrain::TileWidthInWorldUnits, 0.0f, tilePosition.z);
            }

            glEnd();
        }
    }

    void GraphicsContext::drawFeature(const MapFeature& feature)
    {
        if (feature.shadowAnimation)
        {
            disableDepth();
            float alpha = feature.transparentShadow ? 0.5f : 1.0f;
            drawStandingSprite(feature.position, (*feature.shadowAnimation)->sprites[0], alpha);
            enableDepth();
        }

        float alpha = feature.transparentAnimation ? 0.5f : 1.0f;
        drawStandingSprite(feature.position, feature.animation->sprites[0], alpha);
    }

    void GraphicsContext::drawStandingSprite(const Vector3f& position, const Sprite& sprite)
    {
        drawStandingSprite(position, sprite, 1.0f);
    }

    void GraphicsContext::drawStandingSprite(const Vector3f& position, const Sprite& sprite, float alpha)
    {
        auto u = sprite.texture.region.left();
        auto v = sprite.texture.region.top();
        auto uw = sprite.texture.region.width();
        auto vh = sprite.texture.region.height();

        // We stretch sprite y-dimension values by 2x
        // to correct for TA camera distortion.
        auto x = position.x + sprite.bounds.left();
        auto y = position.y - (sprite.bounds.top() * 2.0f);
        auto z = position.z;

        auto width = sprite.bounds.width();
        auto height = sprite.bounds.height() * 2.0f;

        glBindTexture(GL_TEXTURE_2D, sprite.texture.texture.get().value);
        glEnable(GL_TEXTURE_2D);

        // disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);

        glColor4f(1.0f, 1.0f, 1.0f, alpha);

        glTexCoord2f(u, v);
        glVertex3f(x, y, z);

        glTexCoord2f(u, v + vh);
        glVertex3f(x, y - height, z);

        glTexCoord2f(u + uw, v + vh);
        glVertex3f(x + width, y - height, z);

        glTexCoord2f(u + uw, v);
        glVertex3f(x + width, y, z);

        glEnd();
    }

    void GraphicsContext::drawShaderMesh(
        const ShaderMesh& mesh,
        ShaderProgramIdentifier textureShader,
        ShaderProgramIdentifier colorShader,
        const Matrix4f& modelMatrix,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix,
        float seaLevel)
    {
        glBindTexture(GL_TEXTURE_2D, mesh.texture.get().value);

        // disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // disable blending (meshes are opaque)
        glDisable(GL_BLEND);

        drawUnitMesh(mesh.texturedVertices, modelMatrix, viewMatrix, projectionMatrix, seaLevel, textureShader);

        glBindTexture(GL_TEXTURE_2D, 0);

        drawUnitMesh(mesh.coloredVertices, modelMatrix, viewMatrix, projectionMatrix, seaLevel, colorShader);
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

    ShaderMesh GraphicsContext::convertMesh(const Mesh& mesh)
    {
        std::vector<GlTexturedVertex> texturedVerticesBuffer;
        texturedVerticesBuffer.reserve(mesh.faces.size() * 3);

        for (const auto& t : mesh.faces)
        {
            texturedVerticesBuffer.emplace_back(t.a.position, t.a.textureCoord);
            texturedVerticesBuffer.emplace_back(t.b.position, t.b.textureCoord);
            texturedVerticesBuffer.emplace_back(t.c.position, t.c.textureCoord);
        }

        auto texturedMesh = createTexturedMesh(texturedVerticesBuffer, GL_STATIC_DRAW);

        std::vector<GlColoredVertex> coloredVerticesBuffer;
        for (const auto& t : mesh.colorFaces)
        {
            auto color = Vector3f(t.color.r, t.color.g, t.color.b) / 255.0f;

            coloredVerticesBuffer.emplace_back(t.a.position, color);
            coloredVerticesBuffer.emplace_back(t.b.position, color);
            coloredVerticesBuffer.emplace_back(t.c.position, color);
        }

        auto coloredMesh = createColoredMesh(coloredVerticesBuffer, GL_STATIC_DRAW);

        return ShaderMesh(mesh.texture, std::move(texturedMesh), std::move(coloredMesh));
    }

    void GraphicsContext::drawWireframeSelectionMesh(
        const GlMesh& mesh,
        const Matrix4f& modelMatrix,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix,
        ShaderProgramIdentifier shader)
    {
        glDisable(GL_BLEND);
        drawMesh(GL_LINE_LOOP, mesh, modelMatrix, viewMatrix, projectionMatrix, shader);
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

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glBindTexture(GL_TEXTURE_2D, 0);

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

        glDisable(GL_BLEND);
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

    void GraphicsContext::drawLinesMesh(
        const GlMesh& mesh,
        const Matrix4f& modelMatrix,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix,
        ShaderProgramIdentifier shader)
    {
        drawMesh(GL_LINES, mesh, modelMatrix, viewMatrix, projectionMatrix, shader);
    }

    void GraphicsContext::drawTrisMesh(
        const GlMesh& mesh,
        const Matrix4f& modelMatrix,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix,
        ShaderProgramIdentifier shader)
    {
        drawMesh(GL_TRIANGLES, mesh, modelMatrix, viewMatrix, projectionMatrix, shader);
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

    void GraphicsContext::drawMesh(
        GLenum mode,
        const GlMesh& mesh,
        const Matrix4f& modelMatrix,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix,
        ShaderProgramIdentifier shader)
    {
        glUseProgram(shader.value);
        glBindVertexArray(mesh.vao.get().value);

        {
            auto location = glGetUniformLocation(shader.value, "modelMatrix");
            glUniformMatrix4fv(location, 1, GL_FALSE, modelMatrix.data);
        }
        {
            auto location = glGetUniformLocation(shader.value, "viewMatrix");
            glUniformMatrix4fv(location, 1, GL_FALSE, viewMatrix.data);
        }
        {
            auto location = glGetUniformLocation(shader.value, "projectionMatrix");
            glUniformMatrix4fv(location, 1, GL_FALSE, projectionMatrix.data);
        }

        glDrawArrays(mode, 0, mesh.vertexCount);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void GraphicsContext::drawUnitMesh(
        const GlMesh& mesh,
        const Matrix4f& modelMatrix,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix,
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
            auto textureViewMatrix = glGetUniformLocation(shader.value, "viewMatrix");
            glUniformMatrix4fv(textureViewMatrix, 1, GL_FALSE, viewMatrix.data);
        }
        {
            auto textureProjectionMatrix = glGetUniformLocation(shader.value, "projectionMatrix");
            glUniformMatrix4fv(textureProjectionMatrix, 1, GL_FALSE, projectionMatrix.data);
        }

        {
            auto seaLevelUniform = glGetUniformLocation(shader.value, "seaLevel");
            glUniform1f(seaLevelUniform, seaLevel);
        }

        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);

        glBindVertexArray(0);
        glUseProgram(0);
    }
}
