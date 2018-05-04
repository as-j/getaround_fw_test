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

int main(int argc, char **argv) {
  return 0;
}
