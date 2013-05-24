//Puma GCS Location Class Header
//	Created By:		Steven Kibler and Mike Moss
//	Modified On:	05/22/2013

//Begin Define Guards
#ifndef PUMA_GCS_LOCATION_H
#define PUMA_GCS_LOCATION_H

//Location Class Declaration
class location
{
	public:
		//Constructor (Default)
		location(const float lattitude=0.0,const float longitude=0.0,const float altitude=0.0);

		//Public Member Functions
		float lat;
		float lng;
		float alt;
};

//End Define Guards
#endif