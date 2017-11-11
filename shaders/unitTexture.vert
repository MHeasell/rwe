#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec3 position;
in vec2 texCoord;

out vec2 fragTexCoord;
out float height;

void main(void)
{
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
    fragTexCoord = texCoord;
    height = worldPosition.y;
}
