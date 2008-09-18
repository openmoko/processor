CFLAGS = -g -Wall -fPIC -I.
TARGETS = libprocessing.so RGBCube


.PHONY: all
all: ${TARGETS}

libprocessing.so: main.o
	${CC} -shared -ldl -o $@ $^

RGBCube: RGBCube.o
	${CC} ${CFLAGS} -L. -lprocessing -o $@ $^

.PHONY: clean
clean:
	rm -f *.o ${TARGETS}
