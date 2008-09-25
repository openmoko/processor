#ifndef PROCESSING_H
#define PROCESSING_H

#include <psr_common.h>

extern volatile char key;
extern volatile int key_code;
extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_button;
extern volatile int p_mouse_x;
extern volatile int p_mouse_y;
extern volatile int width;
extern volatile int height;

extern int size(int width, int height);
extern int no_loop(void);
extern int loop(void);
extern int redraw(void);
extern int delay(int milliseconds);
extern int frame_rate(float framerate);
extern int cursor(int type);
extern int no_cursor();
extern const char *binary(int i);
extern int unbinary(const char *s);
extern const char *hex(int i);
extern int unhex(const char *s);
extern int stroke(float r, float g, float b, float a);
extern int no_stroke(void);
extern int background(float r, float g, float b, float a);
extern int push_matrix(void);
extern int pop_matrix(void);
extern int translate(float x, float y, float z);
extern int rotate(float angle, float x, float y, float z);
extern int rotate_x(float angle);
extern int rotate_y(float angle);
extern int rotate_z(float angle);
extern int scale(float x, float y, float z);
extern int begin_shape(int mode);
extern int vertex(float x, float y, float z, float u, float v);
extern int end_shape(int end_mode);
extern int triangle(float x1, float y1, float x2, float y2, float x3,
		    float y3);
extern int line(float x1, float y1, float z1, float x2, float y2,
		float z2);
extern int arc(float x, float y, float width, float height, float start,
	       float stop);
extern int point(float x, float y, float z);
extern int quad(float x1, float y1, float x2, float y2, float x3, float y3,
		float x4, float y4);
extern int ellipse(float x, float y, float width, float height);
extern int ellipse_mode(int mode);
extern int rect(float x, float y, float width, float height);
extern int rect_mode(int mode);
extern int fill(float r, float g, float b, float a);
extern int no_fill(void);

extern int processor_init(void);
extern int processor_run(struct psr_usr_func *usr_func);

#endif				/* PROCESSING_H */
