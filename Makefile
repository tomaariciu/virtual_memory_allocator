CC=gcc
CFLAGS=-std=c99 -Wall -Wextra

build:
	$(CC) $(CFLAGS) *.c -o sfl

run_sfl:
	./sfl

clean:
	rm sfl