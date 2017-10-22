#version 150

in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D textureSampler;

void main(void)
{
    outColor = texture(textureSampler, fragTexCoord);
}
