// 컴퓨터공학과 1976433 황주이
#include "raytrace.h"
#include <math.h>
#include <iostream>

using namespace raytraceData;

raytrace::raytrace(int w, int h)
{
	/* low-level graphics setup */
	lowlevel.initCanvas(w, h);

	/* raytracer setup */
	initCamera(w, h);
	initScene();
}

raytrace::~raytrace()
{
	if (s1 != NULL) delete s1;
	if (viewpoint != NULL) delete viewpoint;
}

GLubyte* raytrace::display(void)
{	
	drawScene();  /* draws the picture in the canvas */

	return lowlevel.flushCanvas();  /* draw the canvas to the OpenGL window */	
}

void raytrace::initScene()
{
	s1 = makeSphere(-0.28, 0.0, -2.5, 0.18);
	s1->m = shader.makeMaterial(0.8, 0.1, 0.15, 0.3);

	tracer.s1 = s1;
	shader.s1 = s1;

	hs1 = makeHalfspace(0.0, 0.4, 0.0, 0.0, -1.0, 0.05);
	hs1->m = shader.makeMaterial(0.9, 0.9, 0.9, 0.8);

	tracer.hs1 = hs1;
	shader.hs1 = hs1;

	s2 = makeSphere(0.3, -0.1, -2.8, 0.15);
	s2->m = shader.makeMaterial(0.1, 1.0, 1.0, 0.4);

	tracer.s2 = s2;
	shader.s2 = s2;

	s3 = makeSphere(0.0, -0.180, -2.0, 0.1);
	s3->m = shader.makeMaterial(0.3, 0.4, 0.1, 0.0);

	tracer.s3 = s3;
	shader.s3 = s3;
}

void raytrace::initCamera(int w, int h)
{
	viewpoint = makePoint(0.0, 0.0, 0.0, 1.0);
	pnear = -1.0;
	fovx = M_PI / 6;
	/* window dimensions */
	width = w;
	height = h;
}

void raytrace::drawScene(void)
{
	int i, j;
	GLfloat imageWidth;

	/* declare data structures on stack to avoid malloc & free */
	point worldPix;  /* current pixel in world coordinates */
	point direction;
	ray r;
	color c;
	c.r = 0, c.g = 0, c.b = 0;
	/* z and w are constant for all pixels */
	worldPix.w = 1.0;
	worldPix.z = pnear;

	r.start = &worldPix;
	r.end = &direction;

	imageWidth = 2 * pnear * tan(fovx / 2);

	/* trace a ray for every pixel */
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {

			/* find position of pixel in world coordinates */
			/* y position = (pixel height/middle) scaled to world coords */
			worldPix.y = (j - (height / 2)) * imageWidth / width;
			/* x position = (pixel width/middle) scaled to world coords */
			worldPix.x = (i - (width / 2)) * imageWidth / width;
			
			/* find direction */
			/* note: direction vector is NOT NORMALIZED */
			// ray의 방향 결정
			calculateDirection(viewpoint, &worldPix, &direction);

			/* Find color for pixel */
			// ray를 쏨
			rayColor(&r, &c, 0);
			/* write the pixel! */
			lowlevel.drawPixel(i, j, c.r, c.g, c.b);
		}
	}
}

point* raytrace::makePoint(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	point* p;
	/* allocate memory */	
	p = new point();
	/* put stuff in it */
	p->x = x; p->y = y; p->z = z; p->w = w;
	return (p);
}

sphere* raytrace::makeSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat r) {
	sphere* s;
	/* allocate memory */
	s = new sphere();

	/* put stuff in it */
	s->c = makePoint(x, y, z, 1.0);   /* center */
	s->r = r;   /* radius */
	s->m = NULL;   /* material */
	return(s);
}

halfspace* raytrace::makeHalfspace(GLfloat x, GLfloat y, GLfloat z, GLfloat nx, GLfloat ny, GLfloat nz) {
	halfspace* hs;
	hs = new halfspace();

	hs->c = makePoint(x, y, z, 1.0);   /* center */
	hs->n = makePoint(nx, ny, nz, 1.0);   /* normal */
	hs->m = NULL;
	return(hs);
}

void reflect(ray* u, point* p, vector* n) {
	u->start = p;
	u->end->x = (u->end->x - 2 * (u->end->x * n->x + u->end->y * n->y + u->end->z * n->z) * n->x);
	u->end->y = (u->end->y - 2 * (-u->end->x * n->x + u->end->y * n->y + u->end->z * n->z) * n->y);
	u->end->z = (u->end->z - 2 * (-u->end->x * n->x + u->end->y * n->y + u->end->z * n->z) * n->z);	
	// std::cout << "Reflected: " << std::endl;
}

void refract(ray* u, point* p, vector* n, float refractive_index) {
	float cos_i = -(u->end->x * n->x + u->end->y * n->y + u->end->z * n->z);
	float cos_r = sqrt(1 - refractive_index * refractive_index * (1 - cos_i * cos_i));
	u->start = p;
	if (cos_i < 0) {
		n->x = -n->x;
		n->y = -n->y;
		n->z = -n->z;
		refractive_index = 1 / refractive_index;
	}
	u->end->x = (refractive_index * u->end->x - (cos_r - refractive_index * cos_i) * n->x);
	u->end->y = (refractive_index * u->end->y - (cos_r - refractive_index * cos_i) * n->y);
	u->end->z = (refractive_index * u->end->z - (cos_r - refractive_index * cos_i) * n->z);
}

/* returns the color seen by ray r in parameter c */
void raytrace::rayColor(ray* r, color* c, int step) {
	point p;  /* first intersection point */
	vector n;
	material* m;

	// reflect
	bool reflective = false;
	ray reflected_ray = *r;
	color reflected_color;
	reflected_color.r = 0.0, reflected_color.g = 0.0, reflected_color.b = 0.0;

	// refract
	bool refractive = true;
	ray refracted_ray = *r;
	color refracted_color;
	refracted_color.r = 0.0, refracted_color.g = 0.0, refracted_color.b = 0.0;
	point inner_p;

	p.w = 0.0;  /* inialize to "no intersection" */
	// ray가 만나는 물체와 물체의 normal을 계산하는 function 
	tracer.trace(r, &p, &n, &m, &reflective, &refractive);

	if (p.w != 0.0) {
		//shader.shade(&p, &n, m, c);  /* do the lighting calculations */
		if (step < 1 && reflective) {
			reflect(&reflected_ray, &p, &n);
			//reflected_ray.end->y -= 0.02;
			reflected_ray.end->z += 0.03;
			step += 1;
			rayColor(&reflected_ray, &reflected_color, step);
			shader.shade(&p, &n, m, c, &reflected_color, &refracted_color);
			return;
		}
		if (step < 1 && refractive) {
			refract(&refracted_ray, &p, &n, 0.9);
			//refracted_ray.end->z += 0.03;
			inner_p = p;
			tracer.trace(&refracted_ray, &inner_p, &n, &m, &reflective, &refractive);
			if (inner_p.w != 0.0 && refractive) {
				refract(&refracted_ray, &inner_p, &n, 0.9);
				step += 1;
				refracted_ray.end->x -= 0.15;
				rayColor(&refracted_ray, &refracted_color, step);
				shader.shade(&p, &n, m, c, &reflected_color, &refracted_color);
				return;
			}
			else {
				c->r = 0.0;
				c->g = 0.0;
				c->b = 0.0;
			}
		}
		shader.shade(&p, &n, m, c, &reflected_color, &refracted_color);
	}
	else {             /* nothing was hit */
		c->r = 0.0;
		c->g = 0.0;
		c->b = 0.0;
	}
}

/* vector from point p to point q is returned in v */
void raytrace::calculateDirection(point* p, point* q, point* v) {
	float length;

	v->x = q->x - p->x;
	v->y = q->y - p->y;
	v->z = q->z - p->z;
	/* a direction is a point at infinity */
	v->w = 0.0;
}
