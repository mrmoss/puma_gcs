#ifndef LOCATION_H
#define LOCATION_H

class location
{
  public:
    location(const float LAT=0.0,const float LNG=0.0,const float ALT=0.0);

    float lat;
    float lng;
    float alt;
};

#endif
