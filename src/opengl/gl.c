/**
 * the stroke problem needs to be take care of.  the edge and the fill
 * can be different color in processing but not in opengl.
 *
 * need to revert y.
 */

#include <stdlib.h>
#include <errno.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "common.h"
#include "linux_list.h"

static struct processor_context *psr_context = NULL;

struct vertex {
    float x;
    float y;
    float z;
    struct llist_head list;
};

static LLIST_HEAD(vertex_list_head);

static int current_mode = -1;

static inline int glCheckError(void)
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

int init(struct processor_context *context)
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glScalef(1, -1, 1); /* flip y-axis to match with the processing
			 * coordinate */
    psr_context = context;
    return 0;
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

int push_matrix(void)
{
    glPushMatrix();
    return glCheckError();
}

int pop_matrix(void)
{
    glPopMatrix();
    return glCheckError();
}

int translate(float x, float y, float z)
{
    glTranslatef(x, y, z);
    return glCheckError();
}

int rotate(float angle, float x, float y, float z)
{
    glRotatef(angle, x, y, z);
    return glCheckError();
}

int scale(float x, float y, float z)
{
    glScalef(x, y, z);
    return glCheckError();
}

int begin_shape(int mode)
{
    switch (mode) {
    case POINTS:
	mode = GL_POINTS;
	break;
    case LINES:
	mode = GL_LINES;
	break;
    case TRIANGLES:
	mode = GL_TRIANGLES;
	break;
    case TRIANGLE_FAN:
	mode = GL_TRIANGLE_FAN;
	break;
    case TRIANGLE_STRIP:
	mode = GL_TRIANGLE_STRIP;
	break;
    case QUADS:
	mode = GL_QUADS;
	break;
    case QUAD_STRIP:
	mode = GL_QUAD_STRIP;
	break;
    case POLYGON:
	mode = GL_POLYGON;
	break;
    default:
	psr_system_error(EINVAL, "invalid 'mode' argument.");
    }
    current_mode = mode;
    glBegin(mode);
    return glCheckError();
}

/** don't care about texture yet. */
int vertex(float x, float y, float z, float u, float v)
{
    /* add it into our list first. */
    struct vertex *vertex = malloc(sizeof(struct vertex));
    if (!vertex) {
	psr_system_error(errno, "No memory for new vertex.");
    }
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    llist_add_tail(&vertex->list, &vertex_list_head);

    /* we only care about fill here. */
    if (!psr_context->fill) {
	return 0;
    }

    glVertex3f(x, y, z);
    return glCheckError();
}

int end_shape(int end_mode)
{
    int r = 0;
    int mode = current_mode;
    struct vertex *pos, *n;

    glEnd();
    if (!stroke) goto exit;

    /* draw the edges and free the list */
    if (mode == POLYGON) {
	if (end_mode == CLOSE) {
	    mode = GL_LINE_STRIP;
	} else {
	    mode = GL_LINE_LOOP;
	}
    }
    glBegin(mode);
    llist_for_each_entry_safe(pos, n, &vertex_list_head, list) {
	glVertex3f(pos->x, pos->y, pos->z);
	free(pos);
    }
    glEnd();

exit:
    return glCheckError();
}

int fill(float x, float y, float z, float a)
{
    glColor4f(x, y, z, a);
    return glCheckError();
}

