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
extern int apply_matrix(float n11, float n12, float n13, float n14,
			float n21, float n22, float n23, float n24,
			float n31, float n32, float n33, float n34,
			float n41, float n42, float n43, float n44);
extern int reset_matrix(void);
extern int print_matrix(void);
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
extern int bezier_detail(int level);
extern int bezier_vertex(float cx1, float cy1, float cz1,
			 float cx2, float cy2, float cz2,
			 float x, float y, float z);
extern int bezier(float x1, float y1, float z1,
		  float cx1, float cy1, float cz1,
		  float cx2, float cy2, float cz2,
		  float x2, float y2, float z2);
extern int box(float width, float height, float depth);
extern int sphere(float radius);
extern int sphere_detail(int n);
extern int stroke_weight(float width);
extern int smooth(void);
extern int no_smooth(void);
extern int fill(float r, float g, float b, float a);
extern int no_fill(void);
extern int save(struct psr_image *img);
extern int image(struct psr_image *img, float x, float y, float width, float height);
extern int camera_default(void);
extern int camera(float eye_x, float eye_y, float eye_z,
		  float center_x, float center_y, float center_z,
		  float up_x, float up_y, float up_z);
extern int begin_camera(void);
extern int end_camera(void);
extern int ortho(float left, float right, float bottom, float top,
		 float near, float far);
extern int processor_init(void);
extern int processor_run(struct psr_usr_func *usr_func);

#endif				/* PROCESSING_H */
