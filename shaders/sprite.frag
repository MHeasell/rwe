#version 150

in vec2 fragTexCoord;
in float fragAlpha;
out vec4 outColor;

uniform sampler2D textureSampler;

void main(void)
{
    outColor = texture(textureSampler, fragTexCoord) * vec4(1.0, 1.0, 1.0, fragAlpha);
}
