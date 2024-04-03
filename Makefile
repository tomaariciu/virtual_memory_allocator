CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g

build:
	$(CC) $(CFLAGS) *.c -o sfl

run_sfl:
	./sfl

run_debug:
	./sfl < test.in > test.out

clean:
	rm sfl