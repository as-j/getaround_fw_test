#ifndef _PARSE_H_
#define _PARSE_H_

/*

$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A

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


typedef int nmea_error_t;

/* Possible error values returned from the parse routines
 */
#define NMEA_SUCCESS                 0
#define NMEA_ERR_INVALID_ARG         1
#define NMEA_ERR_INVALID_STRING      2
#define NMEA_ERR_BAD_CHECKSUM        3
#define NMEA_ERR_INVALID_DELIM       4
#define NMEA_ERR_INVALID_INT         5
#define NMEA_ERR_INVALID_STRING_LEN  6
#define NMEA_ERR_INVALID_HEADER      7
#define NMEA_ERR_INVALID_DELIM_COUNT 8
#define NMEA_ERR_INVALID_FIELD       9

/* The nmea spec says 80 chars after removing the line feed */
#define SENTENCE_RMC_LEN_MAX  80

/* It looks like we need at least 20 for the header, delims, and checksum: $GPRMC,,,,,,,,,,,*XX */
#define SENTENCE_RMC_LEN_MIN  20

/* Checksum: http://nmeachecksum.eqth.net/ */
typedef struct
{
	/* 1 - Time (UTC)
	 */
	int time_valid;
	unsigned int hour;
	unsigned int min;
	unsigned int sec;

	/* 2 - Status */
	char         status;

	/* 3 - Laditude */
	int lat_valid;
	double lat_deg;
	double lat_min;

	/* 4 - N/S */
	char lat_n_s;

	/* 5 - Longitude */
	int lon_valid;
	double lon_deg;
	double lon_min;

	/* 6 - E/W */
	char lon_e_w;

	/* 7 - speed, knots */
	double speed_knots;

	/* 8 - track good */
	double degrees_true;

	/* 9 - Date */
	int date_valid;
	unsigned int day;
	unsigned int month;
	unsigned int year;

	/* 10 - Magnetic Variation, degrees
	 */
	double magnetic_variation;

	/* 10.5 - Magnetic variation direction - E/W */
	char mag_e_w;

	/* 11 - A=Autonomous, D=DGPS, E=DR */
	char mode;
} nmea_rmc_s;


/* Parse the GPRMC sentence into the rmc structure.
 *
 * If validate_checksum is 0 then the calculated checksum will not
 * be compared against the checksum in the sentence. This makes it
 * easier to feed the function arbitrary test data without having
 * to calc a checksum for each piece of test input first.
 *
 * If an error is returned then the contents of rmc are undefined.
 */
nmea_error_t nmea_process_sentence_rmc(
	const char *sentence,
	nmea_rmc_s *rmc,
	int validate_checksum
	);

/* Print a few basic elements from the rmc structure
 *
 * Right now this is time, latitude, and longitude (without NSEW directions)
 */
void nmea_print_result(
	const nmea_rmc_s *rmc
	);

#endif  /* _PARSE_H_ */
