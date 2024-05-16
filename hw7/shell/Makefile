CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: mysh

mysh: mysh.c builtin_cmds.c parser.c linked_jobs.c
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable mysh.c builtin_cmds.c parser.c linked_jobs.c -o $@ -lm -lrt

clean:
	rm -f mysh