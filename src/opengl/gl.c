#include <stdlib.h>
#include <errno.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <string.h>

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
    /* if not 0, it means this is a glList. */
    GLuint gl_list;
    struct color_internal stroke;
    struct color_internal fill;
    struct llist_head list;
};

static struct color_internal stroke_color, fill_color;

static LLIST_HEAD(vertex_list_head);

static int glmode = -1;
static int bezier_detail_level;
static int sphere_detail_level;
static GLUquadric *quad = NULL;
static int dont_fill = 0, dont_stroke = 0;
static GLuint recorded_list = 0;
static GLfloat saved_modelview[16];
static int g_width, g_height;
static GLdouble g_depth;

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

/******************************************************************** 
 * Shape functions
 ********************************************************************/

static int arc(float x, float y, float width, float height, float start,
	       float stop)
{
    float ratio = width / height;

    height = height / 2;	/* we need radius */

    /* set coordinates.  revert y again to get the angle right.
     * set ratio to get asymmetry disk */
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(ratio, -1, 1);

    if (!dont_fill) {
	glColor4f(fill_color.r, fill_color.g, fill_color.b, fill_color.a);
	gluQuadricDrawStyle(quad, GLU_FILL);
	/* FIXME: hardcode 30 */
	gluPartialDisk(quad, 0, height, 30, 1, start, stop - start);
    }

    if (!dont_stroke) {
	glColor4f(stroke_color.r, stroke_color.g, stroke_color.b,
		  stroke_color.a);
	gluQuadricDrawStyle(quad, GLU_SILHOUETTE);
	gluPartialDisk(quad, 0, height, 20, 1, start, stop - start);
    }

    glPopMatrix();		/* restore coordinates */
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
    vertex->gl_list = 0;	/* not a list */
    vertex->stroke = stroke_color;
    vertex->fill = fill_color;
    llist_add_tail(&vertex->list, &vertex_list_head);
    return 0;
}

static int bezier_detail(int level)
{
    bezier_detail_level = level;
    return 0;
}

static int bezier_vertex(float cx1, float cy1, float cz1,
			 float cx2, float cy2, float cz2,
			 float x, float y, float z)
{
    struct vertex *last_v, *v;
    GLfloat ctrlpoints[4][3];
    int i;

    /* get the last vertex */
    last_v = llist_entry(vertex_list_head.prev, struct vertex, list);
    if (!last_v) {
	/* if there is none, no way we can draw the curve */
	psr_error("Set at least one vertex before you call bezier_vertex");
	return -1;
    }
    v = malloc(sizeof(struct vertex));
    if (!v) {
	psr_system_error(errno, "No memory for new vertex.");
    }
    v->x = x;
    v->y = y;
    v->z = z;
    v->stroke = stroke_color;
    v->fill = fill_color;

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

    /* we create a list of this curve for later usage. */
    v->gl_list = glGenLists(1);
    if (v->gl_list == 0) {
	free(v);
	return glCheckError();
    }
    glNewList(v->gl_list, GL_COMPILE);
    for (i = 0; i <= bezier_detail_level; ++i) {
	glEvalCoord1f((GLfloat) i / bezier_detail_level);
    }
    glEndList();
    llist_add_tail(&v->list, &vertex_list_head);

    return glCheckError();
}

static int end_shape(int end_mode)
{
    struct vertex *pos, *n;

    /* we fill the shape first, then draw the edges */

    /* do fill */
    switch (glmode) {
    case GL_POINTS:
    case GL_LINES:
	break;
    default:
	if (dont_fill) {
	    break;
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(glmode);
	llist_for_each_entry(pos, &vertex_list_head, list) {
	    glColor4f(pos->fill.r, pos->fill.g, pos->fill.b, pos->fill.a);
	    if (pos->gl_list) {
		glCallList(pos->gl_list);
	    } else {
		glVertex3f(pos->x, pos->y, pos->z);
		psr_debug
		    ("fill point: (%f, %f, %f) color: (%f, %f, %f, %f)",
		     pos->x, pos->y, pos->z, pos->fill.r, pos->fill.g,
		     pos->fill.b, pos->fill.a);
	    }
	}
	glEnd();
    }
    if (glmode == GL_POLYGON) {
	/* depends on CLOSE or not */
	if (end_mode == OPEN) {
	    glmode = GL_LINE_STRIP;
	} else {
	    glmode = GL_LINE_LOOP;
	}
    }

    /* do stroke */
    if (!dont_stroke) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(glmode);
	llist_for_each_entry(pos, &vertex_list_head, list) {
	    glColor4f(pos->stroke.r, pos->stroke.g, pos->stroke.b,
		      pos->stroke.a);
	    if (pos->gl_list) {
		glCallList(pos->gl_list);
	    } else {
		glVertex3f(pos->x, pos->y, pos->z);
		psr_debug
		    ("stroke point: (%f, %f, %f) color: (%f, %f, %f, %f)",
		     pos->x, pos->y, pos->z, pos->stroke.r, pos->stroke.g,
		     pos->stroke.b, pos->stroke.a);
	    }
	}
	glEnd();
    }

    /* release the resource */
    llist_for_each_entry_safe(pos, n, &vertex_list_head, list) {
	if (pos->gl_list) {
	    glDeleteLists(pos->gl_list, 1);
	}
	llist_del(&pos->list);
	free(pos);
    }
    return glCheckError();
}

static int box(float width, float height, float depth)
{
    const float w = width/2, h = height/2, d = depth/2;

    if (!dont_fill) {
	glColor4f(fill_color.r, fill_color.g, fill_color.b, fill_color.a);
	glBegin(GL_QUADS);
	/* back */
	glVertex3f(-w, -h, -d);
	glVertex3f(-w,  h, -d);
	glVertex3f( w,  h, -d);
	glVertex3f( w, -h, -d);
	/* left */
	glVertex3f(-w, -h, -d);
	glVertex3f(-w, -h,  d);
	glVertex3f(-w,  h,  d);
	glVertex3f(-w,  h, -d);
	/* right */
	glVertex3f( w, -h, -d);
	glVertex3f( w, -h,  d);
	glVertex3f( w,  h,  d);
	glVertex3f( w,  h, -d);
	/* front */
	glVertex3f(-w, -h,  d);
	glVertex3f(-w,  h,  d);
	glVertex3f( w,  h,  d);
	glVertex3f( w, -h,  d);
	/* top */
	glVertex3f(-w, -h, -d);
	glVertex3f(-w, -h,  d);
	glVertex3f( w, -h,  d);
	glVertex3f( w, -h, -d);
	/* bottom */
	glVertex3f(-w,  h, -d);
	glVertex3f(-w,  h,  d);
	glVertex3f( w,  h,  d);
	glVertex3f( w,  h, -d);
	glEnd();
    }
    if (!dont_stroke) {
	glColor4f(stroke_color.r, stroke_color.g, stroke_color.b, stroke_color.a);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_LINE_LOOP);
	glVertex3f(-w, -h, -d);
	glVertex3f( w, -h, -d);
	glVertex3f( w,  h, -d);
	glVertex3f(-w,  h, -d);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(-w, -h,  d);
	glVertex3f( w, -h,  d);
	glVertex3f( w,  h,  d);
	glVertex3f(-w,  h,  d);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-w, -h,  d);
	glVertex3f(-w, -h, -d);
	glVertex3f( w, -h,  d);
	glVertex3f( w, -h, -d);
	glVertex3f( w,  h,  d);
	glVertex3f( w,  h, -d);
	glVertex3f(-w,  h,  d);
	glVertex3f(-w,  h, -d);
	glEnd();
	glEnable(GL_DEPTH_TEST);
    }
    return glCheckError();
}

static int sphere(float radius)
{
    if (!dont_fill) {
	glColor4f(fill_color.r, fill_color.g, fill_color.b, fill_color.a);
	gluQuadricDrawStyle(quad, GLU_FILL);
	gluSphere(quad, radius, sphere_detail_level, sphere_detail_level);
    }
    return 0;
}

static int sphere_detail(int n)
{
    sphere_detail_level = n;
    return 0;
}

static int stroke_weight(float width)
{
    glPointSize(width);
    glLineWidth(width);
    return glCheckError();
}

static int smooth(void)
{
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    return glCheckError();
}

static int no_smooth(void)
{
    glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    return glCheckError();
}


/******************************************************************** 
 * Output functions
 ********************************************************************/

/** notice: since we don't deal with file format here, we return the
 * memory block for the upper level to handle.  upper level must free
 * this block of memory. */
static int save(struct psr_image *img)
{
    int r;
    void *saved_image = malloc(sizeof(GLubyte) * 3 * g_width * g_height);
    glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE,
		 saved_image);
    r = glCheckError();
    if (r) {
	/* something wrong */
	free(saved_image);
	return r;
    }
    img->width = g_width;
    img->height = g_height;
    img->data = saved_image;
//    FILE *fp = fopen("in", "w");
//    psr_debug("wrote: %d", (int) fwrite(img->data, sizeof(GLubyte) * 3,
//					img->width * img->height, fp));
//    fclose(fp);
    return 0;
}


/******************************************************************** 
 * Transform functions
 ********************************************************************/

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
    return glCheckError();
}

static int print_matrix(void)
{
    GLfloat matrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) matrix);
    printf("%10.4f, %10.4f, %10.4f, %10.4f, \n"
	   "%10.4f, %10.4f, %10.4f, %10.4f, \n"
	   "%10.4f, %10.4f, %10.4f, %10.4f, \n"
	   "%10.4f, %10.4f, %10.4f, %10.4f, \n",
	   matrix[0], matrix[4], matrix[8], matrix[12],
	   -matrix[1], -matrix[5], -matrix[9], -matrix[13],
	   matrix[2], matrix[6], matrix[10], matrix[14],
	   matrix[3], matrix[7], matrix[11], matrix[15]);
    return 0;
}

static int apply_matrix(float n11, float n12, float n13, float n14,
			float n21, float n22, float n23, float n24,
			float n31, float n32, float n33, float n34,
			float n41, float n42, float n43, float n44)
{
    GLfloat matrix[] = {n11, n12, n13, n14,
			n21, n22, n23, n24,
			n31, n32, n33, n34,
			n41, n42, n43, n44};

    glMultMatrixf((GLfloat *) matrix);
    return glCheckError();
}

static int reset_matrix(void)
{
    glLoadIdentity();
    return glCheckError();
}


/******************************************************************** 
 * Color functions
 ********************************************************************/

static int stroke(float r, float g, float b, float a)
{
    dont_stroke = 0;
    stroke_color.r = r;
    stroke_color.g = g;
    stroke_color.b = b;
    stroke_color.a = a;
    return 0;
}

static int no_stroke(void)
{
    dont_stroke = 1;
    return 0;
}

static int fill(float r, float g, float b, float a)
{
    dont_fill = 0;
    fill_color.r = r;
    fill_color.g = g;
    fill_color.b = b;
    fill_color.a = a;
    return 0;
}

static int no_fill(void)
{
    dont_fill = 1;
    return 0;
}

static int background(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return glCheckError();
}


/******************************************************************** 
 * Image functions
 ********************************************************************/

static int image(struct psr_image *img, float x, float y, float width,
		 float height)
{
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, g_width, 0, g_height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (width == 0 || height == 0) {
	/* don't resize */
	glRasterPos2f(x, y);
	glDrawPixels(img->width, img->height, GL_RGB, GL_UNSIGNED_BYTE,
		     img->data);
    } else {
	glRasterPos2f(x, y);
	glPixelZoom(width/img->width, height/img->height);
	glDrawPixels(img->width, img->height, GL_RGB, GL_UNSIGNED_BYTE,
		     img->data);
    }
    glPopMatrix();
    return glCheckError();
}


/******************************************************************** 
 * Lights and camera functions
 ********************************************************************/

static int camera_default(void)
{
    glLoadIdentity();
    /* flip y-axis to match with the processing coordinate */
    glScalef(1, -1, 1);
    /* adjust the window to the correct position because the camera
     * sits at the origin.  tricky. */
    glTranslatef(-g_width/2, -g_height/2, -g_depth);
    return glCheckError();
}

static int camera(float eye_x, float eye_y, float eye_z,
		  float center_x, float center_y, float center_z,
		  float up_x, float up_y, float up_z)
{
    glLoadIdentity();
    gluLookAt(eye_x, -eye_y, eye_z,
	      center_x, -center_y, center_z,
	      up_x, up_y, up_z);
    glScalef(1, -1, 1);
    return glCheckError();
}

static int begin_camera(void)
{
    /* save the current modelview matrix */
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) saved_modelview);
    glLoadIdentity();
    /* operation to the camera should be reverted to applied to the
     * model view. */
    glScalef(-1, -1, -1);
    return glCheckError();
}

static int end_camera(void)
{
    GLfloat camera_view[16];
    /* invert y offset */
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) camera_view);
    camera_view[13] = -camera_view[13];
    glLoadMatrixf((GLfloat *) camera_view);
    /* revert it again to go back to the original scale */
    glScalef(-1, -1, -1);
    glMultMatrixf((GLfloat *) saved_modelview);
    return glCheckError();
}

static int ortho(float left, float right, float bottom, float top,
		 float near, float far)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(left, right, -bottom, -top, near, far);
    glMatrixMode(GL_MODELVIEW);
    return glCheckError();
}


/******************************************************************** 
 * Other functions
 ********************************************************************/

static GLvoid glu_error_handle(GLenum e)
{
    psr_error("gluQuadric error: %s", gluErrorString(e));
}

int gl_init(struct psr_context *lpsr_cxt,
	    struct psr_renderer_context *renderer_cxt)
{
    int r = 0;

    psr_debug("gl_init");

    /* Note: refer to the mesa performance tips */

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_MAP1_VERTEX_3);
    glFlush();
    r = glCheckError();

    quad = gluNewQuadric();
    gluQuadricCallback(quad, GLU_ERROR, glu_error_handle);
    gluQuadricNormals(quad, GLU_NONE);

    psr_cxt = lpsr_cxt;
    renderer_cxt->stroke = stroke;
    renderer_cxt->no_stroke = no_stroke;
    renderer_cxt->background = background;
    renderer_cxt->push_matrix = push_matrix;
    renderer_cxt->pop_matrix = pop_matrix;
    renderer_cxt->translate = translate;
    renderer_cxt->rotate = rotate;
    renderer_cxt->scale = scale;
    renderer_cxt->begin_shape = begin_shape;
    renderer_cxt->vertex = vertex;
    renderer_cxt->end_shape = end_shape;
    renderer_cxt->arc = arc;
    renderer_cxt->bezier_detail = bezier_detail;
    renderer_cxt->bezier_vertex = bezier_vertex;
    renderer_cxt->box = box;
    renderer_cxt->sphere = sphere;
    renderer_cxt->sphere_detail = sphere_detail;
    renderer_cxt->stroke_weight = stroke_weight;
    renderer_cxt->smooth = smooth;
    renderer_cxt->no_smooth = no_smooth;
    renderer_cxt->fill = fill;
    renderer_cxt->no_fill = no_fill;
    renderer_cxt->save = save;
    renderer_cxt->image = image;
    renderer_cxt->apply_matrix = apply_matrix;
    renderer_cxt->reset_matrix = reset_matrix;
    renderer_cxt->print_matrix = print_matrix;
    renderer_cxt->camera_default = camera_default;
    renderer_cxt->camera = camera;
    renderer_cxt->begin_camera = begin_camera;
    renderer_cxt->end_camera = end_camera;
    renderer_cxt->ortho = ortho;

    return r;
}

int gl_end(void)
{
    gluDeleteQuadric(quad);
    quad = NULL;
    if (recorded_list) {
	glDeleteLists(recorded_list, 1);
    }
    return 0;
}

int gl_reshape(int width, int height)
{
    const GLdouble fov = 60;
    const GLdouble aspect = width / height;
    const GLdouble z = height / 2 / 0.577350269;	/* tan(30 deg) */
    const GLdouble z_near = z / 10;
    const GLdouble z_far = z * 10;

    psr_debug("gl_reshape(%d, %d), aspect %f, z_near %f, z_far %f",
	      width, height, aspect, z_near, z_far);

    g_width = width;
    g_height = height;
    g_depth = z;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, z_near, z_far);

    glMatrixMode(GL_MODELVIEW);
    camera_default();

    return glCheckError();
}

int gl_record(void (*func) (void))
{
    if (recorded_list) {
	glDeleteLists(recorded_list, 1);
    }
    recorded_list = glGenLists(1);
    if (recorded_list == 0) {
	return glCheckError();
    }
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) saved_modelview);
    glNewList(recorded_list, GL_COMPILE_AND_EXECUTE);
    func();			/* do the actual drawing */
    glEndList();
    return glCheckError();
}

int gl_replay(void)
{
    if (recorded_list) {
	glPushMatrix();
	glLoadMatrixf((GLfloat *) saved_modelview);
	glCallList(recorded_list);
	glPopMatrix();
    }
    return glCheckError();
}
