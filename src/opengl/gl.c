#include <GL/gl.h>
#include <GL/glu.h>

#include "util.h"

static void glCheckError(void)
{
    GLenum e;
    int r = 0;
    e = glGetError();
    while (e != GL_NO_ERROR) {
	r = 1;
	psr_error("%s\n", gluErrorString(e));
    }
    return r;
}


int init(void)
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int stroke(float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    return glCheckError();
}

int background(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return glCheckError();
}

int push_matrix()
{
    return 0;
}
