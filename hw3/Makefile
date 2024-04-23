CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: myshell

myshell: mysh.c parser.c linked_jobs.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable mysh.c parser.c linked_jobs.c -o $@ -lm -lreadline -lrt

clean: 
	rm myshell





