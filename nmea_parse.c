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
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "parse.h"

#define HEADER_GPRMC      "$GPRMC"
#define NUMBER_OF_DELIMS  11

/* Checks if this field is empty. If the field is empty then it returns non-zero value
 * and increments pointer
 *
 * TODO - this isn't returning error values
 */
static int _skip_empty_field(
	const char **ptr,
	char delim
	)
{
	if(ptr != NULL)
	{
//		printf("(%c)", **ptr);

		if(**ptr == delim)
		{
//			printf("empty field\n");
			(*ptr)++;
			return 1;
		}
	}

	return 0;
}

/* Check if *ptr is a proper delimiter and if so increment past it and return success.
 * Otherwise return an error.
 */
static nmea_error_t _check_delim(
	const char **ptr
	)
{
	nmea_error_t error = NMEA_SUCCESS;

	if(NULL == ptr)
	{
		error = NMEA_ERR_INVALID_ARG;
	}

	if(NMEA_SUCCESS == error)
	{
		if(',' == **ptr)
		{
			(*ptr)++;
		}
		else
		{
			error = NMEA_ERR_INVALID_DELIM;
		}
	}

	return error;
}

/* Convert len chars from *ptr to an unsigned int, after first validating that
 * all values in the string are numbers.
 *
 * Increment ptr by len if there were no errors.
 */
static nmea_error_t _read_uint(
	const char **ptr,
	int len,
	unsigned int *val
	)
{
	nmea_error_t error = NMEA_SUCCESS;
	int          i = 0;
	char         tmp_str[SENTENCE_RMC_LEN_MAX + 1];

	if((NULL == ptr) || (NULL == val) || (len <= 0) || (len >= SENTENCE_RMC_LEN_MAX))
	{
		error = NMEA_ERR_INVALID_ARG;
	}

	if(NMEA_SUCCESS == error)
	{
		memset(tmp_str, 0x00, sizeof(tmp_str));
		memcpy(tmp_str, *ptr, len);

		/* Validate that everything we are looking at are digits */
		for(i = 0; i < len; i++)
		{
			if(isdigit(tmp_str[i]) == 0)
			{
				error = NMEA_ERR_INVALID_INT;
				break;
			}
		}
	}

	if(NMEA_SUCCESS == error)
	{
		*val = (unsigned int)strtoul(tmp_str, NULL, 10);
		*ptr += len;
	}

	return error;
}

/* Convert len chars from *ptr to an unsigned int, after first validating that
 * all values in the string are numbers.
 *
 * Increment ptr by len if there were no errors.
 *
 * TODO - validate the input here, make sure it is in the form dd.[d].
 */
static nmea_error_t _read_double(
	const char **ptr,
	double *val
	)
{
	nmea_error_t error = NMEA_SUCCESS;
	char         *endptr = NULL;

	if((NULL == ptr) || (NULL == val))
	{
		error = NMEA_ERR_INVALID_ARG;
	}

	if(NMEA_SUCCESS == error)
	{
		*val = strtod(*ptr, &endptr);

		*ptr = endptr;
	}

	return error;
}

nmea_error_t nmea_process_sentence_rmc(
	const char *sentence,
	nmea_rmc_s *rmc,
	int validate_checksum
	)
{
	nmea_error_t  error = NMEA_SUCCESS;
	int           i = 0;
	int           delim_count = 0;  /* Number of delims found in sentence */
	int           strlen_sentence = 0;  /* We need this a lot */
	unsigned int  tmp_uint = 0;
	unsigned char checksum_calc = 0;
	unsigned char checksum_sentence = 0;
	const char    *ptr = NULL;  /* Current location as we are parsing sentence */
//	char          tmp_str[SENTENCE_RMC_LEN_MAX];

	if((NULL == sentence) || (NULL == rmc))
	{
		error = NMEA_ERR_INVALID_ARG;
	}

	if(NMEA_SUCCESS == error)
	{
		strlen_sentence = strlen(sentence);
		memset(rmc, 0x00, sizeof(*rmc));

		/* The spec says no more than SENTENCE_RMC_LEN_MAX chars once the newlines removed
		 * and we need at least 20 chars for $GPRMC,,,,,,,,,,,*XX
		 */
		if((strlen_sentence < SENTENCE_RMC_LEN_MIN) || (strlen_sentence > SENTENCE_RMC_LEN_MAX))
		{
			error = NMEA_ERR_INVALID_STRING_LEN;
		}
	}

	/* Now start to validate the sentence format
	 */

	/* 1 - Make sure the header is what we are expecting */
	if(NMEA_SUCCESS == error)
	{
		if(memcmp(sentence, HEADER_GPRMC, strlen(HEADER_GPRMC)) != 0)
		{
			error = NMEA_ERR_INVALID_HEADER;
		}
	}

	/* 2 - Make sure we have the expected number of delimeters */
	if(NMEA_SUCCESS == error)
	{
		/* - 3 because we expect the end to be "*XX", which will be verified later */
		for(i = strlen(HEADER_GPRMC); i < strlen_sentence - 3; i++)
		{
			if(',' == sentence[i])
			{
				delim_count++;
			}
		}

		if(NUMBER_OF_DELIMS != delim_count)
		{
			error = NMEA_ERR_INVALID_DELIM_COUNT;
		}
	}

	/* 3 - Make sure the sentence ends in *XX where XX are hex digits */
	if(NMEA_SUCCESS == error)
	{
		if(('*' != sentence[strlen_sentence - 3]) ||
		   (isxdigit(sentence[strlen_sentence - 2]) == 0) ||
		   (isxdigit(sentence[strlen_sentence - 1]) == 0)
		   )
		{
			error = NMEA_ERR_INVALID_STRING;
		}
	}

	/* 4 - Calculate and compare the checksum */
	if(NMEA_SUCCESS == error)
	{
		checksum_calc = 0;
		for(i = 1; i < strlen_sentence - 3; i++)
		{
			checksum_calc ^= sentence[i];
		}

		checksum_sentence = strtol(sentence + (strlen_sentence - 2), NULL, 16);

//		printf("Checksum_calc: 0x%.2X   Checksum_sentence: 0x%.2X\n", checksum_calc, checksum_sentence);

		if(validate_checksum > 0)
		{
			if(checksum_calc != checksum_sentence)
			{
				error = NMEA_ERR_BAD_CHECKSUM;
			}
		}
	}

	/* 5 - Iterate over the sentence and pull out the values, saving into rmc */
	if(NMEA_SUCCESS == error)
	{
		ptr = sentence + strlen(HEADER_GPRMC);

		error = _check_delim(&ptr);
	}

	/* Read HHMMSS */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			/* Read hours */
			error = _read_uint(&ptr, 2, &rmc->hour);			

			/* Read minutes */
			if(NMEA_SUCCESS == error)
			{
				error = _read_uint(&ptr, 2, &rmc->min);
			}

			/* Read seconds */
			if(NMEA_SUCCESS == error)
			{
				error = _read_uint(&ptr, 2, &rmc->sec);
			}

			if(NMEA_SUCCESS == error)
			{
				rmc->time_valid = 1;
				error = _check_delim(&ptr);
			}
		}
	}

//	printf("h: %d m: %d s: %d\n", rmc->hour, rmc->min, rmc->sec);

	/* Read status */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			if(('A' == *ptr) || ('V' == *ptr))
			{
				rmc->status = *ptr;
				ptr++;

				error = _check_delim(&ptr);
			}
			else
			{
				error = NMEA_ERR_INVALID_FIELD;
			}
		}
	}

	/* Read latitude */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			/* Read degrees from latitude */
			error = _read_uint(&ptr, 2, &tmp_uint);
			if(NMEA_SUCCESS == error)
			{
				rmc->lat_deg = tmp_uint;
			}

			/* Read minutes from latitude: mm.mmmm */
			if(NMEA_SUCCESS == error)
			{
				error = _read_double(&ptr, &rmc->lat_min);

				if(NMEA_SUCCESS == error)
				{
					rmc->lat_valid = 1;
					error = _check_delim(&ptr);
				}
			}
		}
	}

//	printf("f: %f\n", rmc->lat_min);

	/* N/S */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			if(('N' == *ptr) || ('S' == *ptr))
			{
				rmc->lat_n_s = *ptr;
				ptr++;

				error = _check_delim(&ptr);
			}
			else
			{
				error = NMEA_ERR_INVALID_FIELD;
			}
		}
	}

	/* Read longitude */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			/* Read degrees from longitude */
			error = _read_uint(&ptr, 3, &tmp_uint);
			if(NMEA_SUCCESS == error)
			{
				rmc->lon_deg = tmp_uint;
			}

			/* Read minutes from longitude: mm.mmmm */
			if(NMEA_SUCCESS == error)
			{
				error = _read_double(&ptr, &rmc->lon_min);
				if(NMEA_SUCCESS == error)
				{
					rmc->lon_valid = 1;
					error = _check_delim(&ptr);
				}
			}
		}
	}

//	printf("f: %f\n", rmc->lon_min);

	/* E/W */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			if(('E' == *ptr) || ('W' == *ptr))
			{
				rmc->lon_e_w = *ptr;
				ptr++;

				error = _check_delim(&ptr);
			}
			else
			{
				error = NMEA_ERR_INVALID_FIELD;
			}
		}
	}

	/* Read ground speed in knots */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			error = _read_double(&ptr, &rmc->speed_knots);
//			printf("f: %f\n", rmc->speed_knots);

			if(NMEA_SUCCESS == error)
			{
				error = _check_delim(&ptr);
			}
		}
	}

	/* Read degrees true */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			error = _read_double(&ptr, &rmc->degrees_true);
//			printf("f: %f\n", rmc->degrees_true);

			if(NMEA_SUCCESS == error)
			{
				error = _check_delim(&ptr);
			}
		}
	}

	/* Read date: DDMMYY */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			/* Read day */
			error = _read_uint(&ptr, 2, &rmc->day);

			/* Read month */
			if(NMEA_SUCCESS == error)
			{
				error = _read_uint(&ptr, 2, &rmc->month);
			}

			/* Read year */
			if(NMEA_SUCCESS == error)
			{
				error = _read_uint(&ptr, 2, &rmc->year);
			}

			if(NMEA_SUCCESS == error)
			{
				rmc->date_valid = 1;
				error = _check_delim(&ptr);
			}
		}
	}

//	printf("d: %d m: %d y: %d\n", rmc->day, rmc->month, rmc->year);

	/* Read magnetic variation */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, ',') == 0)
		{
			error = _read_double(&ptr, &rmc->magnetic_variation);
//			printf("f: %f\n", rmc->magnetic_variation);

			if(NMEA_SUCCESS == error)
			{
				error = _check_delim(&ptr);
			}
		}
	}

#if 1
	/* Read mag variation direction */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr, '*') == 0)
		{
			if(('E' == *ptr) || ('W' == *ptr))
			{
				rmc->mag_e_w = *ptr;
				ptr++;
			}
			else
			{
				error = NMEA_ERR_INVALID_FIELD;
			}

			/* Next field is checksum - don't check for delim */
		}
	}
#endif

#if 0
	/* Read mode */
	if(NMEA_SUCCESS == error)
	{
		if(_skip_empty_field(&ptr) == 0)
		{
			if(('A' == *ptr) || ('D' == *ptr) || ('E' == *ptr))
			{
				rmc->mode = *ptr;
				ptr++;
			}
			else
			{
				error = NMEA_ERR_INVALID_FIELD;
			}

			/* Next field is checksum - don't check for delim */
		}
	}
#endif

	return error;
}

void nmea_print_result(
	const nmea_rmc_s *rmc
	)
{
	struct tm time_s;
	time_t time_since_epoch = 0;
	double lat = 0.0;
	double lon = 0.0;

	memset(&time_s, 0x00, sizeof(time_s));
	time_s.tm_sec = rmc->sec;
	time_s.tm_min = rmc->min;
	time_s.tm_hour = rmc->hour;
	time_s.tm_mday = rmc->day;
	time_s.tm_mon = rmc->month - 1;

	/* TODO - not sure when to consider a 2 digit date code
	 * pre or post Y2K. Pick 1970 for now.
	 */
	if(rmc->year < 70)
	{
		/* Assume 2000+ */
		time_s.tm_year = rmc->year + 100;
	}
	else
	{
		/* Assume 1970 - 1999 */
		time_s.tm_year = rmc->year;
	}

	/* TODO - check date_valid and time_valid flags in rmc to make sure
	 * those fields were not empty. Not doing that now for simplicity.
	 */


	/* TODO - check lat_valid and lon_valid flags in rmc to make sure
	 * those fields were not empty. Not doing that now for simplicity.
	 */

	time_since_epoch = mktime(&time_s);
	lat = rmc->lat_deg + (rmc->lat_min / 60);
	lon = rmc->lon_deg + (rmc->lon_min / 60);
//	printf("%s Time: %lu\n", ctime(&time_since_epoch), (long)time_since_epoch);
//	printf("%f %f\n", rmc->lat_deg, rmc->lat_min);
//	printf("%f %f\n", rmc->lon_deg, rmc->lon_min);

	printf("%lu,%f,%f\n", (long)time_since_epoch, lat, lon);
}
