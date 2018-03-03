#version 130

uniform mat4 mvpMatrix;

in vec3 position;
in vec3 color;
out vec3 fragColor;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragColor = color;
}
