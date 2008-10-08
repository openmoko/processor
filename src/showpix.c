#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define glCheckError()					\
    do {						\
	GLenum e;					\
	int r = 0;					\
	while ((e = glGetError()) != GL_NO_ERROR) {	\
	    r = -1;					\
	    fprintf(stderr, "%s", gluErrorString(e));	\
	}						\
    } while(0)

static int width = 100, height = 100;
void *image_data;

static void draw(void)
{
    /* display pic here */
    fprintf(stderr, "draw\n");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glRasterPos2i(0, 0);
    glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    glutSwapBuffers();
    glCheckError();
    return;
}

static void reshape(int lwidth, int lheight)
{
    fprintf(stderr, "reshape(%d, %d)\n", width, height);
    width = lwidth;
    height = lheight;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void idle(void)
{
    glutPostRedisplay();
}

static void visible(int vis)
{
    if (vis == GLUT_VISIBLE) {
	glutIdleFunc(idle);
    } else {
	glutIdleFunc(NULL);
    }
}

/* static void passive_motion(int x, int y) */
/* { */
/*     mouse_x = x; */
/*     mouse_y = y; */
/* } */

static void init(const char *filename)
{
    FILE *fp;
    const size_t s = sizeof(GLubyte) * 3;

    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glFlush();

    image_data = calloc(s, width * height);
    fp = fopen(filename, "r");
    fread(image_data, s, width * height, fp);
    fclose(fp);
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutCreateWindow("showpix");
    init(argv[1]);

    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    //glutKeyboardFunc(key);
    //glutSpecialFunc(special);
    //glutPassiveMotionFunc(passive_motion);
    //glutVisibilityFunc(visible);

    glutMainLoop();
    return 0;			/* ANSI C requires main to return int. */
}
