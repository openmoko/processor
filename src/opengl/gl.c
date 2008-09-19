#include <stdlib.h>
#include <errno.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "common.h"
#include "linux_list.h"

static struct psr_context *psr_cxt = NULL;

struct vertex {
    float x;
    float y;
    float z;
    struct llist_head list;
};

struct color_internal {
    float r;
    float g;
    float b;
    float a;
};

static struct color_internal stroke_color, fill_color;

static LLIST_HEAD(vertex_list_head);

static int glmode = -1;

static inline int glCheckError(void)
{
    GLenum e;
    int r = 0;
    psr_debug("glCheckError");
    while ((e = glGetError()) != GL_NO_ERROR) {
	r = 1;
	psr_warn("%s", gluErrorString(e));
    }
    return r;
}

static int stroke(float r, float g, float b, float a)
{
    stroke_color.r = r;
    stroke_color.g = g;
    stroke_color.b = b;
    stroke_color.a = a;
    return 0;
}

static int background(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return glCheckError();
}

static int push_matrix(void)
{
    glPushMatrix();
    return glCheckError();
}

static int pop_matrix(void)
{
    glPopMatrix();
    return glCheckError();
}

static int translate(float x, float y, float z)
{
    glTranslatef(x, y, z);
    return glCheckError();
}

static int rotate(float angle, float x, float y, float z)
{
    glRotatef(angle * 180 / M_PI, x, y, z);
    return glCheckError();
}

static int scale(float x, float y, float z)
{
    glScalef(x, y, z);
    /* FIXME: this doesn't consider the situation that we have
     * different scales on x, y, z. */
    glPointSize(x);
    glLineWidth(x);
    return glCheckError();
}

static int begin_shape(int mode)
{
    int r;
    switch (mode) {
    case POINTS:
	glmode = GL_POINTS;
	break;
    case LINES:
	glmode = GL_LINES;
	break;
    case TRIANGLES:
	glmode = GL_TRIANGLES;
	break;
    case TRIANGLE_FAN:
	glmode = GL_TRIANGLE_FAN;
	break;
    case TRIANGLE_STRIP:
	glmode = GL_TRIANGLE_STRIP;
	break;
    case QUADS:
	glmode = GL_QUADS;
	break;
    case QUAD_STRIP:
	glmode = GL_QUAD_STRIP;
	break;
    case POLYGON:
	glmode = GL_POLYGON;
	break;
    default:
	psr_system_error(EINVAL, "invalid 'mode' argument.");
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    r = glCheckError();
    /* well, don't call glGetError before glEnd */
    glBegin(glmode);
    return r;
}

/** don't care about texture yet. */
static int vertex(float x, float y, float z, float u, float v)
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
    if (!psr_cxt->fill) {
	return 0;
    }

    glVertex3f(x, y, z);
    return 0;
}

static int end_shape(int end_mode)
{
    struct vertex *pos, *n;

    glEnd();
    if (!psr_cxt->stroke) {
	/* FIXME: does not free */
	psr_debug("no stroke, don't draw edges.");
	goto exit;
    }

    switch (glmode) {
    case GL_POINTS:
    case GL_LINES:
	/* don't need to draw the edge.  already did. */
	goto exit;
    case GL_TRIANGLES:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLE_STRIP:
    case GL_QUADS:
    case GL_QUAD_STRIP:
	/* draw the edges */
	break;
    case GL_POLYGON:
	/* depends on CLOSE or not */
	if (end_mode == CLOSE) {
	    glmode = GL_LINE_STRIP;
	} else {
	    glmode = GL_LINE_LOOP;
	}
	break;
    default:
	break;
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(glmode);
    llist_for_each_entry_safe(pos, n, &vertex_list_head, list) {
	glVertex3f(pos->x, pos->y, pos->z);
	free(pos);
    }
    glEnd();

exit:
    return glCheckError();
}

static int fill(float r, float g, float b, float a)
{
    fill_color.r = r;
    fill_color.g = g;
    fill_color.b = b;
    fill_color.a = a;
    glColor4f(r, g, b, a);
    return 0;
}

int gl_init(struct psr_context *lpsr_cxt,
	    struct psr_renderer_context *renderer_cxt)
{
    int r = 0;
    glShadeModel(GL_SMOOTH);
    glClearColor(1, 1, 1, 1);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glFlush();
    r = glCheckError();

    psr_cxt = lpsr_cxt;
    renderer_cxt->stroke = stroke;
    renderer_cxt->background = background;
    renderer_cxt->push_matrix = push_matrix;
    renderer_cxt->pop_matrix = pop_matrix;
    renderer_cxt->translate = translate;
    renderer_cxt->rotate = rotate;
    renderer_cxt->scale = scale;
    renderer_cxt->begin_shape = begin_shape;
    renderer_cxt->vertex = vertex;
    renderer_cxt->end_shape = end_shape;
    renderer_cxt->fill = fill;

    return r;
}

int gl_reshape(int width, int height)
{
    glViewport(0, 0, width, height);    /* Reset The Current Viewport And Perspective Transformation */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    /* flip y-axis to match with the processing coordinate */
    glScalef(1, -1, 1);
    return glCheckError();
}
