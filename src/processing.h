#ifndef PROCESSING_H
#define PROCESSING_H

#include <psr_constants.h>

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
extern int fill(float r, float g, float b, float a);
extern int no_fill(void);

extern int processor_init(void);
extern int processor_run(void (*setup) (void), void (*draw) (void));

#endif /* PROCESSING_H */
