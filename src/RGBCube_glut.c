#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

static float xmag = 0, ymag = 0;
static float newXmag = 0, newYmag = 0;
static int width = 200, height = 200;
static int mouse_x = 0, mouse_y = 0;

static void draw(void)
{
    GLenum e;
    float diff;

    glClearColor(0.5, 0.5, 0.45, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
//    glPushMatrix();

    //glTranslatef(width / 2, height / 2, -30);
    glTranslatef(0, 0, -6.0);

    newXmag = mouse_x / width * 2 * M_PI;
    newYmag = mouse_y / height * 2 * M_PI;

    diff = xmag - newXmag;
    if (abs((int) diff) > 0.01) {
	xmag -= diff / 4.0;
    }

    diff = ymag - newYmag;
    if (abs((int) diff) > 0.01) {
	ymag -= diff / 4.0;
    }

    //glRotatef(-ymag, 1.0, 0, 0);
    //glRotatef(-xmag, 0, 1.0, 0);

    //glScalef(50, 50, 50);
    glBegin(GL_QUADS);

    glColor4f(0, 1, 1, 1);
    glVertex3f(-1, 1, 1);
    glColor4f(1, 1, 1, 1);
    glVertex3f(1, 1, 1);
    glColor4f(1, 0, 1, 1);
    glVertex3f(1, -1, 1);
    glColor4f(0, 0, 1, 1);
    glVertex3f(-1, -1, 1);

    glColor4f(1, 1, 1, 1);
    glVertex3f(1, 1, 1);
    glColor4f(1, 1, 0, 1);
    glVertex3f(1, 1, -1);
    glColor4f(1, 0, 0, 1);
    glVertex3f(1, -1, -1);
    glColor4f(1, 0, 1, 1);
    glVertex3f(1, -1, 1);

    glColor4f(1, 1, 0, 1);
    glVertex3f(1, 1, -1);
    glColor4f(0, 1, 0, 1);
    glVertex3f(-1, 1, -1);
    glColor4f(0, 0, 0, 1);
    glVertex3f(-1, -1, -1);
    glColor4f(1, 0, 0, 1);
    glVertex3f(1, -1, -1);

    glColor4f(0, 1, 0, 1);
    glVertex3f(-1, 1, -1);
    glColor4f(0, 1, 1, 1);
    glVertex3f(-1, 1, 1);
    glColor4f(0, 0, 1, 1);
    glVertex3f(-1, -1, 1);
    glColor4f(0, 0, 0, 1);
    glVertex3f(-1, -1, -1);

    glColor4f(0, 1, 0, 1);
    glVertex3f(-1, 1, -1);
    glColor4f(1, 1, 0, 1);
    glVertex3f(1, 1, -1);
    glColor4f(1, 1, 1, 1);
    glVertex3f(1, 1, 1);
    glColor4f(0, 1, 1, 1);
    glVertex3f(-1, 1, 1);

    glColor4f(0, 0, 0, 1);
    glVertex3f(-1, -1, -1);
    glColor4f(1, 0, 0, 1);
    glVertex3f(1, -1, -1);
    glColor4f(1, 0, 1, 1);
    glVertex3f(1, -1, 1);
    glColor4f(0, 0, 1, 1);
    glVertex3f(-1, -1, 1);

    glEnd();

    //glPopMatrix();

    glutSwapBuffers();

    while ((e = glGetError()) != GL_NO_ERROR) {
	fprintf(stderr, "%s", gluErrorString(e));
    }
    return;
}

static void reshape(int width, int height)
{
    glViewport(0, 0, width, height);    /* Reset The Current Viewport And Perspective Transformation */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
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

static void init(void)
{
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glFlush();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutCreateWindow("RGBCube");
    init();

    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    //glutKeyboardFunc(key);
    //glutSpecialFunc(special);
    glutVisibilityFunc(visible);

    glutMainLoop();
    return 0;			/* ANSI C requires main to return int. */
}
