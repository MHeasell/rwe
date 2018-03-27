#version 130

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec2 fragTexCoord;
out float height;
out vec3 worldNormal;

void main(void)
{
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragTexCoord = texCoord;
    height = worldPosition.y;
    worldNormal = mat3(modelMatrix) * normal;
}
