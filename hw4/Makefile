CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: userthread

userthread: userthread.c fifo.c sjf.c priority.c
	$(CC) -g -Wall -Wvla  userthread.c fifo.c sjf.c priority.c  -o $@ -lm

clean:
	rm userthread