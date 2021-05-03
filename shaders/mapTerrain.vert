#version 130

uniform mat4 mvpMatrix;

in vec3 position;
in vec3 texCoord;

out vec3 fragTexCoord;

void main(void)
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragTexCoord = texCoord;
}
