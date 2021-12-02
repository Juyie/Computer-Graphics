#pragma once

#include <glad/glad.h>
#include <stdlib.h> 

#define RED 0    /* offset to red byte */
#define GREEN 1    /* offset to green byte */
#define BLUE 2    /* offset to blue byte */

class lowlevel {
public:
	lowlevel();
	~lowlevel();

	/* lowlevel drawing funtions */
	void initCanvas(int w , int h);
	void drawPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b);
	GLubyte* flushCanvas(void);
	
private:
	/* pointer to image */
	GLubyte* canvas;
	int width;     /* width of window in pixels */
	int height;    /* height of window in pixels */
};
