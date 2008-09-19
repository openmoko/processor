#include <errno.h>
#include <GL/glut.h>

#include "common.h"

static struct psr_context *psr_cxt = NULL;
static struct psr_renderer_context *renderer_cxt = NULL;

extern int gl_init(struct psr_context *psr_cxt,
		   struct psr_renderer_context *renderer_cxt);

extern int gl_reshape(int width, int height);

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
    gl_reshape(width, height);
}

static void idle(void)
{
    if (psr_cxt->draw) {
	glutPostRedisplay();
    }
}

static void visible(int vis)
{
    psr_debug("visible");
    if (vis == GLUT_VISIBLE) {
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

int init(struct psr_context *lpsr_cxt,
	 struct psr_renderer_context *lrenderer_cxt)
{
    psr_cxt = lpsr_cxt;
    renderer_cxt = lrenderer_cxt;
    renderer_cxt->size = size;
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
    /* FIXME: implement this */
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
