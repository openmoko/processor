#include <time.h>
#include <errno.h>
#include <GL/glut.h>

#include "common.h"

static struct psr_context *psr_cxt = NULL;
static struct psr_renderer_context *renderer_cxt = NULL;
static struct timespec interval = {0, 0};
static volatile int looping = 1;

extern int gl_init(struct psr_context *psr_cxt,
		   struct psr_renderer_context *renderer_cxt);

extern int gl_reshape(int width, int height);

/* Subtract the `struct timespec' values X and Y,
   storing the result in RESULT.
   Return -1 if the difference is negative, otherwise 0.
   -- from glibc info doc */
static inline int timespec_sub (
    struct timespec *sub,
    struct timespec *left,
    struct timespec *right)
{
    sub->tv_sec = left->tv_sec - right->tv_sec;
    sub->tv_nsec = left->tv_nsec - right->tv_nsec;
    if (sub->tv_nsec < 0) {
	--sub->tv_sec;
	sub->tv_nsec += 1000000000;
    }
    if (left->tv_sec < right ->tv_sec ||
	(left->tv_sec == right->tv_sec &&
	 left->tv_nsec < right->tv_nsec)) {
	return -1;
    }
    return 0;
}

static inline void timespec_add (
    struct timespec *sum,
    const struct timespec *left,
    const struct timespec *right)
{
    sum->tv_sec = left->tv_sec + right->tv_sec;
    sum->tv_nsec = left->tv_nsec + right->tv_nsec;
    if (sum->tv_nsec >= 1000000000) {
	++sum->tv_sec;
	sum->tv_nsec -= 1000000000;
    }
}

static void display_draw(void)
{
    psr_debug("display_draw");
    psr_cxt->draw();
    glutSwapBuffers();
}

static void display_setup(void)
{
    psr_debug("display_setup");
    psr_cxt->setup();
    glutSwapBuffers();
    if (psr_cxt->draw) {
	glutDisplayFunc(display_draw);
    }
}

static void reshape(int width, int height)
{
    psr_cxt->update_size(width, height);
    gl_reshape(width, height);
}

static void idle(void)
{
    static struct timespec expire = {0, 0};
    struct timespec now;
    struct timespec diff;
    int r;
    r = clock_gettime(CLOCK_REALTIME, &now);
    if (r) {
	psr_system_warn(errno, "gettimeofday");
	return;
    }
    if (!timespec_sub(&diff, &expire, &now)) {
	/* not yet, sleep for a while */
	psr_debug("nanosleep(tv_sec = %ld, tv_nsec = %ld)", diff.tv_sec, diff.tv_nsec);
	r = nanosleep(&diff, &expire);
	if (r) {
	    psr_system_warn(errno, "nanosleep");
	}
    }
    glutPostRedisplay();
    timespec_add(&expire, &now, &interval);
    if (!looping) {
	glutIdleFunc(NULL);
	return;
    }
}

static void visible(int vis)
{
    psr_debug("visible");
    if (vis == GLUT_VISIBLE && psr_cxt->draw) {
	glutIdleFunc(idle);
    } else {
	glutIdleFunc(NULL);
    }
}

static void keyboard(unsigned char key, int x, int y)
{
    int keycode = -1;		/* -1 means we don't update keycode */
    switch (key) {
    case BACKSPACE:
    case TAB:
    case ENTER:
    case RETURN:
    case ESC:
    case DELETE:
	keycode = key;
    default:
	break;
    }
    psr_cxt->update_key(key, keycode);
    return;
}

/* NOTE: Does NOT support ALT, CONTROL and SHIFT */
static void special(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP:
	psr_cxt->update_key(CODED, UP);
	break;
    case GLUT_KEY_DOWN:
	psr_cxt->update_key(CODED, DOWN);
	break;
    case GLUT_KEY_LEFT:
	psr_cxt->update_key(CODED, LEFT);
	break;
    case GLUT_KEY_RIGHT:
	psr_cxt->update_key(CODED, RIGHT);
	break;
    case GLUT_KEY_F1:
    case GLUT_KEY_F2:
    case GLUT_KEY_F3:
    case GLUT_KEY_F4:
    case GLUT_KEY_F5:
    case GLUT_KEY_F6:
    case GLUT_KEY_F7:
    case GLUT_KEY_F8:
    case GLUT_KEY_F9:
    case GLUT_KEY_F10:
    case GLUT_KEY_F11:
    case GLUT_KEY_F12:
    case GLUT_KEY_PAGE_UP:
    case GLUT_KEY_PAGE_DOWN:
    case GLUT_KEY_HOME:
    case GLUT_KEY_END:
    case GLUT_KEY_INSERT:
	break;
    default:
	psr_system_error(EINVAL, "invalid 'key' argument.");
    }
    return;
}

static void mouse(int button, int state, int x, int y)
{
    switch (button) {
    case GLUT_LEFT_BUTTON:
	button = LEFT;
	break;
    case GLUT_MIDDLE_BUTTON:
	button = CENTER;
	break;
    case GLUT_RIGHT_BUTTON:
	button = RIGHT;
	break;
    default:
	psr_system_error(EINVAL, "invalid 'button' argument.");
    }
    psr_cxt->update_mouse(x, y, button);
    if (state == GLUT_DOWN) {
	psr_cxt->mouse_pressed();
    } else if (state == GLUT_UP) {
	psr_cxt->mouse_released();
	psr_cxt->mouse_clicked();
    }
}

static void motion(int x, int y)
{
    psr_cxt->update_mouse(x, y, -1);	/* don't update button */
    psr_cxt->mouse_dragged();
}

static void passive_motion(int x, int y)
{
    psr_cxt->update_mouse(x, y, -1);	/* don't update button */
    psr_cxt->mouse_moved();
}

static int size(int width, int height)
{
    glutReshapeWindow(width, height);
    return 0;
}

static int no_loop(void)
{
    looping = 0;
    return 0;
}

static int loop(void)
{
    looping = 1;
    glutIdleFunc(idle);
    return 0;
}

static int redraw(void)
{
    glutIdleFunc(idle);
    return 0;
}

static int frame_rate(float framerate)
{
    float frametime = 1 / framerate;
    interval.tv_sec = frametime;
    interval.tv_nsec = (frametime - interval.tv_sec) * 1000000000;
    return 0;
}

int init(struct psr_context *lpsr_cxt,
	 struct psr_renderer_context *lrenderer_cxt)
{
    psr_cxt = lpsr_cxt;
    renderer_cxt = lrenderer_cxt;
    renderer_cxt->size = size;
    renderer_cxt->no_loop = no_loop;
    renderer_cxt->loop = loop;
    renderer_cxt->redraw = redraw;
    renderer_cxt->frame_rate = frame_rate;
    return 0;
}

int main_loop_start(void)
{
    int argc = 1;
    char *argv[] = { "Processor" };

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    glutCreateWindow("Processor");
    gl_init(psr_cxt, renderer_cxt);

    glutDisplayFunc(display_setup);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passive_motion);
    glutVisibilityFunc(visible);

    psr_cxt->default_setup();
    glutMainLoop();
    return 0;
}
