// 컴퓨터공학과 1976433 황주이
#pragma once

#include "shader.h"
#include "math.h"
#include "raytrace.h"
#include "tracer.h"
#include <iostream>

using namespace raytraceData;

shader::shader()
{
}

shader::~shader()
{
}

material* shader::makeMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat amb) {
	material* m;

	/* allocate memory */
	m = new material();

	/* put stuff in it */
	m->c.r = r;
	m->c.g = g;
	m->c.b = b;
	m->amb = amb;
	return(m);
}

// 거리 계산
float dist(point* c,point* p) {
	float x = p->x - c->x;
	float y = p->y - c->y;
	float z = p->z - c->z;
	float d = pow((x * x + y * y + z * z), 0.5);
	return d;
}

point* normalize(point* p) {
	point *norm = new point();
	float x = p->x / p->w;
	float y = p->y / p->w;
	float z = p->z / p->w;
	float size = pow((x * x + y * y + z * z), 0.5);

	norm->x = x / size;
	norm->y = y / size;
	norm->z = z / size;
	norm->w = 1.0f;
	return(norm);
}


/* LIGHTING CALCULATIONS */

/* shade */
/* color of point p with normal vector n and material m returned in c */
void shader::shade(point* p, vector* n, material* m, color* c, color* reflected, color* refracted) {
	/* so far, just finds ambient component of color */
	// diffuse와 specular계산하여 추가 (Step1)
	// reflection 추가 (Step3)
	// shadow 추가 (Step4)

	//float fradatten = 1 / (1 + dist(p) * dist(p));
	// Idiff = max(N*L, 0) * Kdiff
	// Ispec = max(N*H, 0) * Kspec

	point dirlight;
	point pointlight;
	point norm_point;
	point* norm_l;
	float fradatten;
	float fangatten;
	float i_diff;
	point v;
	point* norm_v;
	point r;
	float n_dot_l;
	float ns;
	float i_spec;
	color kd;
	color ks;
	float vis_i = 1.0;
	tracer tracer;
	ray shadow_ray;
	float t = 0;
	int hit1 = FALSE;
	int hit2 = FALSE;
	int hit3 = FALSE;
	
	// point light
	
	pointlight.x = -1.0;
	pointlight.y = -1.5;
	pointlight.z = -1.0;
	pointlight.w = 1.0;

	
	norm_point.x = p->x - pointlight.x;
	norm_point.y = p->y - pointlight.y;
	norm_point.z = p->z - pointlight.z;
	norm_l = normalize(&norm_point);

	fradatten = 1 / (1 + dist(&pointlight, p) * dist(&pointlight, p));
	fangatten = 1.0;
	

	// directional light 사용, 아래는 L값
	/*
	dirlight.x = 0.5f;
	dirlight.y = -3.0f;
	dirlight.z = 2.0f;
	dirlight.w = 1.0f;
	norm_l = normalize(&dirlight);


	// directional light는 attenuation 없음
	fradatten = 1.0f;
	fangatten = 1.0f;
	*/

	i_diff = (norm_l->x * n->x + norm_l->y * n->y + norm_l->z * n->z) * 3.2;

	v.x = 0.0f - p->x;
	v.y = 0.0f - p->y;
	v.z = 0.0f - p->z;
	v.w = 1.0f;
	norm_v = normalize(&v);
	
	n_dot_l = n->x * norm_l->x + n->y * norm_l->y + n->z * norm_l->z;
	r.x = n_dot_l * n->x - norm_l->x;
	r.y = n_dot_l * n->y - norm_l->y;
	r.z = n_dot_l * n->z - norm_l->z;
	ns = 10.0f;
	i_spec = (norm_v->x * r.x + norm_v->y * r.y + norm_v->z * r.z) * 3.2;
	
	if (i_diff < 0.0f) {
		i_diff = 0.0f;
		i_spec = 0.0f;
	}

	if (i_spec < 0.0f) {
		i_spec = 0.0f;
	}
	else {
		i_spec = pow(i_spec, ns);
	}

	// shadow
	shadow_ray.start = p;
	shadow_ray.end = norm_l;
	hit1 = tracer.raySphereIntersect(&shadow_ray, s1, &t);
	hit2 = tracer.raySphereIntersect(&shadow_ray, s2, &t);
	hit3 = tracer.raySphereIntersect(&shadow_ray, s3, &t);
	if (hit1 || hit2 || hit3) {
		vis_i = 0.0;
	}

	kd.r = 0.6f;
	kd.g = 0.6f;
	kd.b = 0.6f;

	ks.r = 0.8f;
	ks.g = 0.8f;
	ks.b = 0.8f;
	c->r = (m->amb * m->c.r + (fradatten * fangatten * (i_diff * kd.r + i_spec * ks.r) * 2) * vis_i) + reflected->r * 0.5 + refracted->r;
	c->g = (m->amb * m->c.g + (fradatten * fangatten * (i_diff * kd.g + i_spec * ks.g) * 2) * vis_i) + reflected->g * 0.5 + refracted->g;
	c->b = (m->amb * m->c.b + (fradatten * fangatten * (i_diff * kd.b + i_spec * ks.b) * 2) * vis_i) + reflected->b * 0.5 + refracted->b;
	
	
	/* clamp color values to 1.0 */
	if (c->r > 1.0) c->r = 1.0;
	if (c->g > 1.0) c->g = 1.0;
	if (c->b > 1.0) c->b = 1.0;
}
