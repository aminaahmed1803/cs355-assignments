CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: mem

mem: mem.c  
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable mem.c -o $@ -lm -lreadline -lrt

clean: 
	rm mem