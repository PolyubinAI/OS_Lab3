FLAGS = -std=c99 -pthread
СС = gcc
all: main

main: main.c
	$(CC) $(FLAGS) main.c -o main

clean:
	rm main
