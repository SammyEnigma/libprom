.PHONY: clean

GCC ?= gcc
CFLAGS ?= -g -Wall -I../prom/include -I../promhttp/include -DPROM_LOG_ENABLE \
	-I /usr/include/microhttpd -I /opt/local/include
# You should "export LD_LIBRARY_PATH=../prom/build:../promhttp/build"
LDFLAGS ?= -L../prom/build -L../promhttp/build -L/opt/local/lib

DEPS = foo.h bar.h
OBJ = foo.o bar.o main.o

example: ${OBJ} ${LIBS}
	gcc -g -o $@ ${CFLAGS} -O1 -pthread ${OBJ} $(LDFLAGS) -lprom -lpromhttp -lmicrohttpd

%.o: %.c ${DEPS}
	${GCC} ${CFLAGS} -c $< -o $@

clean:
	-rm -f ${OBJ} example
