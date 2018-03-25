#version 130

in vec3 fragColor;
in float height;
in vec3 worldNormal;
out vec4 outColor;

uniform float seaLevel;
uniform bool shade;

const vec3 waterTint = vec3(0.5, 0.5, 1.0);
const vec3 normalTint = vec3(1.0, 1.0, 1.0);
const vec3 lightDirection = normalize(vec3(-1.0, 4.0, 1.0));

void main(void)
{
    vec3 baseColor = fragColor;
    float lightIntensity = shade
        ? 1.5 * clamp(dot(worldNormal, lightDirection), 0.0, 1.0) + 0.5
        : 1.0;
    outColor = vec4(baseColor * lightIntensity * (height > seaLevel ? normalTint : waterTint), 1.0);
}
