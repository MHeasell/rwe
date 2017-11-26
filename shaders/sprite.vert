#version 150

uniform mat4 mvpMatrix;
uniform float alpha;

in vec3 position;
in vec2 texCoord;

out vec2 fragTexCoord;
out float fragAlpha;

void main(void)
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
    fragTexCoord = texCoord;
    fragAlpha = alpha;
}
