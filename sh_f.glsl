#version 330

in vec3 fragNorm;	// Interpolated model-space normal
in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

out vec4 outCol;	// Final pixel color

void main() {
	// Visualize normals as colors
	//outCol = normalize(fragNorm) * 0.5f + vec3(0.5f);

	outCol = texture(texture1, TexCoord);
}