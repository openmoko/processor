CFLAGS = -g -I../ -fPIC -Wall
TARGETS = libpsr_gl.so

.PHONY: all
all: ${TARGETS}

libpsr_gl.so: gl.o glut.o
	${CC} -shared -lrt -lGL -lGLU -lglut -o $@ $^

.PHONY: clean
clean:
	rm -f *.o ${TARGETS}
