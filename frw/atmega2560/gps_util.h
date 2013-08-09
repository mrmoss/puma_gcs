//GPS Utilities Header
//	Created By:		Mike Moss
//	Modified On:	05/29/2013

//Begin Define Guards
#ifndef GPS_UTIL_H
#define GPS_UTIL_H

//Needed for STL Headers
#include <new.h>
#include <iterator>

//String Header
#include <string>

//GPS Checksum Function (Give complete sentence (with $ at beginning \r\n at end)
bool gps_checksum(const std::string& gps_sentence);

//Convert GPS Time to Seconds
float gps_hhmmss_ss_to_seconds(const std::string& buffer);

//Convert GPS Degrees Minutes to Degrees
float gps_degrees_minutes_to_degrees(const std::string& buffer);

//End Define Guards
#endif
