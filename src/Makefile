CFLAGS = -g -Wall -fPIC -I.
TARGETS = libprocessing.so RGBCube RGBCube_glut


.PHONY: all
all: ${TARGETS}

libprocessing.so: main.o
	${CC} -shared -ldl -o $@ $^

RGBCube: RGBCube.o
	${CC} ${CFLAGS} -L. -lprocessing -o $@ $^

RGBCube_glut: RGBCube_glut.o
	${CC} ${CFLAGS} -lGL -lGLU -lglut -o $@ $^

.PHONY: clean
clean:
	rm -f *.o ${TARGETS}
