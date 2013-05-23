#ifndef PUMA_GCS_LOCATION_H
#define PUMA_GCS_LOCATION_H

class location
{
	public:
		location(const float lattitude=0,const float longitude=0,const float altitude=0);

		float lat;
		float lng;
		float alt;
};

#endif