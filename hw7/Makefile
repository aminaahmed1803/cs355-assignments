CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: fat format mysh

fat: fat.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable fat.c -o $@ -lm -lrt

format: format.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable format.c -o $@ -lm -lrt

read: reader.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable reader.c -o $@ -lm -lrt

mysh: mysh.c parser.c linked_jobs.c fat.c fat_cmds.c
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable mysh.c parser.c linked_jobs.c fat.c fat_cmds.c -o $@ -lm -lrt


clean: 
	rm fat format mysh