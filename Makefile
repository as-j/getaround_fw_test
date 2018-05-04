parse: parse.c nmea_parse.c
	gcc -o parse -Wall parse.c nmea_parse.c

parse2: parse2.c nmea_parse.c
	gcc -o parse2 -Wall parse2.c nmea_parse.c

unittest: unittest.c nmea_parse.c
	gcc -o unittest -Wall unittest.c nmea_parse.c

all: parse parse2 unittest

clean:
	rm parse parse2 unittest
