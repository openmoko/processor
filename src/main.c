#include <limits.h>
#include <dlfcn.h>
#include <errno.h>
#include <time.h>

#include "psr_internal.h"

/* Force a compilation error if condition is true */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

struct psr_context psr_context;
struct psr_renderer_context renderer_context;

//volatile int debug_level = 1 << 3 | 1 << 2;
volatile int debug_level = 15;

volatile char key;
volatile int keycode;
volatile int mouse_x;
volatile int mouse_y;
volatile int mouse_button;
volatile int p_mouse_x;
volatile int p_mouse_y;
volatile int width;
volatile int height;

static int g_rect_mode;
static int g_ellipse_mode;

static int (*main_loop_start) (void);

static int load_renderer(const char *libpath)
{
    int (*renderer_init) (struct psr_context *psr_cxt,
			  struct psr_renderer_context *renderer_cxt);
    void *handle;
    char *errstr;

    handle = dlopen(libpath, RTLD_LAZY);
    if (!handle) {
	goto error_exit;
    }
    renderer_init = dlsym(handle, "init");
    if (!renderer_init) {
	goto error_exit;
    }
    main_loop_start = dlsym(handle, "main_loop_start");
    if (!main_loop_start) {
	goto error_exit;
    }
    return renderer_init(&psr_context, &renderer_context);

error_exit:
    errstr = dlerror();
    if (errstr) {
	psr_error("%s", errstr);
    }
    if (handle) {
	dlclose(handle);
    }
    return -1;
}

static void update_key(int lkey, int lkeycode)
{
    key = lkey;
    if (lkeycode != NONE) {
	keycode = lkeycode;
    }
}

static void update_mouse(int x, int y, int button)
{
    p_mouse_x = mouse_x;
    mouse_x = x;
    p_mouse_y = mouse_y;
    mouse_y = y;
    if (button != NONE) {
	mouse_button = button;
    }
}

static void update_size(int lwidth, int lheight)
{
    width = lwidth;
    height = lheight;
}

int size(int lwidth, int lheight)
{
    psr_debug("size(%d, %d)", lwidth, lheight);
    width = lwidth;
    height = lheight;
    return renderer_context.size(width, height);
}

int no_loop(void)
{
    psr_debug("no_loop()");
    return renderer_context.no_loop();
}

int loop(void)
{
    psr_debug("loop()");
    return renderer_context.loop();
}

int redraw(void)
{
    psr_debug("redraw()");
    return renderer_context.redraw();
}

int delay(int milliseconds)
{
    int r;
    struct timespec requested_time, remaining;
    psr_debug("delay(%d)", milliseconds);
    requested_time.tv_sec = milliseconds / 1000000;
    requested_time.tv_nsec = (long int) milliseconds % 1000000 * 1000;
    r = nanosleep(&requested_time, &remaining);
    if (r) {
	psr_system_warn(errno, "nanosleep error return.");
    }
    return r;
}

int frame_rate(float framerate)
{
    psr_debug("frame_rate(%f)", framerate);
    return renderer_context.frame_rate(framerate);
}

int cursor(int type)
{
    psr_debug("cursor(%d)", type);
    return renderer_context.cursor(type);
}

int no_cursor(void)
{
    psr_debug("no_cursor()");
    return renderer_context.cursor(NONE);
}

const char *binary(int i)
{
    static char buf[1024];  /* supports up to 1024 bits */
    char *p = buf;
    int len = sizeof(i) * 8;  /* length in bits */
    BUILD_BUG_ON(len >= 1024);
    while(len > 0) {
	*p++ = i >> --len & 1 ? '1' : '0';
    }
    *p = 0;
    return buf;
}

int unbinary(const char *s)
{
    long int i;
    i = strtol(s, NULL, 2);
    if (((i == LONG_MIN || i == LONG_MAX) && errno == ERANGE)
	|| (i < INT_MIN || i > INT_MAX)) {
	psr_system_error(ERANGE, "unbinary(%s) out of range", s);
	return -1;
    }
    return (int) i;
}

const char *hex(int i)
{
    static char buf[256];  /* supports up to 1024 bits */
    BUILD_BUG_ON(sizeof(i) > (1024 / 8));
    snprintf(buf, 256, "%X", i);
    return buf;
}

int unhex(const char *s)
{
    long int i;
    i = strtol(s, NULL, 16);
    if (((i == LONG_MIN || i == LONG_MAX) && errno == ERANGE)
	|| (i < INT_MIN || i > INT_MAX)) {
	psr_system_error(ERANGE, "unhex(%s) out of range", s);
	return -1;
    }
    return (int) i;
}

int stroke(float r, float g, float b, float a)
{
    psr_debug("stroke(%f, %f, %f, %f)", r, g, b, a);
    return renderer_context.stroke(r, g, b, a);
}

int no_stroke(void)
{
    psr_debug("no_stroke()");
    return renderer_context.no_stroke();
}

int background(float r, float g, float b, float a)
{
    psr_debug("background(%f, %f, %f, %f)", r, g, b, a);
    return renderer_context.background(r, g, b, a);
}

int push_matrix(void)
{
    psr_debug("push_matrix()");
    return renderer_context.push_matrix();
}

int pop_matrix(void)
{
    psr_debug("pop_matrix()");
    return renderer_context.pop_matrix();
}

int apply_matrix(float n11, float n12, float n13, float n14,
		 float n21, float n22, float n23, float n24,
		 float n31, float n32, float n33, float n34,
		 float n41, float n42, float n43, float n44)
{
    psr_debug("apply_matrix(%f, %f, %f, %f, %f, %f, %f, %f"
	      ", %f, %f, %f, %f, %f, %f, %f, %f",
	      n11, n12, n13, n14, n21, n22, n23, n24,
	      n31, n32, n33, n34, n41, n42, n43, n44);
    return renderer_context.apply_matrix(
	n11, n12, n13, n14, n21, n22, n23, n24,
	n31, n32, n33, n34, n41, n42, n43, n44);
}

int reset_matrix(void)
{
    psr_debug("reset_matrix()");
    return renderer_context.reset_matrix();
}

int print_matrix(void)
{
    psr_debug("print_matrix()");
    return renderer_context.print_matrix();
}

int translate(float x, float y, float z)
{
    psr_debug("translate(%f, %f, %f)", x, y, z);
    return renderer_context.translate(x, y, z);
}

int rotate(float angle, float x, float y, float z)
{
    psr_debug("rotate(%f, %f, %f, %f)", angle, x, y, z);
    return renderer_context.rotate(angle, x, y, z);
}

int rotate_x(float angle)
{
    psr_debug("rotate_x(%f)", angle);
    return rotate(angle, 1.0, 0, 0);
}

int rotate_y(float angle)
{
    psr_debug("rotate_y(%f)", angle);
    return rotate(angle, 0, 1.0, 0);
}

int rotate_z(float angle)
{
    psr_debug("rotate_z(%f)", angle);
    return rotate(angle, 0, 0, 1.0);
}

int scale(float x, float y, float z)
{
    psr_debug("scale(%f, %f, %f)", x, y, z);
    return renderer_context.scale(x, y, z);
}

int begin_shape(int mode)
{
    psr_debug("begin_shape(%d)", mode);
    return renderer_context.begin_shape(mode);
}

int vertex(float x, float y, float z, float u, float v)
{
    psr_debug("vertex(%f, %f, %f, %f, %f)", x, y, z, u, v);
    return renderer_context.vertex(x, y, z, u, v);
}

int end_shape(int end_mode)
{
    psr_debug("end_shape(%d)", end_mode);
    return renderer_context.end_shape(end_mode);
}

int triangle(float x1, float y1,
	     float x2, float y2,
	     float x3, float y3)
{
    psr_debug("triangle(%f, %f, %f, %f, %f, %f)", x1, y1, x2, y2, x3, y3);
    begin_shape(TRIANGLES);
    vertex(x1, y1, 0, 0, 0);
    vertex(x2, y2, 0, 0, 0);
    vertex(x3, y3, 0, 0, 0);
    return end_shape(CLOSE);
}

int line(float x1, float y1, float z1,
	 float x2, float y2, float z2)
{
    psr_debug("line(%f, %f, %f, %f, %f, %f)", x1, y1, z1, x2, y2, z2);
    begin_shape(LINES);
    vertex(x1, y1, z1, 0, 0);
    vertex(x2, y2, z2, 0, 0);
    return end_shape(CLOSE);
}

int arc(float x, float y, float width, float height, float start,
	float stop)
{
    psr_debug("arc(%f, %f, %f, %f, %f, %f)", x, y, width, height, start, stop);
    switch(g_ellipse_mode) {
    case CENTER:
	break;
    case RADIUS:
	width = width * 2;
	height = height * 2;
	break;
    case CORNER:
	x = x + width / 2;
	y = y + height / 2;
	break;
    case CORNERS:
	x = (x + width) / 2;
	y = (y + height) / 2;
	width = fabsf(x - width);
	height = fabsf(y - height);
	break;
    default:
	psr_error("invalid g_ellipse_mode.  this should not happen");
	return -1;
    }
    return renderer_context.arc(x, y, width, height, start, stop);
}

int point(float x, float y, float z)
{
    psr_debug("point(%f, %f, %f)", x, y, z);
    begin_shape(POINTS);
    vertex(x, y, z, 0, 0);
    return end_shape(CLOSE);
}

int quad(float x1, float y1,
	 float x2, float y2,
	 float x3, float y3,
	 float x4, float y4)
{
    psr_debug("quad(%f, %f, %f, %f, %f, %f, %f, %f)", x1, y1, x2, y2, x3, y3, x4, y4);
    begin_shape(QUADS);
    vertex(x1, y1, 0, 0, 0);
    vertex(x2, y2, 0, 0, 0);
    vertex(x3, y3, 0, 0, 0);
    vertex(x4, y4, 0, 0, 0);
    return end_shape(CLOSE);
}

int ellipse(float x, float y, float width, float height)
{
    /* FIXME: room for improvement.  maybe don't call arc */
    psr_debug("ellipse(%f, %f, %f, %f)", x, y, width, height);
    return arc(x, y, width, height, 0, 360);
}

int ellipse_mode(int mode)
{
    psr_debug("ellipse_mode(%d)", mode);
    switch(mode) {
    case CENTER:
    case RADIUS:
    case CORNER:
    case CORNERS:
	break;
    default:
	psr_error("invalid ellipse mode");
	return -1;
    }
    g_ellipse_mode = mode;
    return 0;
}

int rect(float x, float y, float width, float height)
{
    psr_debug("rect(%f, %f, %f, %f)", x, y, width, height);
    begin_shape(QUADS);
    switch(g_rect_mode) {
    case CORNER:
	vertex(x, y, 0, 0, 0);
	vertex(x + width, y, 0, 0, 0);
	vertex(x + width, y + height, 0, 0, 0);
	vertex(x, y + height, 0, 0, 0);
	break;
    case CORNERS:
	vertex(x, y, 0, 0, 0);
	vertex(width, y, 0, 0, 0);
	vertex(width, height, 0, 0, 0);
	vertex(x, height, 0, 0, 0);
	break;
    case CENTER:
	vertex(x - width / 2, y - height / 2, 0, 0, 0);
	vertex(x + width / 2, y - height / 2, 0, 0, 0);
	vertex(x + width / 2, y + height / 2, 0, 0, 0);
	vertex(x - width / 2, y + height / 2, 0, 0, 0);
	break;
    case RADIUS:
	vertex(x - width, y - height, 0, 0, 0);
	vertex(x + width, y - height, 0, 0, 0);
	vertex(x + width, y + height, 0, 0, 0);
	vertex(x - width, y + height, 0, 0, 0);
	break;
    default:
	end_shape(CLOSE);
	psr_error("invalid g_rect_mode.  this should not happen.");
	return -1;
    }
    return end_shape(CLOSE);
}

int rect_mode(int mode)
{
    psr_debug("rect_mode(%d)", mode);
    switch(mode) {
    case CORNER:
    case CORNERS:
    case CENTER:
    case RADIUS:
	break;
    default:
	psr_error("invalid rect mode");
	return -1;
    }
    g_rect_mode = mode;
    return 0;
}

int bezier_detail(int level)
{
    psr_debug("bezier_detail(%d)", level);
    return renderer_context.bezier_detail(level);
}

int bezier_vertex(float cx1, float cy1, float cz1,
		  float cx2, float cy2, float cz2,
		  float x, float y, float z)
{
    psr_debug("bezier_vertex(%f, %f, %f, %f, %f, %f, %f, %f, %f",
	      cx1, cy1, cz1, cx2, cy2, cz2, x, y, z);
    return renderer_context.bezier_vertex(cx1, cy1, cz1, cx2, cy2, cz2, x, y, z);
}

int bezier(float x1, float y1, float z1,
	   float cx1, float cy1, float cz1,
	   float cx2, float cy2, float cz2,
	   float x2, float y2, float z2)
{
    psr_debug("bezier(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)",
	      x1, y1, z1, cx1, cy1, cz1, cx2, cy2, cz2, x2, y2, z2);
    begin_shape(POLYGON);
    vertex(x1, y1, z1, 0, 0);
    bezier_vertex(cx1, cy1, cz1, cx2, cy2, cz2, x2, y2, z2);
    return end_shape(OPEN);
}

/** FIXME: not finished yet. */
int box(float width, float height, float depth)
{
    psr_debug("box(%f, %f, %f)", width, height, depth);
    return renderer_context.box(width, height, depth);
}

int sphere(float radius)
{
    psr_debug("sphere(%f)", radius);
    return renderer_context.sphere(radius);
}

int sphere_detail(int n)
{
    psr_debug("sphere_detail(%d)", n);
    return renderer_context.sphere_detail(n);
}

int stroke_weight(float width)
{
    psr_debug("stroke_weight(%f)", width);
    return renderer_context.stroke_weight(width);
}

int smooth(void)
{
    psr_debug("smooth()");
    return renderer_context.smooth();
}

int no_smooth(void)
{
    psr_debug("no_smooth()");
    return renderer_context.no_smooth();
}

int fill(float r, float g, float b, float a)
{
    psr_debug("fill(%f, %f, %f, %f)", r, g, b, a);
    return renderer_context.fill(r, g, b, a);
}

int no_fill(void)
{
    psr_debug("no_fill()");
    return renderer_context.no_fill();
}

int save(struct psr_image *img)
{
    psr_debug("save(%p)", img);
    return renderer_context.save(img);
}

int image(struct psr_image *img, float x, float y, float width, float height)
{
    psr_debug("image(%p, %f, %f, %f, %f)", img, x, y, width, height);
    return renderer_context.image(img, x, y, width, height);
}

int camera_default(void)
{
    psr_debug("camera_default()");
    return renderer_context.camera_default();
}

int camera(float eye_x, float eye_y, float eye_z,
	   float center_x, float center_y, float center_z,
	   float up_x, float up_y, float up_z)
{
    psr_debug("camera(%f %f %f %f %f %f %f %f %f)",
	      eye_x, eye_y, eye_z,
	      center_x, center_y, center_z,
	      up_x, up_y, up_z);
    return renderer_context.camera(
	eye_x, eye_y, eye_z,
	center_x, center_y, center_z,
	up_x, up_y, up_z);
}

int begin_camera(void)
{
    psr_debug("begin_camera()");
    return renderer_context.begin_camera();
}

int end_camera(void)
{
    psr_debug("end_camera()");
    return renderer_context.end_camera();
}

int ortho(float left, float right, float bottom, float top,
	  float near, float far)
{
    psr_debug("ortho(%f, %f, %f, %f, %f, %f)",
	      left, right, bottom, top, near, far);
    return renderer_context.ortho(left, right, bottom, top, near, far);
}

/* default setup */
static void default_setup(void)
{
    psr_debug("default_setup()");
    //size(100, 100);
    frame_rate(60);
    rect_mode(CORNER);
    ellipse_mode(CENTER);
    bezier_detail(20);
    sphere_detail(30);
    stroke(0, 0, 0, 1);
    fill(1, 1, 1, 1);
    background(0.8, 0.8, 0.8, 1);
    psr_debug("end of default_setup()");
}

int processor_init(void)
{
    psr_context.update_key = update_key;
    psr_context.update_mouse = update_mouse;
    psr_context.update_size = update_size;
    psr_context.default_setup = default_setup;

    /* FIXME: check return value */
    load_renderer("./opengl/libpsr_gl.so");

    return 0;
}

int processor_run(struct psr_usr_func *usr_func)
{
    psr_context.usr_func = *usr_func;
    main_loop_start();
    return 0;
}
