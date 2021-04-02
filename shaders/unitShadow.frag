#version 130

in float height;
out vec4 outColor;

uniform float groundHeight;

void main(void)
{
    if (height < groundHeight)
    {
        discard;
    }

    outColor = vec4(0.0, 0.0, 0.0, 1.0);
}
