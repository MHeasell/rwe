#version 130

in vec3 fragTexCoord;
out vec4 outColor;

uniform sampler2DArray textureArraySampler;

void main(void)
{
    outColor = texture(textureArraySampler, fragTexCoord);
}
