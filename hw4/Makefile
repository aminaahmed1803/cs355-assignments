CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: fifo userthread

fifo: fifo.c 
	$(CC) -g -Wall -Wvla  fifo.c -o $@ -lm

userthread: userthread.c fifo.c sjf.c
	$(CC) -g -Wall -Wvla  userthread.c fifo.c sjf.c  -o $@ -lm

clean:
	rm userthread