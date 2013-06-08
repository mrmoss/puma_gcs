//CRC Header
//	Created By:		Mike Moss
//	Modified On:	06/07/2013

//Begin Define Guards
#ifndef GCS_CRC_H
#define GCS_CRC_H

//String Header
#include <string>

//Single Byte CRC Checker (XORs bytes together)
char calculate_crc(const std::string& packet);

//End Define Guards
#endif