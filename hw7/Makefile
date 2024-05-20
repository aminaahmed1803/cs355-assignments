CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: format mysh

format: format.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable format.c -o $@ -lm -lrt
mysh: mysh.c parser.c linked_jobs.c fat.c fat_cmds.c
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable mysh.c parser.c linked_jobs.c fat.c fat_cmds.c -o $@ -lm -lrt

test1: test_dir.c fat.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable test_dir.c fat.c -o $@ -lm -lrt

test2: test_write_read.c fat.c
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable test_write_read.c fat.c -o $@ -lm -lrt

test3: test_fopen.c fat.c
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable test_fopen.c fat.c -o $@ -lm -lrt

clean: 
	rm format mysh