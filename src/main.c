#include <dlfcn.h>
#include <errno.h>

#include "common.h"

struct psr_context psr_context;
struct psr_renderer_context renderer_context;

volatile int debug_level = 1 << 3;

volatile char key;
volatile int keycode;
volatile int mouse_x;
volatile int mouse_y;
volatile int mouse_button;
volatile int p_mouse_x;
volatile int p_mouse_y;
volatile int width;
volatile int height;

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
    return 1;
}

static void update_key(int lkey, int lkeycode)
{
    key = lkey;
    if (lkeycode > -1) {
	keycode = lkeycode;
    }
}

static void update_mouse(int x, int y, int button)
{
    p_mouse_x = mouse_x;
    mouse_x = x;
    p_mouse_y = mouse_y;
    mouse_y = y;
    if (button > -1) {
	mouse_button = button;
    }
}

static void update_size(int lwidth, int lheight)
{
    width = lwidth;
    height = lheight;
}

static void null_stub(void)
{
    return;
}

int size(int lwidth, int lheight)
{
    psr_debug("size(%d, %d)", lwidth, lheight);
    width = lwidth;
    height = lheight;
    return renderer_context.size(width, height);
}

int stroke(float r, float g, float b, float a)
{
    psr_debug("stroke(%f, %f, %f, %f)", r, g, b, a);
    psr_context.stroke = 1;
    return renderer_context.stroke(r, g, b, a);
}

int no_stroke(void)
{
    int r;
    psr_debug("no_stroke");
    r = stroke(0, 0, 0, 0);
    psr_context.stroke = 0;
    return r;
}

int background(float r, float g, float b, float a)
{
    psr_debug("background(%f, %f, %f, %f)", r, g, b, a);
    return renderer_context.background(r, g, b, a);
}

int push_matrix(void)
{
    psr_debug("push_matrix");
    return renderer_context.push_matrix();
}

int pop_matrix(void)
{
    psr_debug("pop_matrix");
    return renderer_context.pop_matrix();
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

int fill(float r, float g, float b, float a)
{
    psr_debug("fill(%f, %f, %f, %f)", r, g, b, a);
    psr_context.fill = 1;
    return renderer_context.fill(r, g, b, a);
}

int no_fill(void)
{
    int r;
    psr_debug("no_fill");
    r = fill(0, 0, 0, 0);
    psr_context.fill = 0;
    return r;
}

/* default setup */
static void default_setup(void)
{
    background(1, 1, 1, 1);
    stroke(0, 0, 0, 1);
    fill(0, 0, 0, 1);
}

int processor_init(void)
{
    psr_context.update_key = update_key;
    psr_context.key_pressed = null_stub;
    psr_context.key_released = null_stub;

    psr_context.update_mouse = update_mouse;
    psr_context.mouse_dragged = null_stub;
    psr_context.mouse_moved = null_stub;
    psr_context.mouse_released = null_stub;
    psr_context.mouse_pressed = null_stub;
    psr_context.mouse_clicked = null_stub;

    psr_context.update_size = update_size;

    psr_context.default_setup = default_setup;
    psr_context.setup = NULL;
    psr_context.draw = NULL;

    /* FIXME: check this */
    psr_context.stroke = 1;
    psr_context.fill = 1;

    /* FIXME: check return value */
    load_renderer("./opengl/libpsr_gl.so");

    return 0;
}

int processor_run(void (*setup) (void),
		  void (*draw) (void))
{
    psr_context.setup = setup;
    psr_context.draw = draw;
    main_loop_start();
    return 0;
}
