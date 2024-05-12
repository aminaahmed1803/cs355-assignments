CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: mem test

mem: mem.c  
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable mem.c -o $@ -lm -lreadline -lrt

test: test.c mem.c  
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable test.c mem.c -o $@ -lm -lreadline -lrt

random: test.c random.c  
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable random.c mem.c -o $@ -lm -lreadline -lrt

clean: 
	rm mem test random