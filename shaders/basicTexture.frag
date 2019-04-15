#version 130

in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D textureSampler;
uniform vec4 tint;

void main(void)
{
    outColor = texture(textureSampler, fragTexCoord) * tint;
}
