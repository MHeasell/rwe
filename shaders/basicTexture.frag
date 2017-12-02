#version 150

in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D textureSampler;
uniform float alpha;

void main(void)
{
    outColor = texture(textureSampler, fragTexCoord) * vec4(1.0, 1.0, 1.0, alpha);
}
