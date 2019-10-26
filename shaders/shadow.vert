#version 130

uniform mat4 mvpMatrix;

in vec3 position;

void main(void)
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
}
