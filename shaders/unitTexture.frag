#version 150

in vec2 fragTexCoord;
in float height;
out vec4 outColor;

uniform sampler2D textureSampler;
uniform float seaLevel;

const vec4 waterTint = vec4(0.5, 0.5, 1.0, 1.0);
const vec4 normalTint = vec4(1.0, 1.0, 1.0, 1.0);

void main(void)
{
    vec4 baseColor = texture(textureSampler, fragTexCoord);
    outColor = baseColor * (height > seaLevel ? normalTint : waterTint);
}
