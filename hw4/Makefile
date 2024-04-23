CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: userthread test

userthread: userthread.c queue.c rr_thread.c sjf_thread.c fifo_thread.c
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable userthread.c queue.c rr_thread.c sjf_thread.c fifo_thread.c -o $@ -lm

test: test.c userthread.c queue.c rr_thread.c sjf_thread.c fifo_thread.c
	$(CC) -g -Wall  -Wno-unused-variable test.c userthread.c queue.c rr_thread.c sjf_thread.c fifo_thread.c  -o $@ -lm


clean: 
	rm userthread test