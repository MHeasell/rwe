#version 130

in vec3 position;
in vec2 texCoord;

out vec2 fragTexCoord;

void main(void)
{
    gl_Position = vec4(position, 1.0);
    fragTexCoord = texCoord;
}
