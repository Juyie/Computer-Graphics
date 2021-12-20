// 컴퓨터공학과 1976433 황주이
#pragma once

#include <glad/glad.h>
#include "raytraceData.h"
#include "lowlevel.h"
#include "shader.h"
#include "tracer.h"

class raytrace {
public:
	raytrace(int w, int h);
	~raytrace();

	GLubyte* display(void);

private:
	void initScene();
	void initCamera(int w, int h);
	void drawScene(void);	
	raytraceData::point* makePoint(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	raytraceData::sphere* makeSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat r);
	raytraceData::halfspace* makeHalfspace(GLfloat x, GLfloat y, GLfloat z, GLfloat nx, GLfloat ny, GLfloat nz);
	void rayColor(raytraceData::ray* r, raytraceData::color* c, int step);
	void calculateDirection(raytraceData::point* p, raytraceData::point* q, raytraceData::point* v);

	int width;     /* width of window in pixels */
	int height;    /* height of window in pixels */
	raytraceData::sphere* s1;	   /* the scene: so far, just one sphere */
	raytraceData::halfspace* hs1;
	raytraceData::sphere* s2;
	raytraceData::sphere* s3;

	/* parameters defining the camera */
	raytraceData::point* viewpoint;
	GLfloat pnear;  /* distance from viewpoint to image plane */
	GLfloat fovx;  /* x-angle of view frustum */
	lowlevel lowlevel;
	shader shader;
	tracer tracer;
};