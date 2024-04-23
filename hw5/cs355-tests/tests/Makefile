C_FILES := $(wildcard *.c)
TARGETS := ${C_FILES:.c=}

all: ${TARGETS}

%: %.c
	gcc -g -L.. -lmem -lm -Xlinker -rpath=. -o $@ $<

clean:
	rm -rf ${TARGETS} *.o
