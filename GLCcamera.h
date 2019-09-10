#ifndef GLCCAMERAH
#define GLCCAMERAH

#include "ray.h"

class GLCcamera {
public:
	GLCcamera() {}
	GLCcamera(vec3 a1, vec3 b1, vec3 c1, vec3 a2, vec3 b2, vec3 c2) { 
		planeUV_a = a1; planeUV_b = b1; planeUV_c = c1;
		planeST_a = a2; planeST_b = b2; planeST_c = c2;
		coords = vec3(0.0f, 0.0f, 0.0f);
		reverse = vec2(-1.0f, -1.0f);
	}
	
	vec3 planeUV_a, planeUV_b, planeUV_c;
	vec3 planeST_a, planeST_b, planeST_c;
	vec3 coords;
	vec2 reverse;

	ray genRay(vec3 screenPos);
	void reset(vec3 a1, vec3 b1, vec3 c1, vec3 a2, vec3 b2, vec3 c2);
	void setCoords(vec3 coo);
	void reverseX();
	void reverseY();
};

void GLCcamera::reset(vec3 a1, vec3 b1, vec3 c1, vec3 a2, vec3 b2, vec3 c2)
{
	planeUV_a = a1; planeUV_b = b1; planeUV_c = c1;
	planeST_a = a2; planeST_b = b2; planeST_c = c2;
	coords = vec3(0.0f, 0.0f, 0.0f);
}

void GLCcamera::reverseX()
{
	reverse.x *= -1.0f;
}

void GLCcamera::reverseY()
{
	reverse.y *= -1.0f;
}

void GLCcamera::setCoords(vec3 coo)
{
	coords = coo;
}

ray GLCcamera::genRay(vec3 screenPos)
{
	mat3 M1 = mat3(planeST_a.x, planeST_a.y, planeST_a.z,
		planeST_b.x, planeST_b.y, planeST_b.z, 
		planeST_c.x, planeST_c.y, planeST_c.z);

	mat3 M2 = mat3(planeUV_a.x, planeUV_a.y, planeUV_a.z,
		planeUV_b.x, planeUV_b.y, planeUV_b.z,
		planeUV_c.x, planeUV_c.y, planeUV_c.z);

	vec3 UVPos = M2 * inverse(M1) * screenPos;

	mat4 rot = rotate(mat4(1.0f), radians(coords.y), vec3(reverse.y, 0.0, 0.0));
	rot = rotate(rot, radians(coords.x), vec3(0.0, reverse.x, 0.0));

	vec4 UVRotatedPos = rot * vec4(UVPos, 0.0f);
	vec4 DirRotatedPos = rot * vec4(screenPos - UVPos, 0.0f);
	
	return ray(vec3(UVRotatedPos.x, UVRotatedPos.y, UVRotatedPos.z), vec3(DirRotatedPos.x , DirRotatedPos.y, DirRotatedPos.z));
}


#endif
