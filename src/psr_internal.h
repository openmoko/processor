#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include "psr_common.h"

/* error reporting */

#include <stdio.h>
#include <error.h>

extern volatile int debug_level;

#define psr_system_error(e, fmt...) do {			\
	error_at_line(1, (e), __FILE__, __LINE__, ##fmt);	\
    } while(0)

#define psr_system_warn(e, fmt...) do {				\
	error_at_line(0, (e), __FILE__, __LINE__, ##fmt);	\
    } while(0)

#define psr_error(fmt...)					\
    if (debug_level & 1 << 3) {					\
	fprintf(stderr, "ERROR:%s:%d:", __FILE__, __LINE__);	\
	fprintf(stderr, ##fmt);					\
	fputs("\n", stderr);					\
	exit(1);						\
    }

#define psr_warn(fmt...)					\
    if (debug_level & 1 << 2) {					\
	fprintf(stderr, "WARN:%s:%d:", __FILE__, __LINE__);	\
	fprintf(stderr, ##fmt);					\
	fputs("\n", stderr);					\
    }

#define psr_note(fmt...)					\
    if (debug_level & 1 << 1) {					\
	fprintf(stderr, "NOTE:%s:%d:", __FILE__, __LINE__);	\
	fprintf(stderr, ##fmt);					\
	fputs("\n", stderr);					\
    }

#define psr_debug(fmt...)					\
    if (debug_level & 1 << 0) {					\
	fprintf(stderr, "DEBUG:%s:%d:", __FILE__, __LINE__);	\
	fprintf(stderr, ##fmt);					\
	fputs("\n", stderr);					\
    }


struct psr_context {
    void (*update_key) (int key, int keycode);
    void (*update_mouse) (int x, int y, int button);
    void (*update_size) (int width, int height);
    void (*default_setup) (void);
    struct psr_usr_func usr_func;
};

struct psr_renderer_context {
    int (*size) (int width, int height);
    int (*no_loop) (void);
    int (*loop) (void);
    int (*redraw) (void);
    int (*frame_rate) (float framerate);
    int (*cursor) (int type);
    int (*stroke) (float r, float g, float b, float a);
    int (*background) (float r, float g, float b, float a);
    int (*push_matrix) (void);
    int (*pop_matrix) (void);
    int (*translate) (float x, float y, float z);
    int (*rotate) (float angle, float x, float y, float z);
    int (*scale) (float x, float y, float z);
    int (*begin_shape) (int mode);
    int (*vertex) (float x, float y, float z, float u, float v);
    int (*end_shape) (int end_mode);
    int (*arc) (float x, float y, float width, float height, float start,
		float stop);
    int (*bezier_detail) (int level);
    int (*bezier_vertex) (float cx1, float cy1, float cz1,
			  float cx2, float cy2, float cz2,
			  float x, float y, float z);
    int (*fill) (float r, float g, float b, float a);
};

#define DEFAULT_WIDTH (100)
#define DEFAULT_HEIGHT (100)

#endif				/* COMMON_H */
