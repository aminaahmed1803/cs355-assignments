all: matrix mysh

matrix: matrix.c
		gcc -g -w matrix.c -o matrix -lm -lpthread

mysh: mysh.c
		gcc -g -w mysh.c -o mysh -lm -lreadline

clean:
		rm matrix mysh