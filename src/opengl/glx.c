/** don't compile this. */

#include <X11/Xlib.h>
#include <GL/glx.h>

#include "util.h"

static int attrListSgl[] = {
    GLX_RGBA,
    GLX_RED_SIZE, 4,
    GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 16,
    None
};

static int attrListDbl[] = {
    GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 4,
    GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 16,
    None
};

struct glx_window {
    Display *dpy;
    int screen;
    Window win;
    GLXContext ctx;
    XSetWindowAttributes attr;
    XF86VidModeModeInfo deskMode;
    unsigned int width, height;
};

/**
 * Defines the dimension of the display window in units of pixels.
 */
int size(struct glx_window *handle, int width, int height)
{
    Display *dpy;
    int screen;
    XVisualInfo *vis;
    GLXContext *ctx;

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
	psr_error("XOpenDisplay returns NULL.");
	return False;
    }
    screen = DefaultScreen(dpy)
	vis = glXChooseVisual(dpy, screen, attrListDbl);
    if (!vis) {
	psr_warn("No double buffered visual!");
	vis = glXChooseVisual(dpy, screen, attrListSgl);
	if (!vis) {
	    psr_error("No matching visual.");
	    return False;
	}
    }
#ifndef NDEBUG
    int major, minor;
    if (glXQueryVersion(dpy, &major, &minor)) {
	psr_debug("glX version: %d.%d", major, minor);
    }
#endif
    ctx = glXCreateContext(dpy, vis, NULL, True);
    if (!ctx) {
	psr_error("glXCreateContext failed.");
	return False;
    }
}
