#version 130

in vec3 fragColor;
in float height;
out vec4 outColor;

uniform float seaLevel;

const vec4 waterTint = vec4(0.5, 0.5, 1.0, 1.0);
const vec4 normalTint = vec4(1.0, 1.0, 1.0, 1.0);

void main(void)
{
    vec4 baseColor = vec4(fragColor, 1.0);
    outColor = baseColor * (height > seaLevel ? normalTint : waterTint);
}
