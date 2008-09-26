#include <stdlib.h>
#include <errno.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "psr_internal.h"
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
    struct color_internal stroke;
    struct color_internal fill;
    struct llist_head list;
};

static struct color_internal stroke_color, fill_color;

static LLIST_HEAD(vertex_list_head);

static int glmode = -1;

#define glCheckError()					\
    ({							\
	GLenum e;					\
	int r = 0;					\
	while ((e = glGetError()) != GL_NO_ERROR) {	\
	    r = -1;					\
	    psr_error("%s", gluErrorString(e));		\
	}						\
	r;						\
    })

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
	psr_error("invalid 'mode' argument.");
	return -1;
    }
    return 0;
}

/** FIXME: don't care about texture yet. */
static int vertex(float x, float y, float z, float u, float v)
{
    struct vertex *vertex = malloc(sizeof(struct vertex));
    if (!vertex) {
	psr_system_error(errno, "No memory for new vertex.");
    }
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    vertex->stroke = stroke_color;
    vertex->fill = fill_color;
    llist_add_tail(&vertex->list, &vertex_list_head);
    return 0;
}

static int bezier_vertex(float cx1, float cy1, float cz1,
			 float cx2, float cy2, float cz2,
			 float x, float y, float z)
{
    struct vertex *last_v;
    GLfloat ctrlpoints[4][3];

    /* get the last vertex */
    last_v = llist_entry(vertex_list_head.prev, struct vertex, list);
    if (!last_v) {
	/* if there is none, no way we can draw the curve */
	psr_error("Set at least one vertex before you call bezier_vertex");
	return -1;
    }
    ctrlpoints[0][0] = last_v->x;
    ctrlpoints[0][1] = last_v->y;
    ctrlpoints[0][2] = last_v->z;
    ctrlpoints[1][0] = cx1;
    ctrlpoints[1][1] = cy1;
    ctrlpoints[1][2] = cz1;
    ctrlpoints[2][0] = cx2;
    ctrlpoints[2][1] = cy2;
    ctrlpoints[2][2] = cz2;
    ctrlpoints[3][0] = x;
    ctrlpoints[3][1] = y;
    ctrlpoints[3][2] = z;
    glMap1f(GL_MAP1_VERTEX_3, 0, 1, 3, 4, (GLfloat *) ctrlpoints);
    glEnable(GL_MAP1_VERTEX_3);
    return 0;
}


static int end_shape(int end_mode)
{
    struct llist_head *pos, *n;

    /* we fill the shape first, then draw the edges */

    /* do fill */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(glmode);
    llist_for_each(pos, &vertex_list_head) {
	struct vertex *v = llist_entry(pos, struct vertex, list);
	psr_debug("fill color: %f, %f, %f, %f\n point: %f, %f, %f\n",
		  v->fill.r, v->fill.g, v->fill.b, v->fill.a,
		  v->x, v->y, v->z);
	glColor4f(v->fill.r, v->fill.g, v->fill.b, v->fill.a);
	glVertex3f(v->x, v->y, v->z);
    }
    glEnd();

    switch (glmode) {
    case GL_POLYGON:
	/* depends on CLOSE or not */
	if (end_mode == CLOSE) {
	    glmode = GL_LINE_STRIP;
	} else {
	    glmode = GL_LINE_LOOP;
	}
    case GL_POINTS:
    case GL_LINES:
    case GL_TRIANGLES:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLE_STRIP:
    case GL_QUADS:
    case GL_QUAD_STRIP:
	break;
    default:
	psr_error("Invalid internal variable: glmode (%d)."
		  "  Something is very wrong.", glmode);
	return -1;
    }

    /* now draw the edges */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(glmode);
    llist_for_each_safe(pos, n, &vertex_list_head) {
	struct vertex *v = llist_entry(pos, struct vertex, list);
	psr_debug("stroke color: %f, %f, %f, %f\n point: %f, %f, %f\n",
		  v->stroke.r, v->stroke.g, v->stroke.b, v->stroke.a,
		  v->x, v->y, v->z);
	glColor4f(v->stroke.r, v->stroke.g, v->stroke.b, v->stroke.a);
	glVertex3f(v->x, v->y, v->z);
	llist_del(pos);
	free(v);
    }
    glEnd();
    return glCheckError();
}

static GLvoid glu_error_handle(GLenum e)
{
    psr_error("gluQuadric error: %s", gluErrorString(e));
}

static int arc(float x, float y, float width, float height, float start,
	       float stop)
{
    GLUquadricObj *qobj = gluNewQuadric();
    float ratio = width / height;

    gluQuadricCallback(qobj, GLU_ERROR, glu_error_handle);
    height = height / 2; /* we need radius */

    gluQuadricNormals(qobj, GLU_NONE);

    /* set coordinates.  revert y again to get the angle right.
     * set ratio to get asymmetry disk */
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(ratio, -1, 1);

    /* first pass, fill */
    gluQuadricDrawStyle(qobj, GLU_FILL);
    /* FIXME: should set the slices (20) to something that matchs the
     * size of the arc. */
    gluPartialDisk(qobj, 0, height, 20, 1, start, stop - start);

    /* second pass, stroke */
    glColor4f(stroke_color.r, stroke_color.g, stroke_color.b, stroke_color.a);
    gluQuadricDrawStyle(qobj, GLU_LINE);
    gluPartialDisk(qobj, 0, height, 20, 1, start, stop - start);

    glPopMatrix();  /* restore coordinates */
    gluDeleteQuadric(qobj);
    /* restore color */
    glColor4f(fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    return glCheckError();
}

static int fill(float r, float g, float b, float a)
{
    fill_color.r = r;
    fill_color.g = g;
    fill_color.b = b;
    fill_color.a = a;
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
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
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
    renderer_cxt->arc = arc;

    return r;
}

int gl_reshape(int width, int height)
{
    const GLdouble fov = 60;
    const GLdouble aspect = width / height;
    const GLdouble z = height / 2 / 0.577350269;	/* tan(30 deg) */
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
    glTranslatef(-width / 2, -height / 2, -z);

    return glCheckError();
}
