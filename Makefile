CC ?= cc

all: tf1d

tf1d: tf1d.c
	$(CC) -std=gnu99 -O2 -lasound -lusb-1.0 -o tf1d tf1d.c

clean:
	rm -rf tf1d

.PHONY: all clean

