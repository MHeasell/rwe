#version 130

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;

in vec3 position;
in vec3 color;
in vec3 normal;

out vec3 fragColor;
out float height;
out vec3 worldNormal;

void main(void)
{
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragColor = color;
    height = worldPosition.y;
    worldNormal = mat3(modelMatrix) * normal;
}

