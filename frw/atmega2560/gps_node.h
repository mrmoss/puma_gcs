//GPS Node Header
//	Created By:		Mike Moss
//	Modified On:	05/29/2013

//Begin Define Guards
#ifndef GPS_NODE_H
#define GPS_NODE_H

//Needed for STL Headers
#include <new.h>
#include <iterator>

//String Header
#include <string>

//Class Declaration
class gps_node
{
	public:
		//Update Function (Pass in GPS Sentences)
		bool update(const std::string& buffer);

		//Public Member Variables
		float time;
		float lat;
		float lng;
		int fix;
		int sat_num;
		float alt;
		float speed;
		float course;
};

//End Define Guards
#endif
