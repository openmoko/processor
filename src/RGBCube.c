#include <stdio.h>
#include <stdlib.h>
#include <processing.h>

static float xmag = 0, ymag = 0;
static float newXmag = 0, newYmag = 0;

static void setup(void)
{
    //size(200, 200);
    //no_stroke();
    no_fill();

    push_matrix();
    begin_camera();
    translate(10, 10, 0);
    //rotate_x(-PI/6);
    end_camera();

    translate(50, 50, 0);
    rotate_y(PI/3);
    box(45, 45, 45);
    pop_matrix();
    return;
}

static void draw2d(void)
{
    background(0.5, 0.5, 0.5, 1);
    smooth();
    ellipse(70, 48, 36, 36);
    no_smooth();
    ellipse(30, 48, 36, 36);
    no_loop();
    return;
}

static void draw3d(void)
{
    float diff;

    background(0.5, 0.5, 0.45, 1);

    push_matrix();

    translate(width / 2, height / 2, -30);

    newXmag = (float) mouse_x / (float) width * TWO_PI;
    newYmag = (float) mouse_y / (float) height * TWO_PI;

    diff = xmag - newXmag;
    if (fabsf(diff) > 0.01) {
	xmag -= diff / 4.0;
    }

    diff = ymag - newYmag;
    if (fabsf(diff) > 0.01) {
	ymag -= diff / 4.0;
    }

    rotate_x(-ymag);
    rotate_y(-xmag);

    scale(50, 50, 50);

    begin_shape(QUADS);

    fill(0, 1, 1, 1);
    vertex(-1, 1, 1, 0, 0);
    fill(1, 1, 1, 1);
    vertex(1, 1, 1, 0, 0);
    fill(1, 0, 1, 1);
    vertex(1, -1, 1, 0, 0);
    fill(0, 0, 1, 1);
    vertex(-1, -1, 1, 0, 0);

    fill(1, 1, 1, 1);
    vertex(1, 1, 1, 0, 0);
    fill(1, 1, 0, 1);
    vertex(1, 1, -1, 0, 0);
    fill(1, 0, 0, 1);
    vertex(1, -1, -1, 0, 0);
    fill(1, 0, 1, 1);
    vertex(1, -1, 1, 0, 0);

    fill(1, 1, 0, 1);
    vertex(1, 1, -1, 0, 0);
    fill(0, 1, 0, 1);
    vertex(-1, 1, -1, 0, 0);
    fill(0, 0, 0, 1);
    vertex(-1, -1, -1, 0, 0);
    fill(1, 0, 0, 1);
    vertex(1, -1, -1, 0, 0);

    fill(0, 1, 0, 1);
    vertex(-1, 1, -1, 0, 0);
    fill(0, 1, 1, 1);
    vertex(-1, 1, 1, 0, 0);
    fill(0, 0, 1, 1);
    vertex(-1, -1, 1, 0, 0);
    fill(0, 0, 0, 1);
    vertex(-1, -1, -1, 0, 0);

    fill(0, 1, 0, 1);
    vertex(-1, 1, -1, 0, 0);
    fill(1, 1, 0, 1);
    vertex(1, 1, -1, 0, 0);
    fill(1, 1, 1, 1);
    vertex(1, 1, 1, 0, 0);
    fill(0, 1, 1, 1);
    vertex(-1, 1, 1, 0, 0);

    fill(0, 0, 0, 1);
    vertex(-1, -1, -1, 0, 0);
    fill(1, 0, 0, 1);
    vertex(1, -1, -1, 0, 0);
    fill(1, 0, 1, 1);
    vertex(1, -1, 1, 0, 0);
    fill(0, 0, 1, 1);
    vertex(-1, -1, 1, 0, 0);

    end_shape(CLOSE);

    pop_matrix();
    return;
}

int main(int argc, char *argv[])
{
    struct psr_usr_func my_func = {
	.mouse_dragged = NULL,
	.mouse_moved = NULL,
	.mouse_released = NULL,
	.mouse_pressed = NULL,
	.key_released = NULL,
	.key_pressed = NULL,
	.setup = setup,
	.draw = NULL,
    };
    processor_init();
    processor_run(&my_func);
    return 0;
}
