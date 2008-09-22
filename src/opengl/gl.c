#include <stdlib.h>
#include <errno.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "common.h"
#include "linux_list.h"

static struct psr_context *psr_cxt = NULL;

struct color_internal {
    float r;
    float g;
    float b;
    float a;
};

struct vertex {
    float x;
    float y;
    float z;
    struct color_internal color;
    struct llist_head list;
};

static struct color_internal stroke_color;

static LLIST_HEAD(vertex_list_head);

static int glmode = -1;

static inline int glCheckError(void)
{
    GLenum e;
    int r = 0;
    psr_debug("glCheckError");
    while ((e = glGetError()) != GL_NO_ERROR) {
	r = 1;
	psr_error("%s", gluErrorString(e));
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
    /* the idea is we draw the inside of the shape first, then we draw
     * the edges of the shape. */
    int r = 0;
    switch (mode) {
    case POINTS:
	glmode = GL_POINTS;
	goto exit_no_begin;
    case LINES:
	glmode = GL_LINES;
	goto exit_no_begin;
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
	psr_error("invalid 'mode' argument.");
	return 1;
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    r = glCheckError();
    /* well, don't call glGetError before glEnd */
    glBegin(glmode);

exit_no_begin:
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
    /* remember what stroke color we use for this vertex.  we need
     * this later. */
    vertex->color = stroke_color;
    llist_add_tail(&vertex->list, &vertex_list_head);
    if (glmode != GL_POINTS && glmode != GL_LINES) {
	glVertex3f(x, y, z);
    }
    return 0;
}

static int end_shape(int end_mode)
{
    struct llist_head *pos, *n;

    switch (glmode) {
    case GL_POINTS:
    case GL_LINES:
	break;
    case GL_POLYGON:
	/* depends on CLOSE or not */
	if (end_mode == CLOSE) {
	    glmode = GL_LINE_STRIP;
	} else {
	    glmode = GL_LINE_LOOP;
	}
    case GL_TRIANGLES:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLE_STRIP:
    case GL_QUADS:
    case GL_QUAD_STRIP:
	/* okay for these shapes we did glBegin() so now we have to
	 * glEnd() it. */
	glEnd();
	break;
    default:
	psr_error("Invalid internal variable: glmode (%d)."
		  "  Something is very wrong.", glmode);
	return 1;
    }
    /* now draw the edges */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(glmode);
    llist_for_each_safe(pos, n, &vertex_list_head) {
	struct vertex *v = llist_entry(pos, struct vertex, list);
	psr_debug("color: %f, %f, %f, %f\n point: %f, %f, %f\n",
		  v->color.r, v->color.g, v->color.b, v->color.a,
		  v->x, v->y, v->z);
	glColor4f(v->color.r, v->color.g, v->color.b, v->color.a);
	glVertex3f(v->x, v->y, v->z);
	llist_del(pos);
	free(v);
    }
    glEnd();
    return glCheckError();
}

static int fill(float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    return 0;
}

int gl_init(struct psr_context *lpsr_cxt,
	    struct psr_renderer_context *renderer_cxt)
{
    int r = 0;
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
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
    const GLdouble fov = 60;
    const GLdouble aspect = width / height;
    const GLdouble z = height / 2 / 0.577350269; /* tan(30 deg) */
    const GLdouble z_near = z / 10;
    const GLdouble z_far = z * 10;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, z_near, z_far);
    //glOrtho(-width/2, width/2, -height/2, height/2, -1000, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    /* flip y-axis to match with the processing coordinate */
    glScalef(1, -1, 1);
    /* adjust the window to the correct position because the camera
     * sits at the origin.  tricky. */
    glTranslatef(-width/2, -height/2, -z);

    return glCheckError();
}
