#ifndef TRIANGLEMESHH
#define TRIANGLEMESHH

#include "hitable.h"

class triangleMesh : public hitable {
	public:
		triangleMesh() {}
		triangleMesh(vec3 aa, vec3 bb, vec3 cc, vec3 col) : a(aa), b(bb), c(cc), color(col) {};
		virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
		vec3 a, b, c;
		vec3 color;
};


bool triangleMesh::hit(const ray& r, float tmin, float tmax, hit_record& rec) const {
	vec3 orig = r.origin();
	vec3 dir = r.direction();
	vec3 v0 = a;
	vec3 v1 = b;
	vec3 v2 = c;

	// E1
	vec3 E1 = v1 - v0;

	// E2
	vec3 E2 = v2 - v0;

	// P
	vec3 P = cross(dir, E2);

	// determinant
	float det = dot(E1, P);

	// keep det > 0, modify T accordingly
	vec3 T;
	if (det > 0)
	{
		T = orig - v0;
	}
	else
	{
		T = v0 - orig;
		det = - det;
	}

	// If determinant is near zero, ray lies in plane of triangle
	if (det < 0.0001f)
		return false;

	// Calculate u and make sure u <= 1
	float u = dot(T, P);
	if (u < 0.0f || u > det)
		return false;

	// Q
	vec3 Q = cross(T, E1);

	// Calculate v and make sure u + v <= 1
	float v = dot(dir, Q);
	if (v < 0.0f || u + v > det)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	float t = dot(E2, Q);

	float fInvDet = 1.0f / det;
	t *= fInvDet;

	if (t < tmax && t > tmin) {
		rec.t = t;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = normalize(cross(E2, E1));
		rec.color = color;
		return true;
	}


	return false;
}

#endif