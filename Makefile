parse: parse.c
	gcc -o parse -Wall parse.c

parse2: parse2.c
	gcc -o parse2 -Wall parse2.c

unittest: unittest.c
	gcc -o unittest -Wall unittest.c

all: parse parse2 unittest

clean:
	rm parse parse2 unittest
