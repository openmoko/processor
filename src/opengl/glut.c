#include <errno.h>
#include <GL/glut.h>

#include "common.h"

static struct processor_context *psr_context = NULL;

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
    psr_context->update_key(key, keycode);
    return;
}

/* NOTE: Does NOT support ALT, CONTROL and SHIFT */
static void special(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP:
	psr_context->update_key(CODED, UP);
	break;
    case GLUT_KEY_DOWN:
	psr_context->update_key(CODED, DOWN);
	break;
    case GLUT_KEY_LEFT:
	psr_context->update_key(CODED, LEFT);
	break;
    case GLUT_KEY_RIGHT:
	psr_context->update_key(CODED, RIGHT);
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
    psr_context->update_mouse(x, y, button);
    if (state == GLUT_DOWN) {
	psr_context->mouse_pressed();
    } else if (state == GLUT_UP) {
	psr_context->mouse_released();
	psr_context->mouse_clicked();
    }
}

static void motion(int x, int y)
{
    psr_context->update_mouse(x, y, -1);	/* don't update button */
    psr_context->mouse_dragged();
}

static void passive_motion(int x, int y)
{
    psr_context->update_mouse(x, y, -1);	/* don't update button */
    psr_context->mouse_moved();
}

int init(struct processor_context *context)
{
    psr_context = context;
    return 0;
}

int size(int width, int height)
{
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    return 0;
}

int pre_setup(void)
{
    int argc = 1;
    char *argv[] = {"processor"};

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passive_motion);
    glutCreateWindow("Processor");
    return 0;
}

int post_setup(void)
{
    return 0;
}
