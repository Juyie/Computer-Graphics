#include "lowlevel.h"

lowlevel::lowlevel() :
	canvas(NULL),
	width(0),
	height(0)
{
}

lowlevel::~lowlevel()
{
	if (canvas != NULL) delete canvas;
}

void lowlevel::initCanvas(int w , int h)
{
	int i;

	/* get memory */	
	int cavasSize = w * h * 3;
	canvas = new GLubyte[cavasSize]();

	/* clear it */
	for (i = 0; i < w * h * 3; i++) {
		canvas[i] = 0xFF;
	}
	width = w;
	height = h;
}

void lowlevel::drawPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b)
{
	if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) return;
	canvas[3 * width * (y)+3 * (x)+RED] = r * 255;
	canvas[3 * width * (y)+3 * (x)+GREEN] = g * 255;
	canvas[3 * width * (y)+3 * (x)+BLUE] = b * 255;
}

GLubyte* lowlevel::flushCanvas(void)
{
	return canvas;
}
