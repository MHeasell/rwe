#version 130

in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D screenTexture;
uniform sampler2D dodgeMask;

void main(void)
{
    vec4 screenValue = texture(screenTexture, fragTexCoord);
    vec4 dodgeMaskValue = texture(dodgeMask, fragTexCoord);
    outColor = vec4(screenValue.rgb / (vec3(1.0, 1.0, 1.0) - dodgeMaskValue.rgb), 1.0);
}
