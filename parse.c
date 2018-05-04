/*

Parse the strings from stdin:

$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
$GPRMC,183731,A,3907.482,N,12102.436,W,000.0,360.0,080301,015.5,E*67
$GPRMC,183731,A,3907.482,N,12102.436,W,000.0,360.0,080301,015.5,E*67

and print out:

<seconds since 1970>,<lon>,<latitude>


I should be able to run the program as:

echo "strings" | ./parse

and see something like:

1234567890,48.342345,01.00234322
etc

Hint, rot13: zxgvzr() pna pbaireg n qngr/gvzr vagb frpbaqf fvapr RCBP

Format:

.      1      2 3       4  5       6 7   8   9   10  11 12
              | |       |  |       | |   |   |    |   | |
$--RMC,hhmmss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a*hh
 1) Time (UTC)
 2) Status, A=data valid or V=data not valid
 3) Latitude (ddmm.mmmm)
 4) N or S
 5) Longitude
 6) E or W
 7) Speed over ground, knots
 8) Track made good, degrees true
 9) Date, ddmmyy
10) Magnetic Variation, degrees
11) A=Autonomous, D=DGPS, E=DR
12) Checksum
*/

#include <stdio.h>
#include <string.h>

#include "parse.h"


int main(int argc, char **argv) {
	nmea_error_t error = NMEA_SUCCESS;
	nmea_rmc_s   rmc;

	/* NMEA spec says 80 chars plus line terminators. Give it a bit
	 * more and we'll be able to flag lines that are too long
	 */
	char sentence[SENTENCE_RMC_LEN_MAX + 10];

	/* Set the last char to 0, fgets below will only fill up to n-1
	 * so our sentence will always be null terminated even if the input
	 * string is longer
	 */
	sentence[sizeof(sentence) - 1] = '\0';

	while(fgets(sentence, sizeof(sentence), stdin) != NULL)
	{
		/* Strip the CR and/or LF from the sentence */
		/* TODO - check for \r as well */
		if(strlen(sentence) > 0)
		{
			if(sentence[strlen(sentence) - 1] == '\n')
			{
				sentence[strlen(sentence) - 1] = '\0';
			}
		}

//		printf("%s\n", sentence);

		error = nmea_process_sentence_rmc(sentence, &rmc, 1);

		if(NMEA_SUCCESS != error)
		{
			fprintf(stderr, "Error %d processing: %s\n", error, sentence);
		}
		else
		{
			nmea_print_result(&rmc);
		}

//		printf("\n");
	}

	return 0;
}
