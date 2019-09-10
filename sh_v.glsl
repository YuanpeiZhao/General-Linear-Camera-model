#version 330

layout(location = 0) in vec3 pos;		// Model-space position
layout(location = 1) in vec3 norm;		// Model-space normal
layout(location = 2) in vec2 aTexCoord;

out vec3 fragNorm;	// Model-space interpolated normal
out vec2 TexCoord;

void main() {
	// Transform vertex position
	gl_Position = vec4(pos, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);

	// Interpolate normals
	fragNorm = norm;
}