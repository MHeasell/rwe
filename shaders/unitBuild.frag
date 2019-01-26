#version 130

in vec2 fragTexCoord;
in float height;
in vec3 worldNormal;
out vec4 outColor;

uniform sampler2D textureSampler;
uniform float unitY;
uniform float seaLevel;
uniform bool shade;
uniform float percentComplete;

const vec3 waterTint = vec3(0.5, 0.5, 1.0);
const vec3 normalTint = vec3(1.0, 1.0, 1.0);
const vec3 lightDirection = normalize(vec3(-1.0, 4.0, 1.0));

vec3 shadeNormal()
{
    vec3 baseColor = vec3(texture(textureSampler, fragTexCoord));
    float lightIntensity = shade
        ? 1.5 * clamp(dot(worldNormal, lightDirection), 0.0, 1.0) + 0.5
        : 1.0;
    return baseColor * lightIntensity * (height > seaLevel ? normalTint : waterTint);
}

void main(void)
{
    if (height - unitY >= percentComplete * 100) discard;
    vec3 c = shadeNormal();
    outColor = vec4(c, 1.0);
}
