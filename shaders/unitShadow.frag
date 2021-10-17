#version 130

in vec2 fragTexCoord;
in float height;
out vec4 outColor;

uniform float groundHeight;

uniform sampler2D textureSampler;

void main(void)
{
    if (height < groundHeight)
    {
        discard;
    }

    vec4 baseColor = texture(textureSampler, fragTexCoord);
    if (baseColor.a < 0.5)
    {
        discard;
    }

    outColor = vec4(0.0, 0.0, 0.0, 1.0);
}
