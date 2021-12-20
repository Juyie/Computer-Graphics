//컴퓨터공학과 1976433 황주이
#include "tracer.h"
#include <iostream>

#include <math.h>

using namespace raytraceData;

tracer::tracer():
	s1(NULL)
{
}

tracer::~tracer()
{
}

/* point on ray r parameterized by t is returned in p */
void tracer::findPointOnRay(ray* r, float t, point* p) {
	p->x = r->start->x + t * r->end->x;
	p->y = r->start->y + t * r->end->y;
	p->z = r->start->z + t * r->end->z;
	p->w = 1.0;
}

int tracer::rayHalfspaceIntersect(ray* r, halfspace* hs, float* t) {
	float a, b;  /* coefficients of quadratic equation */
	float D;    /* discriminant */

	a = ((hs->c->x - r->start->x) * hs->n->x + (hs->c->y - r->start->y) * hs->n->y + (hs->c->z - r->start->z) * hs->n->z);
	b = r->end->x * hs->n->x + r->end->y * hs->n->y + r->end->z * hs->n->z;
	D = a / b;
	
	if (D < 0.0001) {  /* no intersection */
		return (FALSE);
	}
	else {
		*t = D;
		return(TRUE);
	}
}

void findHalfspaceNormal(halfspace* hs, vector* n) {
	float size = sqrt(hs->n->x * hs->n->x + hs->n->y * hs->n->y + hs->n->z * hs->n->z);
	n->x = hs->n->x / size;
	n->y = hs->n->y / size;
	n->z = hs->n->z / size;
	n->w = 0.0;
}

/* raySphereIntersect */
/* returns TRUE if ray r hits sphere s, with parameter value in t */
int tracer::raySphereIntersect(ray* r, sphere* s, float* t) {
	point p;   /* start of transformed ray */
	float a, b, c;  /* coefficients of quadratic equation */
	float D;    /* discriminant */
	point* v;

	/* transform ray so that sphere center is at origin */
	/* don't use matrix, just translate! */
	p.x = r->start->x - s->c->x;
	p.y = r->start->y - s->c->y;
	p.z = r->start->z - s->c->z;
	v = r->end; /* point to direction vector */

	a = v->x * v->x + v->y * v->y + v->z * v->z;
	b = 2 * (v->x * p.x + v->y * p.y + v->z * p.z);
	c = p.x * p.x + p.y * p.y + p.z * p.z - s->r * s->r;

	D = b * b - 4 * a * c;

	if (D < 0) {  /* no intersection */
		return (FALSE);
	}
	else if (D == 0) {
		*t = -b / 2 * a;
		return(FALSE);
	}
	else {
		D = static_cast<float>(sqrt(D));
		/* First check the root with the lower value of t: */
		/* this one, since D is positive */
		*t = (-b - D) / (2 * a);
		/* ignore roots which are less than zero (behind viewpoint) */
		if (*t < 0.0001) {
			*t = (-b + D) / (2 * a);
		}
		if (*t < 0.0001) { return(FALSE); }
		else return(TRUE);
	}
}

/* normal vector of s at p is returned in n */
/* note: dividing by radius normalizes */
void tracer::findSphereNormal(sphere* s, point* p, vector* n) {
	n->x = (p->x - s->c->x) / s->r;
	n->y = (p->y - s->c->y) / s->r;
	n->z = (p->z - s->c->z) / s->r;
	n->w = 0.0;
}

/* trace */
/* If something is hit, returns the finite intersection point p,
   the normal vector n to the surface at that point, and the surface
   material m. If no hit, returns an infinite point (p->w = 0.0) */
void tracer::trace(ray* r, point* p, vector* n, material** m, bool* reflective, bool* refractive) {
	float t1 = 0;     /* parameter value at first hit */
	float t2 = 0;
	int hit1 = FALSE; // sphere
	int hit2 = FALSE; // halfspace
	float t3 = 0;
	int hit3 = FALSE;
	float t4 = 0;
	int hit4 = FALSE;
	ray* reflected_ray = r;

	// ray와 sphere가 만나는지 확인. 이미 구현 되어있음.
	// 만나면 position와 normal 찾아줌.
	// sphere 외의 다른 물체 추가 하려면 Intersect와 Normal 찾는 부분 추가 구현 (Step2)
	hit1 = raySphereIntersect(r, s1, &t1);
	hit2 = rayHalfspaceIntersect(r, hs1, &t2);
	hit3 = raySphereIntersect(r, s2, &t3);
	hit4 = raySphereIntersect(r, s3, &t4);
	
	if (hit2) {
		*m = hs1->m;
		findPointOnRay(r, t2, p);
		findHalfspaceNormal(hs1, n);
		*reflective = false;
		*refractive = false;
	}

	if (hit3) {
		*m = s2->m;
		findPointOnRay(r, t3, p);
		findSphereNormal(s2, p, n);
		*reflective = true;
		*refractive = false;
	}

	if (hit1) {
		*m = s1->m;
		findPointOnRay(r, t1, p);
		findSphereNormal(s1, p, n);
		*reflective = false;
		*refractive = false;
	}

	if (hit4) {
		*m = s3->m;
		findPointOnRay(r, t4, p);
		findSphereNormal(s3, p, n);
		*reflective = false;
		*refractive = true;
	}

	if (!hit1 && !hit2 && !hit3 && !hit4) {
		/* indicates nothing was hit */
		p->w = 0.0;
	}
}