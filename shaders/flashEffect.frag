#version 130

in vec2 fragTexCoord;
out vec4 outColor;

uniform float intensity;
uniform vec3 color;

float getBrightnessIncreaseVal(float increaseFrac)
{
    return 1.0 - (1.0 / (1.0 + increaseFrac));
}

vec3 getBrightnessIncreaseVal(vec3 increaseFrac)
{
    return vec3(
    getBrightnessIncreaseVal(increaseFrac.r),
    getBrightnessIncreaseVal(increaseFrac.g),
    getBrightnessIncreaseVal(increaseFrac.b));
}

void main(void)
{
    vec2 fromCenter = fragTexCoord - vec2(0.5, 0.5);
    float distance = sqrt(dot(fromCenter, fromCenter));

    if (distance >= 0.5)
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float distanceToEdge = 0.5 - distance;
    float normalizedDistanceToEdge = distanceToEdge * 2.0;

    float brightnessFactor = normalizedDistanceToEdge * normalizedDistanceToEdge;

    brightnessFactor *= intensity;

    outColor = vec4(getBrightnessIncreaseVal(brightnessFactor * color), 1.0);
}
