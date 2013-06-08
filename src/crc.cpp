//CRC Source
//	Created By:		Mike Moss
//	Modified On:	06/07/2013

//Definitions for "crc.hpp"
#include "crc.hpp"

//Single Byte CRC Checker (XORs bytes together)
char calculate_crc(const std::string& packet)
{
	char calc=0x00;

	for(unsigned int ii=0;ii<packet.size();++ii)
		calc^=packet[ii];

	return calc;
}