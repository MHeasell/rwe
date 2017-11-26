#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform float alpha;

in vec3 position;
in vec2 texCoord;

out vec2 fragTexCoord;
out float fragAlpha;

void main(void)
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    fragTexCoord = texCoord;
    fragAlpha = alpha;
}
