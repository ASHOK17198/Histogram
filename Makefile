CC = gcc
CFLAGS = -Wall -std=gnu99 -O0


all:	histogram.c histogram

clean:
	rm -f histogram
