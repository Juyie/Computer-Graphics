// 컴퓨터공학과 1976433 황주이
#pragma once

#include <glad/glad.h>
#include <stdlib.h> 
#include "raytraceData.h"
#include "tracer.h"

class shader {
public: 
	shader();
	~shader();

	raytraceData::material* makeMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat amb);
	void shade(raytraceData::point* p, raytraceData::vector* n, raytraceData::material* m, raytraceData::color* c, raytraceData::color* reflected, raytraceData::color* refracted);

	raytraceData::sphere* s1;	   /* the scene: so far, just one sphere */
	raytraceData::halfspace* hs1;
	raytraceData::sphere* s2;
	raytraceData::sphere* s3;
};
