//GPS Location Header
//	Created By:		Mike Moss
//	Modified On:	06/07/2013

//Begin Define Guards
#ifndef GCS_LOCATION_H
#define GCS_LOCATION_H

//Location Class Declaration
class location
{
	public:
		//Constructor (Default)
		location(const float LAT=0.0,const float LNG=0.0,const float ALT=0.0);

		float lat;
		float lng;
		float alt;
};

//End Define Guards
#endif