CC=gcc
% :: %.c
	$(CC) -g -Wall -Wvla -Werror $< -o $@

all: fat format

fat: fat.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable fat.c -o $@ -lm -lrt

format: format.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable format.c -o $@ -lm -lrt

read: reader.c 
	$(CC) -g -Wall -Wvla -Werror -Wno-unused-variable reader.c -o $@ -lm -lrt

clean: 
	rm fat format