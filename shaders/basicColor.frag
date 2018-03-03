#version 130

in vec3 fragColor;
out vec4 outColor;

uniform float alpha;

void main() {
	outColor = vec4(fragColor, alpha);
}
