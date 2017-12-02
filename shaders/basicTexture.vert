#version 150

uniform mat4 mvpMatrix;

in vec3 position;
in vec2 texCoord;

out vec2 fragTexCoord;

void main(void)
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragTexCoord = texCoord;
}
