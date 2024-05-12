CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: myshell my parser

myshell: mysh.c parser.c linked_jobs.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable mysh.c parser.c linked_jobs.c -o $@ -lm -lreadline -lrt

parser: parser.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable parser.c -o $@ -lm -lreadline -lrt


clean: 
	rm myshell my parser