#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec3 position;
in vec3 color;

out vec3 fragColor;
out float height;

void main(void)
{
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
    fragColor = color;
    height = worldPosition.y;
}

