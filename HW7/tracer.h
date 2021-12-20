// 컴퓨터공학과 1976433 황주이
#pragma once

#include "raytraceData.h"

class tracer {
public:
	tracer();
	~tracer();

	void findPointOnRay(raytraceData::ray* r, float t, raytraceData::point* p);
	int raySphereIntersect(raytraceData::ray* r, raytraceData::sphere* s, float* t);
	int rayHalfspaceIntersect(raytraceData::ray* r, raytraceData::halfspace* hs, float* t);
	void findSphereNormal(raytraceData::sphere* s, raytraceData::point* p, raytraceData::vector* n);
	void trace(raytraceData::ray* r, raytraceData::point* p, raytraceData::vector* n, raytraceData::material** m, bool* reflective, bool* refractive);

	raytraceData::sphere* s1;	   /* the scene: so far, just one sphere */
	raytraceData::halfspace* hs1;
	raytraceData::sphere* s2;
	raytraceData::sphere* s3;
};