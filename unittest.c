/*
 * Write a unit test to verify the operation of parse.c and parse2.c Rules:

   1. There are no rules, you can do anything you want
   2. See 1

   Don't want to bother with bringing in external libraries, so will kind of create
   my own unit testing code.
*/

#include <stdio.h>
#include <string.h>

#include "parse.h"

typedef struct
{
	int  should_parse;
	char *sentence;
} nmea_unit_test_s;

int main(int argc, char **argv)
{
	int              i = 0;
	nmea_error_t     error = NMEA_SUCCESS;
	nmea_rmc_s       rmc;

	/* Add any additional tests here */
	nmea_unit_test_s test[] = {
		{1, "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A"},
		{1, "$GPRMC,183731,A,3907.482,N,12102.436,W,000.0,360.0,080301,015.5,E*67"},
		{1, "$GPRMC,183731,A,3907.482,N,12102.436,W,000.0,360.0,080301,015.5,E*67"},
		{1, "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A"},
		{0, "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6B"},  /* Bad checksum */
		{1, "$GPRMC,,,,,,,,,,,*67"},  /* All fields exist but are empty */
		{0, "$GPRMC,123519,A,4807.038,01131.000,E,022.4,084.4,230394,003.1,W*08"},  /* Missing 4th field */
		{0, "$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598, ,*10"},
		{0, "$GPRMC,230611.016,V,3907.3813,N,12102.4635,W,0.14,136.40,041002,,*04"},
		{0, "$GNRMC,185839.00,V,,,,,,,10042,GSA,A,1,27,16,,,,,,,,,,,18.45,14.67,11.19*28"},
		{0, "$GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M, , , ,0000*18"},
		{0, "$GPRMC,123519,A,4807.0345678904568,N,01131.00043222334,E,022.4,084.4,230394,003.134567890345,W*5D"},  /* Too long */
	};

	for(i = 0; i < sizeof(test) / sizeof(test[0]); i++)
	{
		memset(&rmc, 0x00, sizeof(rmc));

		error = nmea_process_sentence_rmc(test[i].sentence, &rmc, 1);

		if(test[i].should_parse == 1)
		{
			printf("Expect Pass: ");
		}
		else
		{
			printf("Expect Fail: ");
		}

		if(NMEA_SUCCESS == error)
		{
			printf("Pass - ");
		}
		else
		{
			printf("Fail - ");
		}

		printf("%s\n", test[i].sentence);
	}

	return 0;
}
